#include <Windows.h>
#include <winioctl.h>
#include <stdio.h>

int main() {

	HANDLE hDevice = CreateFile("\\\\Device\\helloworld3", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD dwReturn;
	if (hDevice == ((HANDLE)-1)) {
		printf("Handle could not be opened\n");
		return -1;
	}

	WriteFile(hDevice, "HELLO!", sizeof("HELLO!"), &dwReturn, NULL);
	CloseHandle(hDevice);
	return 0;
}