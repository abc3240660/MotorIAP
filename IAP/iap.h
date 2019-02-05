#ifndef __IAP_H__
#define __IAP_H__
#include "sys.h"  
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
typedef  void (*iapfun)(void);				//����һ���������͵Ĳ���.   

/*
Sector Size:
1-4:  16KB
5:    64KB
6-12: 128KB
*/

#define FLASH_BASE_ADDR		0x08000000// offset:0K
#define FLASH_IAP_SIZE		(64*1024)// size:16K*4 = 64K

#define FLASH_FLAG_ADDR		0x08010000// offset:64K
#define FLASH_FLAG_SIZE		(64*1024)// size:64KB = 0x10000

// For Normal Run
#define FLASH_RUN_ADDR		0x08020000// offset:128K
#define FLASH_RUN_SIZE		(256*1024)// size:128K*2 = 256KB = 0x40000
//#define FLASH_RUN_SIZE		(64*1024)// size:128K*2 = 256KB = 0x40000

// For Origin Backup
#define FLASH_OKBAK_ADDR		0x08060000// offset:384KB
#define FLASH_OKBAK_SIZE		(256*1024)// size:128K*2 = 256KB = 0x40000
//#define FLASH_OKBAK_SIZE		(64*1024)// size:128K*2 = 256KB = 0x40000

// Reserved for later use
#define FLASH_APP2_ADDR		0x080A0000// offset:640KB
#define FLASH_APP2_SIZE		(256*1024)// size:128K*2 = 256KB = 0x40000

void iap_load_app(u32 appxaddr);			//��ת��APP����ִ��
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 applen);	//��ָ����ַ��ʼ,д��bin
void write_flag(u32 flag);
u32 read_flag(u32 offset);
void UP_success(void);
int do_upddate_firm(u32 update_addr);
int do_backup_run(void);
int do_restore_run(void);
int do_upddate_firm_spi(void);

#define BOOT_TRY_MAX_TIMES	10

#endif