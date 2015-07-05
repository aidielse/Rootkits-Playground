#include <Windows.h>
#include <stdio.h>

bool _util_load_sysfile(char * theDriverName)
{
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
		CloseServiceHandle(sh);
		CloseServiceHandle(rh);
	}
	return true;
}

int main(int argc, char * argv[]) {
	
	if (_util_load_sysfile(argv[1]) == true) {
		printf("Driver Loaded Successfully!\n");
		return 0;
	}
	printf("Driver load failed :-( \n");
	return -1;
}