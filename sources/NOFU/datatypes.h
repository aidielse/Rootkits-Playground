//common datatypes

typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;

#define OFFSET_DRIVERSECTION Offsets.DriverSection

typedef struct _DRIVER_SECTION
{
	LIST_ENTRY listEntry;
	DWORD fieldl[4];
	DWORD field2;
	DWORD field3;
	DWORD field4;
	UNICODE_STRING filePath;
	UNICODE_STRING fileName;
	//...and who knows what else
}DRIVER_SECTION, *PDRIVER_SECTION;

#define EPROCESS_OFFSET_PID Offsets.ProcPID //offset to PID (DWORD)
#define EPROCESS_OFFSET_NAME Offsets.ProcName //offset to name[16]
#define EPROCESS_OFFSET_LINKS Offsets.ProcLinks //offset to LIST_ENTRY

#define SZ_EPROCESS_NAME 0x010 //16 bytes

#define EPROCESS_OFFSET_TOKEN Offsets.Token
#define TOKEN_OFFSET_SIDCOUNT Offsets.nSIDs
#define TOKEN_OFFSET_PRIV Offsets.PrivPresent
#define TOKEN_OFFSET_ENABLED Offsets.PrivEnabled
#define TOKEN_OFFSET_DEFAULT Offsets.PrivDefaultEnabled

typedef struct _OFFSETS
{
	BOOLEAN isSupported;
	DWORD ProcPID;
	DWORD ProcName;
	DWORD ProcLinks;
	DWORD DriverSection;
	DWORD Token;
	DWORD nSIDs;
	DWORD PrivPresent;
	DWORD PrivEnabled;
	DWORD PrivDefaultEnabled;
}
OFFSETS;

OFFSETS Offsets;

DWORD LockAcquired;
DWORD nCPUsLocked;
