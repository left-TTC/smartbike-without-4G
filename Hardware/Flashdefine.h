#ifndef __FLASHDEFINE_H
#define __FLASHDEFINE_H

#define RentUserFlash_page_Start 0x0800F800                 //It also records the lenth of RentUser's data
#define RentUserFlash_Second_page_Start 0x0800FC00
#define RentUserFlash_Start_Write_Address 0x0800F804
#define SuperUserFlash_page_Start 0x0800F400                //It also records the Flag Weather the flash data is saved
#define SuperUserFlash_Strlen_Record 0x0800F404
#define SuperUserFlash_IfRecordName_Record 0x0800F408
#define SuperUserFlash_Start_Write_Address 0x0800F40C

#endif
