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

BOOLEAN isOSSupported()
{
	return(Offsets.isSupported);
}
/*end isOSSupported() - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void checkOSVersion()
{
	NTSTATUS retVal;
	RTL_OSVERSIONINFOW versionInfo;
	versionInfo.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);
	retVal = RtlGetVersion(&versionInfo);
	Offsets.isSupported = TRUE;
	DBG_PRINT2("[checkOSVersion]: Major #=%d", versionInfo.dwMajorVersion);
	switch (versionInfo.dwMajorVersion)
	{
	case(4) :
	{
		DBG_TRACE("checkOSVersion", "OS=NT");
		Offsets.isSupported = FALSE;
	}break;
	case(5) :
	{
		DBG_TRACE("checkOSVersion", "OS=2000, XP, Server 2003");
		Offsets.isSupported = FALSE;
	}break;
	case(6) :
	{
		DBG_TRACE("checkOSVersion", "OS=Vista, Server 200S");
		Offsets.isSupported = TRUE;
		Offsets.ProcPID = 0x09C;
		Offsets.ProcName = 0x14C;
		Offsets.ProcLinks = 0x0A0;
		Offsets.DriverSection = 0x014;
		Offsets.Token = 0x0e0;
		Offsets.nSIDs = 0x078;
		Offsets.PrivPresent = 0x040;
		Offsets.PrivEnabled = 0x048;
		Offsets.PrivDefaultEnabled = 0x050;
		DBG_PRINT2(" [checkOSVersion]: ProcID=%e3x%", Offsets.ProcPID);
		DBG_PRINT2(" [ checkOSVersion]: ProcName=%e3x%", Offsets.ProcName);
		DBG_PRINT2(" [checkOSVersion]: ProcLinks=%e3x%", Offsets.ProcLinks);
		DBG_PRINT2(" [ checkOSVersion]: Dri verSection=%e3x%", Offsets.DriverSection);
		DBG_PRINT2(" [checkOSVersion]: Token=%e3x%", Offsets.Token);
		DBG_PRINT2(" [checkOSVersion]: nSIDs=%e3x%", Offsets.nSIDs);
		DBG_PRINT2(" [checkOSVersion]: PrivPresent=%e3x%", Offsets.PrivPresent);
		DBG_PRINT2(" [ checkOSVersion] : Pri vEnabled=%e3x%", Offsets.PrivEnabled);
		DBG_PRINT2(" [checkOSVersion]: Pri vDefaul tEnabled=%e3x%", Offsets.PrivDefaultEnabled);
	}break;
	default:
	{
		Offsets.isSupported = FALSE;
	}
	}
	return;
}/*end checkOSVersion() - -----------------------------------------------------*/
