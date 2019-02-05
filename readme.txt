FLASH 

start:0x0800_0000 end:0x080F_FFFF size:(1 MByte)
FLASH 共有11个sector
4个16K
1个64k
7个128K

IAP:占用前4个16K共计64K（sector0~3）
start:0x0800_0000 end:0x0800_FFFF size:64 kbyte

APP1:占用1个64K和3个128K共计458K(sector4~7)
start:0x0801_0000 end:0x0808_27FF 

APP2：占用4个128K共计512K（sector8~11）
start:0x0808_2800 end:0x080F_FFFF size:512kbyte

C:\Keil_v5\ARM\ARMCC\bin\fromelf.exe --bin -o ..\OBJ\jia.bin ..\OBJ\jia.axf
D:\MDK5.14\ARM\ARMCC\bin\fromelf.exe  --bin -o  ..\OBJ\TEST.bin ..\OBJ\TEST.axf