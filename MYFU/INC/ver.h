BOOLEAN isOSSupported()
{
	return(Offsets.isSupported);
}
//this function checks the current OS version, and sets the our structure offsets accordingly.
//Validated to work on 7/28/2015
void checkOSVersion()
{
	NTSTATUS retVal;
	RTL_OSVERSIONINFOW versionInfo;

	versionInfo.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);
	retVal = RtlGetVersion(&versionInfo);

	Offsets.isSupported = TRUE;

	DBG_PRINT2("[checkOSVersion]: Major#= %d", versionInfo.dwMajorVersion);
	switch (versionInfo.dwMajorVersion)
	{
		case(4):
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
			if (versionInfo.dwMinorVersion == 1)
			{
				//NEEDS DOUBLE CHECKING
				DBG_TRACE("checkOSVersion", "OS=7, server 2008");
				//THESE OFFSETS ARE FOR Windows 7 kernel version 7601 MP
				Offsets.isSupported = TRUE;
				//_EPROCESS offsets
				Offsets.ProcPID = 0x0B4; //UniqueProcessId;
				Offsets.ProcName = 0x16C; //ImageFileName
				Offsets.ProcLinks = 0x0B8; //ActiveProcessLinks 
				Offsets.Token = 0x0f8; //Token
				//_DRIVER_OBJECT offsets
				Offsets.DriverSection = 0x014;
				//_TOKEN Privileges
				Offsets.PrivPresent = 0x040;
				Offsets.PrivEnabled = 0x048;
				Offsets.PrivDefaultEnabled = 0x050;
				Offsets.nSIDs = 0x078;

				DBG_PRINT2(" [checkOSVersion]: ProcID=%3x", Offsets.ProcPID);
				DBG_PRINT2(" [checkOSVersion]: ProcName=%3x", Offsets.ProcName);
				DBG_PRINT2(" [checkOSVersion]: ProcLinks=%3x", Offsets.ProcLinks);
				DBG_PRINT2(" [checkOSVersion]: DriverSection=%3x", Offsets.DriverSection);
				DBG_PRINT2(" [checkOSVersion]: Token=%3x", Offsets.Token);
				DBG_PRINT2(" [checkOSVersion]: nSIDs=%3x", Offsets.nSIDs);
				DBG_PRINT2(" [checkOSVersion]: PrivPresent=%3x", Offsets.PrivPresent);
				DBG_PRINT2(" [checkOSVersion]: PrivEnabled=%3x", Offsets.PrivEnabled);
				DBG_PRINT2(" [checkOSVersion]: PrivDefaultEnabled=%3x", Offsets.PrivDefaultEnabled);
			}
			else
			{
				DBG_TRACE("checkOSVersion", "OS=Vista, Server 2003");
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
			}
		}break;
		default:
		{
			Offsets.isSupported = FALSE;
		}
	}
	return;
}
