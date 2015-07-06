//Written by Aaron Sedlacek, 07/05/2015
//this is a simple driver that only exists to be loaded and unloaded.
#include "ntddk.h"
//our device's file handle
const WCHAR deviceNameBuffer[] = L"\\Device\\helloworld3";
//global pointer to our device.
PDEVICE_OBJECT g_RootkitDevice;

//Unload function
NTSTATUS OnUnload(IN PDRIVER_OBJECT DriverObject)
{
	DbgPrint("OnUnload called\n");
	IoDeleteDevice(DriverObject->DeviceObject);
	return STATUS_SUCCESS;
}

NTSTATUS MyOpen(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	DbgPrint("Open Reached\n");
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS MyClose(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	DbgPrint("Close Reached\n");
	return STATUS_SUCCESS;
}
NTSTATUS MyRead(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	DbgPrint("Read Reached\n");
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}
NTSTATUS MyWrite(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	DbgPrint("Write Reached\n");
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS MyIOControl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	DbgPrint("IOCTL Reached\n");

	PIO_STACK_LOCATION IrpSp;
	ULONG FunctionCode;
	IrpSp = IoGetCurrentIrpStackLocation(Irp);
	FunctionCode = IrpSp->Parameters.DeviceIoControl.IoControlCode;

	DbgPrint("IOCTL Code: %lu\n",FunctionCode);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry( IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING theRegistryPath )
//all this driver does is print hello world to debug out.
{
	NTSTATUS ntStatus;
	UNICODE_STRING deviceNameUnicodeString;
	
	RtlInitUnicodeString(&deviceNameUnicodeString, deviceNameBuffer);
	//create a file handle to allow user-mode programs to interface with our device
	ntStatus = IoCreateDevice(DriverObject, 0, &deviceNameUnicodeString,FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &g_RootkitDevice);

	DbgPrint("I loaded!\n");
	//set routines
	DriverObject->MajorFunction[IRP_MJ_CREATE] = MyOpen;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = MyClose;
	DriverObject->MajorFunction[IRP_MJ_READ] = MyRead;
	DriverObject->MajorFunction[IRP_MJ_WRITE] = MyWrite;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = MyIOControl;
	DriverObject->DriverUnload = OnUnload;

	return STATUS_SUCCESS;
}