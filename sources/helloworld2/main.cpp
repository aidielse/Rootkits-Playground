#include <Windows.h>
#include <stdio.h>

bool _util_unload_sysfile(char * theDriverName) {

	HANDLE hService;
	SERVICE_STATUS ss;
	SC_HANDLE sh = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (!sh)
	{
		printf("Could not open SCM\n");
		return false;
	}

	hService = OpenService(sh, theDriverName, SERVICE_START | DELETE | SERVICE_STOP);

	if (hService) {
		ControlService(hService, SERVICE_CONTROL_STOP, &ss);

		DeleteService(hService);

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