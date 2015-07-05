#include "ntddk.h"
NTSTATUS DriverEntry( IN PDRIVER_OBJECT theDriverObject, IN PUNICODE_STRING theRegistryPath )

{
	DbgPrint("Hello World!");

	return STATUS_SUCCESS;
}