//Written by Aaron Sedlacek
//07/03/2015
//Most of this code comes directly from "Rootkits" by Greg Hoglund and James Butler, with some minor tweaks
#include <Windows.h>
#include <stdio.h>

//this function loads a driver into kernel space using SCM
bool _util_load_sysfile(char * theDriverName)
{

	SERVICE_STATUS ss;
	char aPath[1024];
	char aCurrentDirectory[515];
	SC_HANDLE sh = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (!sh)
	{
		return false;
	}

	GetCurrentDirectory(512, aCurrentDirectory);

	_snprintf_s(aPath, 1022, "%s\\%s.sys", aCurrentDirectory, theDriverName);

	printf("loading %s\n", aPath);

	SC_HANDLE rh = CreateService(sh,
		theDriverName,
		theDriverName,
		SERVICE_ALL_ACCESS,
		SERVICE_KERNEL_DRIVER,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_NORMAL,
		aPath,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL);
	if (!rh)
	{
		if (GetLastError() == ERROR_SERVICE_EXISTS)
		{
			//service exists
			rh = OpenService(sh,
				theDriverName,
				SERVICE_ALL_ACCESS);
			if (!rh)
			{
				CloseServiceHandle(sh);
				return false;
			}
		}
		else
		{
			CloseServiceHandle(sh);
			return false;
		}
	}
	if (rh)
	{
		if (0 == StartService(rh, 0, NULL))
		{
			if (ERROR_SERVICE_ALREADY_RUNNING == GetLastError()){}
			else
			{
				CloseServiceHandle(sh);
				CloseServiceHandle(rh);
				return false;
			}
		}
		printf("Press Enter to close service\r\n");
		getchar();
		ControlService(rh, SERVICE_CONTROL_STOP, &ss);

		DeleteService(rh);
		CloseServiceHandle(rh);
		CloseServiceHandle(sh);
	}
	return true;
}

int main(int argc, char * argv[]) {

	if (argv[1]) {
		//call the loader function
		_util_load_sysfile(argv[1]);
		return 0;
	}
	else {
		printf("Usage: driverLauncher.exe DriverName");
		return -1;
	}
}