//Written by Aaron Sedlacek
//07/03/2015
//This program loads a driver into the kernel, and then unloads it later.
#include <Windows.h>
#include <stdio.h>

//This code mostly comes from Greg Hoglund's and James Butler's 'Rootkits' book.
bool _util_load_sysfile(char * theDriverName)
{
	SERVICE_STATUS ss;
	char aPath[1024];
	char aCurrentDirectory[515];
	//get handle to SCM database
	SC_HANDLE sh = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (!sh)
	{
		return false;
	}

	GetCurrentDirectory(512, aCurrentDirectory);

	_snprintf_s(aPath, 1022, "%s\\%s.sys", aCurrentDirectory, theDriverName);

	printf("loading %s\n", aPath);
	//create our driver as a service
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
		//start the service
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
		//stop the service
		ControlService(rh, SERVICE_CONTROL_STOP, &ss);
		//remove the service from the kernel
		DeleteService(rh);
		//close our handles
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