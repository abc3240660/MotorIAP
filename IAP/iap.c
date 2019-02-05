#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "stmflash.h"
#include "iap.h"
#include "sdio_sdcard.h"    
#include "malloc.h" 
#include "w25qxx.h"    
#include "ff.h"  
#include "exfuns.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//IAP ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/7/21
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 

extern u8 g_flash_buf[W25Q_SECTOR_SIZE];

iapfun jump2app; 
u32 iapbuf[2048]; 	//2K�ֽڻ���  
//appxaddr:Ӧ�ó������ʼ��ַ
//appbuf:Ӧ�ó���CODE.
//appsize:Ӧ�ó����С(�ֽ�).
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 appsize)
{
	u32 t;
	u16 i=0;
	u32 temp;
	u32 fwaddr=appxaddr;//��ǰд��ĵ�ַ
	u8 *dfu=appbuf;
	for(t=0;t<appsize;t+=4)
	{						   
		temp=(u32)dfu[3]<<24;   
		temp|=(u32)dfu[2]<<16;    
		temp|=(u32)dfu[1]<<8;
		temp|=(u32)dfu[0];	  
		dfu+=4;//ƫ��4���ֽ�
		iapbuf[i++]=temp;	    
		if(i==512)
		{
			i=0; 
			STMFLASH_Write(fwaddr,iapbuf,512);
			fwaddr+=2048;//ƫ��2048  512*4=2048
		}
	} 
	if(i)STMFLASH_Write(fwaddr,iapbuf,i);//������һЩ�����ֽ�д��ȥ.  
}

//��ת��Ӧ�ó����
//appxaddr:�û�������ʼ��ַ.
void iap_load_app(u32 appxaddr)
{
	if(((*(vu32*)appxaddr)&0x2FFE0000)==0x20000000)	//���ջ����ַ�Ƿ�Ϸ�.
	{ 
		jump2app=(iapfun)*(vu32*)(appxaddr+4);		//�û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ)		
		MSR_MSP(*(vu32*)appxaddr);					//��ʼ��APP��ջָ��(�û��������ĵ�һ�������ڴ��ջ����ַ)
		jump2app();									//��ת��APP.
	}
}		 

void write_flag(u32 flag)
{
	FLASH_Status status = FLASH_COMPLETE;
	u32 addr=FLASH_FLAG_ADDR;
	
	while(1){
		status=FLASH_EraseSector(FLASH_Sector_3,VoltageRange_3);//VCC=2.7~3.6V֮��!!
		if(status==FLASH_COMPLETE)
			break;
	}
	while(1){
		if(FLASH_ProgramWord(addr,flag)==FLASH_COMPLETE)
			break;
	}
}
u32 read_flag(u32 offset)
{
	u32 addr = FLASH_FLAG_ADDR + offset;
	return STMFLASH_ReadWord(addr);
}

#define MAX_BUFF_SIZE   512
int do_upddate_firm(u32 update_addr)
{
	FRESULT fr;
  FATFS fs;
  FIL fp;
	u32 br = 0;
	u32 once = 0;
	u32 file_size = 0;
	u8 buf[MAX_BUFF_SIZE] = "";
	
    /* Opens an existing file. If not exist, creates a new file. */
    fr = f_open(&fp, "TEST.bin", FA_READ | FA_OPEN_ALWAYS);
    if (fr != FR_OK) {
		return -1;
    }
	printf("file_size=0x%x ",file_size);
	file_size = fp.fsize;
	printf("file_size=0x%x ",file_size);
	do{
		printf("file_size=0x%x ",file_size);
		if(file_size > MAX_BUFF_SIZE)
			once = MAX_BUFF_SIZE;
		else
			once = file_size;
		fr = f_read (&fp, buf,	once, &br);
		if(once != br)
		{
			printf("Read file failed ! \r\n");
			break;
		}else{
			iap_write_appbin(update_addr + (fp.fsize - file_size), buf, once);
			file_size -= once;
		}
		
	}while(file_size>0);

    f_close(&fp);

	return 0;
}

int do_backup_run(void)
{
	u32 i = 0;
	u32 j = 0;
	u8 buf[MAX_BUFF_SIZE];
	
	u32 *pSectorBuf = (u32*)buf;

	for (i=0; i<(FLASH_RUN_SIZE/MAX_BUFF_SIZE); i++) {
		for (j=0; j<(MAX_BUFF_SIZE/4); j++) {
			pSectorBuf[j] = STMFLASH_ReadWord(FLASH_RUN_ADDR+i*MAX_BUFF_SIZE+j*4);
		}
		iap_write_appbin(FLASH_OKBAK_ADDR + i*MAX_BUFF_SIZE, buf, MAX_BUFF_SIZE);
	}

	return 0;
}

int do_restore_run(void)
{
	u32 i = 0;
	u32 j = 0;
	u8 buf[MAX_BUFF_SIZE];
	
	u32 *pSectorBuf = (u32*)buf;

	for (i=0; i<(FLASH_RUN_SIZE/MAX_BUFF_SIZE); i++) {
		for (j=0; j<(MAX_BUFF_SIZE/4); j++) {
			pSectorBuf[j] = STMFLASH_ReadWord(FLASH_OKBAK_ADDR+i*MAX_BUFF_SIZE+j*4);
		}
		iap_write_appbin(FLASH_RUN_ADDR + i*MAX_BUFF_SIZE, buf, MAX_BUFF_SIZE);
	}

	return 0;
}

int do_upddate_firm_spi(void)
{
	u32 i = 0;
	
	for (i=0; i<(FLASH_RUN_SIZE/W25Q_SECTOR_SIZE); i++) {
			memset(g_flash_buf, 0x55, W25Q_SECTOR_SIZE);
			W25QXX_Write(g_flash_buf, (BIN_SECTOR_INDEX_IAP+i)*W25Q_SECTOR_SIZE, W25Q_SECTOR_SIZE);	
			delay_ms(10);
			memset(g_flash_buf, 0x00, W25Q_SECTOR_SIZE);
			W25QXX_Read(g_flash_buf, (BIN_SECTOR_INDEX_IAP+i)*W25Q_SECTOR_SIZE, W25Q_SECTOR_SIZE);
			delay_ms(10);
			iap_write_appbin(FLASH_RUN_ADDR + i*W25Q_SECTOR_SIZE, g_flash_buf, W25Q_SECTOR_SIZE);
	}

	return 0;
}