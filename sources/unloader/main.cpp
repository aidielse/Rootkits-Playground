#include <Windows.h>
#include <stdio.h>
//Written by Aaron Sedlacek, 07/05/2015
//this program unloads a driver :)
bool _util_unload_sysfile(char * theDriverName) {

	SC_HANDLE hService;
	SERVICE_STATUS ss;
	//open Service Control Manager Database 
	SC_HANDLE sh = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (!sh)
	{
		printf("Could not open SCM\n");
		return false;
	}
	//open our service to unload
	hService = OpenService(sh, theDriverName, SERVICE_START | DELETE | SERVICE_STOP);

	if (hService) {
		//stop the service
		ControlService(hService, SERVICE_CONTROL_STOP, &ss);
		//delete the service
		DeleteService(hService);
		//close our handles
		CloseServiceHandle(hService);
		CloseServiceHandle(sh);
	}
	else {
		printf("Could not open service\n");
		CloseServiceHandle(sh);
		return false;
	}
	return true;
}

int main(int argc, char * argv[]) {

	printf("Unloading %s\n", argv[1]);
	if (_util_unload_sysfile(argv[1])) {
		printf("Service unloaded!\n");
		return 0;
	}
	else {
		printf("Could not unload service\n");
		return -1;
	}
}