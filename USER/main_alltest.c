#include "sys.h"
#include "delay.h"  
#include "usart.h"   
#include "led.h"
#include "lcd.h"
#include "key.h"   
#include "stmflash.h" 
#include "iap.h"   
#include "sdio_sdcard.h"    
#include "malloc.h" 
#include "w25qxx.h"    
#include "ff.h"  
#include "exfuns.h" 

typedef struct {
	u32 need_iap_flag;
	u32 need_bak_flag;
	u32 iap_ok_flag;
	u32 try_run_cnt;
} IAP_ENV;

//flag����
//31~16 ����
//15~8  try         --- ��ΧΪ0~10����������ÿ����������++����APP��Ӧ�ý�����0�����ϻ���Ϊ�ô�����ʧ�ܣ�ʧ��10�κ���ת����ǰ��app
//7~5   ����   
//4     update flag ---1:���� 0 : �����¡��ñ�־��app�н�����̼��Ժ����ô˱�־Ϊ1���´�������ʱ�����¹̼���
//					     ���³ɹ�������˱�־λ0�����л�jump�������¸��µ�app
//						 �������ʧ�ܣ���������α�ǣ�Ҳ�����л�jump����Ȼ�����ɵ�app����app�м��α�ǣ�������Ϊ1�����������ʧ�ܡ�
//3~1   ����
//0	    jump ---       0������app1   1:����app2

//Ҫд�뵽W25Q16���ַ�������
const u8 TEXT_Buffer[]={"Explorer STM32F4 SPI TEST"};
#define SIZE sizeof(TEXT_Buffer)	 
u8 g_flash_buf[W25Q_SECTOR_SIZE] = {0};

int main(void)
{ 
	u8 t;
	u8 key;
	u16 oldcount=0;	//�ϵĴ��ڽ�������ֵ
	u32 applenth=30*1024;	//���յ���app���볤��
	u8 clearflag=0; 
	u32 app_offset=0;
	u32 CHECK_FLAG=0;
	u32 flag=0;
	u32 update_addr=0;
	u32 run_addr=0;
	u32 tmp[2]={0};
	u32 try_cnt=10;
	
	u8 datatemp[SIZE];
	u32 FLASH_SIZE;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	delay_init(168);  //��ʼ����ʱ����
	uart_init(115200);		//��ʼ�����ڲ�����Ϊ115200

	W25QXX_Init();			//W25QXX��ʼ��

#if 0
	while(W25QXX_ReadID()!=W25Q128)								//��ⲻ��W25Q128
	{
		delay_ms(500);
		LED0=!LED0;		//DS0��˸
	}

	FLASH_SIZE=16*1024*1024;	//FLASH ��СΪ16�ֽ�
	
	W25QXX_Write((u8*)TEXT_Buffer,FLASH_SIZE-100,SIZE);		//�ӵ�����100����ַ����ʼ,д��SIZE���ȵ�����
	delay_ms(10);
	W25QXX_Read(datatemp,FLASH_SIZE-100,SIZE);					//�ӵ�����100����ַ����ʼ,����SIZE���ֽ�
#endif

	while ((SD_OK != SD_Init()) && (try_cnt--)>0) {
		delay_us(10);
	}
	
	if (try_cnt > 0) {
		exfuns_init();
		if (f_mount(fs[0],"0:",1) != FR_OK) {
			printf("\r\n FAT Error !\r\n");
		} else {
			printf("\r\n FAT OK !\r\n");
		}
	}
	
	IAP_ENV iap_env;
	
	W25QXX_Read((u8*)&iap_env, ENV_SECTOR_INDEX_IAP*W25Q_SECTOR_SIZE, sizeof(IAP_ENV));
	
	printf("try_run_cnt = %d\n", iap_env.try_run_cnt);
	
	if (iap_env.try_run_cnt > 10) {
		iap_env.try_run_cnt = 0;
	}
	
	iap_env.try_run_cnt++;
	W25QXX_Write((u8*)&iap_env, ENV_SECTOR_INDEX_IAP*W25Q_SECTOR_SIZE, sizeof(IAP_ENV));
	
	if (3 == iap_env.try_run_cnt) {
#if 0
		do_upddate_firm(FLASH_RUN_ADDR);
#else
		do_upddate_firm_spi();
#endif
		printf("update run success\n");
	} else if (5 == iap_env.try_run_cnt) {
		printf("backup run start\n");
		//do_backup_run();
		printf("backup run success\n");
	} else if (7 == iap_env.try_run_cnt) {
		printf("restore run start\n");
		do_restore_run();
		printf("restore run success\n");
	}
	
	try_cnt = 0;
	while(1) {
		if(((*(vu32*)(FLASH_RUN_ADDR+4))&0xFF000000)==0x08000000)//�ж��Ƿ�Ϊ0X08XXXXXX.
		{	 
				iap_load_app(FLASH_RUN_ADDR);//ִ��FLASH APP����
		}
		
		printf("jump app failed\n");
		
		if (10 == try_cnt++) {
			do_restore_run();
		}
		
		delay_ms(1000);
	}
}

