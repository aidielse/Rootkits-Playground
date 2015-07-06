//Written by Aaron Sedlacek, 07/05/2015
//this is a simple driver to be used as a template/reference for future development
#include "ntddk.h"

//demo unload function, for removing the driver
VOID Example_Unload(PDRIVER_OBJECT  DriverObject)
{
	UNICODE_STRING usDosDeviceName;

	DbgPrint("Example_Unload Called \r\n");
	//delete sym link
	RtlInitUnicodeString(&usDosDeviceName, L"\\DosDevices\\Example");
	IoDeleteSymbolicLink(&usDosDeviceName);
	//delete the device itself
	IoDeleteDevice(DriverObject->DeviceObject);
}
//Example unsupported function
NTSTATUS Example_UnSupportedFunction(PDEVICE_OBJECT DeviceObject, PIRP Irp) {

	DbgPrint("UnSupported Function \r\n");
	//set IRP as completed
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}
//Example Close function
NTSTATUS Example_Close(PDEVICE_OBJECT DeviceObject, PIRP Irp) {

	DbgPrint("Close Function \r\n");
	//set IRP as completed
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}
//Example Create function
NTSTATUS Example_Create(PDEVICE_OBJECT DeviceObject, PIRP Irp) {

	DbgPrint("Create Function \r\n");
	//set IRP as completed
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}
//Example IOCTL handler function
NTSTATUS Example_IoControl(PDEVICE_OBJECT DeviceObject, PIRP Irp) {

	DbgPrint("IOCTL Reached\n");

	PIO_STACK_LOCATION IrpSp;
	ULONG FunctionCode;
	//Get pointer to IRP stack
	IrpSp = IoGetCurrentIrpStackLocation(Irp);
	//get IOCTL code
	FunctionCode = IrpSp->Parameters.DeviceIoControl.IoControlCode;
	//print the IOCTL code
	DbgPrint("IOCTL Code: %lu\n", FunctionCode);
	//set IRP as completed
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}
//Example Read Function
NTSTATUS Example_Read(PDEVICE_OBJECT DeviceObject, PIRP Irp) {

	DbgPrint("Read Function \r\n");
	//Complete IRP 
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}
//Example Write, STILL BUGGY, WAS CAUSING KERNEL PANICS SO I GOT LAZY WITH IT
NTSTATUS Example_Write(PDEVICE_OBJECT DeviceObject, PIRP Irp) {

	DbgPrint("Write Function \r\n");

	NTSTATUS NtStatus = STATUS_SUCCESS;
	PIO_STACK_LOCATION pIoStackIrp = NULL;
	PCHAR pWriteDataBuffer;

	pIoStackIrp = IoGetCurrentIrpStackLocation(Irp);

	if (pIoStackIrp)
	{
		/*pWriteDataBuffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);

		if (pWriteDataBuffer)
		{
			if (pWriteDataBuffer[strlen(pWriteDataBuffer)-1])
			{
				DbgPrint(pWriteDataBuffer);
			}
		}*/
	}
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return NtStatus;
}
//The Main DriverEntry for our basic driver. This is the code that is executed when the driver is first loaded
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
	NTSTATUS NtStatus = STATUS_SUCCESS;
	unsigned int uiIndex = 0;
	PDEVICE_OBJECT pDeviceObject = NULL;
	UNICODE_STRING usDriverName, usDosDeviceName;

	DbgPrint("DriverEntry Called \r\n");
	//create unicode strings for use later.
	RtlInitUnicodeString(&usDriverName, L"\\Device\\Example");
	RtlInitUnicodeString(&usDosDeviceName, L"\\DosDevices\\Example");
	//create our device, this will allow usermode to easily interface with the driver by treating it as a file
	NtStatus = IoCreateDevice(pDriverObject,
								0,
								&usDriverName,
								FILE_DEVICE_UNKNOWN,
								FILE_DEVICE_SECURE_OPEN,
								FALSE, &pDeviceObject);
	//fill our MajorFunctions table with a dummy function
	for (uiIndex = 0; uiIndex < IRP_MJ_MAXIMUM_FUNCTION; uiIndex++)
	{
		pDriverObject->MajorFunction[uiIndex] = Example_UnSupportedFunction;
	}
	//specific functions for specific IRP's
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = Example_Close;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = Example_Create;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = Example_IoControl;
	pDriverObject->MajorFunction[IRP_MJ_READ] = Example_Read;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = Example_Write;
	//unload function
	pDriverObject->DriverUnload = Example_Unload;
	//set IO mode for Writing, didnt use this in the end
	pDeviceObject->Flags |= DO_DIRECT_IO;
	//pDeviceObject->Flags |= DO_BUFFERED_IO;

	pDeviceObject->Flags &= (~DO_DEVICE_INITIALIZING);
	/*Tells the I/O Manager that the device is being initialized and not to send any I/O requests to the driver.
	For devices created in the context of the “DriverEntry”, this is not needed since the I/O Manager will clear this flag once the “DriverEntry” is done.
	However, if you create a device in any function outside of the DriverEntry, you need to manually clear this flag for any device you create with IoCreateDevice.
	This flag is actually set by the IoCreateDevice function.
	We cleared it here just for fun even though we weren’t required to.*/
	
	//create a symbolic link for usermode to use
	IoCreateSymbolicLink(&usDosDeviceName, &usDriverName);

	return NtStatus;
}