#ifndef STRUCT_H
#define STRUCT_H
#define Max_RentUser_Count 35

/********************************************************
* Struct name   :RentUserList
* Description   :a structure to save information about the Rentuser
* Parameter     :
* @RentAddress   42-bit address(Omit 0x)
* @CanUserOpenBatteryLock       char: Y=> Unauthorized switch battery lock  N=>Authorized switch battery lock  
* @TimeStamp     Time available
* @IfTrustCompletely            Is it fully trusted?  Y/N
* Return        :None
**********************************************************/
typedef struct{
	char RentAddress[41];
	char CanUserOpenBatteryLock[2];
	char TimeStamp[11];
	char IfTrustCompletely[2];
} RentUser;

/********************************************************
* Struct name   :RentUserList
* Description   :a structure to save all RentUser
* Parameter     :
* @userCount     The number of RentUser
* @RentUser users       the struct RentUser
* Return        :None
**********************************************************/
typedef struct{
	int userCount;
	RentUser users[Max_RentUser_Count];
} RentUserList;

extern RentUserList Rent_Flash_Struct;

/********************************************************
* struct name   :SuperUser
* Description   :Create a superuser structure
* Parameter     :
* @Bike_Name           The Device's name
* @WalletAddress       The SuperUser's wallet Address
* @User_PhoneNumber    The SuperUser's phone number
* @User_Wechat         The SuperUser's Wechat info
* Return        :None
**********************************************************/
typedef struct{
	char Bike_Name[12];
	char WalletAddress[41];
	char User_PhoneNumber[12];
	char User_Wechat[60];
} SuperUser;

extern SuperUser SuperUser_Flash;



#endif
