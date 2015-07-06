#include "ntddk.h"
NTSTATUS DriverEntry( IN PDRIVER_OBJECT theDriverObject, IN PUNICODE_STRING theRegistryPath )
//all this driver does is print hello world to debug out, the only way to unload this driver is by restarting your machine.
{
	DbgPrint("Hello World!");

	return STATUS_SUCCESS;
}