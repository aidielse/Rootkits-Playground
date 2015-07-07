#include <Windows.h>
#include <stdio.h>
//this program sends some IRP's to the target device for demo purposes
int _cdecl main(void)
{
	HANDLE hFile;
	DWORD dwReturn;
	//createfile IRP, gets a handle to the driver
	hFile = CreateFile("\\\\.\\helloworld3",
		GENERIC_READ | GENERIC_WRITE, 0, NULL,
		OPEN_EXISTING, 0, NULL);

	if (hFile)
	{
		//send IOCTL IRP to driver
		DeviceIoControl(hFile,1,NULL,0,NULL,0,&dwReturn,NULL);
		//closehandle, sends IRP to driver.
		CloseHandle(hFile);
	}
	return 0;
}