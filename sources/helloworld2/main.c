//Written by Aaron Sedlacek, 07/05/2015
//this is a simple driver that only exists to be loaded and unloaded.
#include "ntddk.h"
//our device's file handle
const WCHAR deviceNameBuffer[] = L"\\Device\\helloworld2";
//global pointer to our device.
PDEVICE_OBJECT g_RootkitDevice;

//Unload function
VOID OnUnload(IN PDRIVER_OBJECT DriverObject)
{
	DbgPrint("OnUnload called\n");
}
NTSTATUS DriverEntry( IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING theRegistryPath )
//all this driver does is print hello world to debug out.
{
	NTSTATUS ntStatus;
	UNICODE_STRING deviceNameUnicodeString;
	
	RtlInitUnicodeString(&deviceNameUnicodeString, deviceNameBuffer);
	//create a file handle to allow user-mode programs to interface with our device
	ntStatus = IoCreateDevice(DriverObject, 0, &deviceNameUnicodeString, 0x00001234, 0, TRUE, &g_RootkitDevice);

	DbgPrint("I loaded!\n");
	//set unload routine
	DriverObject->DriverUnload = OnUnload;

	return STATUS_SUCCESS;
}