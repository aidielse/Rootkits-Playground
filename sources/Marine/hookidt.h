#include "datatypes.h"
//pragma's guarantee that the structures members
//will be aligned on a n byte boundary. 
//AKA everything will be crammed into the minimum ammount of space,
//no extra padding to satisfy alignment requirements.
#pragma pack(1)
typedef struct _IDTR
{
	WORD nBytes; //size limit in bytes
	WORD baseAddressLow; //lo-order bytes, base address
	WORD baseAddressHi; //hi-order byte, base address
}IDTR;
#pragma pack()

#pragma pack(1)
typedef struct _IDT_DESCRIPTOR
{
	WORD offset00_15; //offset address bits 0-15
	WORD selector; //segment selector
	BYTE unused : 5; //not used
	BYTE zeroes : 3; // null
	BYTE gateType : 5; //Interrupt or Trap (01110,01111)
	BYTE DPL : 2; //descriptor privilege level
	BYTE P : 1; //segment present flag
	WORD offset16_31; //offset address bits
}IDT_DESCRIPTOR, *PIDT_DESCRIPTOR;
#pragma pack()

void LogSystemCall(DWORD dispatchID, DWORD stackPtr);
DWORD makeDWORD(WORD hi, WORD lo);
void HookInt2E();
void HookAllCPUS();