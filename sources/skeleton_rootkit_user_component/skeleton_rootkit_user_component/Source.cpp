//Written By Aaron Sedlacek
//07/05/2015

#include <Windows.h>
#include <stdio.h>

#define FILE_DEVICE_RK 0x00008001
#define IOCTL_TEST_CMD CTL_CODE(FILE_DEVICE_RK, 0x801, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
//this program sends some IRP's to the target device for demo purposes

int TestOperation(HANDLE hFile)
{
	BOOL opStatus = TRUE;
	char *inBuffer;
	char *outBuffer;
	DWORD nBufferSize = 32;
	DWORD bytesRead = 0;
	//allocate input and output buffers
	inBuffer = (char*)malloc(nBufferSize);
	outBuffer = (char*)malloc(nBufferSize);

	if (inBuffer == NULL || outBuffer == NULL)
	{
		printf("TestOperation could not allocate memory for CMD_TEST_OP\n");
		return -1;
	}

	sprintf_s(inBuffer, sizeof("This is the INPUT buffer"), "This is the INPUT buffer");
	sprintf_s(outBuffer, sizeof("This is the OUTPUT buffer"), "This is the OUTPUT buffer");
	//send IOCTL to our device (and therefore, our driver)
	opStatus = DeviceIoControl(hFile,IOCTL_TEST_CMD,(LPVOID)inBuffer,nBufferSize,(LPVOID)outBuffer,nBufferSize,&bytesRead,NULL);
	if (opStatus == FALSE) {
		printf("Call to DeviceIoControl FAILED\n");
	}
	printf("bytesRead=%d\n", bytesRead);
	printf("outBuffer=%s\n", outBuffer);
	//free buffers
	free(inBuffer);
	free(outBuffer);

	return 0;
}
int _cdecl main(void)
{
	HANDLE hFile;
	DWORD dwReturn;
	//createfile IRP, gets a handle to the driver
	hFile = CreateFile("\\\\.\\msnetdiag",
		GENERIC_READ | GENERIC_WRITE, 0, NULL,
		OPEN_EXISTING, 0, NULL);

	if (hFile)
	{
		//send IOCTL IRP to driver
		TestOperation(hFile);
		//closehandle, sends IRP to driver.
		CloseHandle(hFile);
	}
	return 0;
}