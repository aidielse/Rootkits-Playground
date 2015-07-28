//Most of this code belongs to Reverend Bill Blunden.
//It comes from his book: 'The Rootkit Arsenal: Escape and Evasion in the Dark Corners of the System' First Edition
//some modifications were mady by Aaron Sedlacek
//Written 07/05/2015
#include "datatypes.h"
#include "debug.h"
#include "ioctl_defs.h"
#include "devices.h"
//Toggles Debug logging
//#define LOG_OFF
PDEVICE_OBJECT MSNetDiagDeviceObject;
PDRIVER_OBJECT DriverObjectRef;

//IOCTL test function, demonstrates how to manipulate IO buffers (type BUFFERED)
void TestCommand(PVOID inputBuffer, PVOID outputBuffer, ULONG inputBufferLength, ULONG outputBufferLength)
{
	//ptr for data manipulation
	char *ptrBuffer;
	DBG_TRACE("dispatchIOControl", "Displaying InputBuffer");
	//print input buffer
	ptrBuffer = (char *)inputBuffer;
	DBG_PRINT2("[dispatchIOControl]: inputBuffer=%s\n", ptrBuffer);

	DBG_TRACE("dispatchIOControl", "Populating outputBuffer");
	ptrBuffer = (char *)outputBuffer;
	ptrBuffer[0] = '!';
	ptrBuffer[1] = '1';
	ptrBuffer[2] = '2';
	ptrBuffer[3] = '3';
	ptrBuffer[4] = '!';
	//DONT FORGET TO NULL TERMINATE STRINGS OR YOU'LL HAVE A BSOD!!!!1!!1!
	ptrBuffer[5] = '\0';

	DBG_PRINT2("[dispatchIOControl]: outputBuffer=%s\n", ptrBuffer);
	return;
}
//Default IRP Dispatch
NTSTATUS defaultDispatch(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIRP)
{
	((*pIRP).IoStatus).Status = STATUS_SUCCESS;
	((*pIRP).IoStatus).Information = 0;
	IoCompleteRequest(pIRP, IO_NO_INCREMENT);

	return(STATUS_SUCCESS);
}
//IOCTL dispatching function.
NTSTATUS dispatchIOControl(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIRP)
{
	PIO_STACK_LOCATION irpStack;
	PVOID inputBuffer;
	PVOID outputBuffer;
	ULONG inputBufferLength;
	ULONG outputBufferLength;
	ULONG ioctrlcode;
	NTSTATUS ntStatus;

	ntStatus = STATUS_SUCCESS;

	((*pIRP).IoStatus).Status = STATUS_SUCCESS;
	((*pIRP).IoStatus).Information = 0;
	//get input and output buffers
	inputBuffer = (*pIRP).AssociatedIrp.SystemBuffer;
	outputBuffer = (*pIRP).AssociatedIrp.SystemBuffer;
	//get the IRP stack location, we need this to get other metadata
	irpStack = IoGetCurrentIrpStackLocation(pIRP);
	//get input and output buffer length and the IOCTL code
	inputBufferLength = (*irpStack).Parameters.DeviceIoControl.InputBufferLength;
	outputBufferLength = (*irpStack).Parameters.DeviceIoControl.OutputBufferLength;
	ioctrlcode = (*irpStack).Parameters.DeviceIoControl.IoControlCode;

	DBG_TRACE("dispatchIOControl", "Received a command");
	//switch on the IOCTL code
	switch (ioctrlcode)
	{
		//Our test case
		case IOCTL_TEST_CMD:
		{
			TestCommand(inputBuffer, outputBuffer, inputBufferLength, outputBufferLength);
			((*pIRP).IoStatus).Information = outputBufferLength;
		}break;
		default:
			DBG_TRACE("dispatchIOControl", "control code not recognized");
	}	
	((*pIRP).IoStatus).Status = ntStatus;
	IoCompleteRequest(pIRP, IO_NO_INCREMENT);
	return(ntStatus);
}
//This functions registers a device for our driver
NTSTATUS RegisterDriverDeviceName(IN PDRIVER_OBJECT pDriverObject)
{
	NTSTATUS ntStatus;

	UNICODE_STRING unicodeString;
	
	RtlInitUnicodeString(&unicodeString, DeviceNameBuffer);
	//create a device, see global string DeviceNameBuffer.
	ntStatus = IoCreateDevice
		(
		pDriverObject,
		0,
		&unicodeString,
		FILE_DEVICE_RK,
		0,
		TRUE,
		&MSNetDiagDeviceObject
		);

	return ntStatus;
}
//This function registers a symbolic link for user-mode programs.
//This link allows user-mode programs to interface with our device.
NTSTATUS RegisterDriverDeviceLink()
{
	NTSTATUS ntStatus;

	UNICODE_STRING unicodeString;
	UNICODE_STRING unicodeLinkString;

	RtlInitUnicodeString(&unicodeString, DeviceNameBuffer);
	RtlInitUnicodeString(&unicodeLinkString, DeviceLinkBuffer);
	//pretty straightforward.
	ntStatus = IoCreateSymbolicLink(&unicodeLinkString,&unicodeString);
	return ntStatus;
}

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
//this is our unload function, it is called when the driver is uninstalled from memory through SCM.
void unload(IN PDRIVER_OBJECT pDriverObject)
{
	PDEVICE_OBJECT pdeviceObj;
	UNICODE_STRING unicodeString;

	DBG_TRACE("OnUnload", "Received signal to unload the driver");
	pdeviceObj = (*pDriverObject).DeviceObject;

	if (pdeviceObj != NULL)
	{
		//remove our userland sym link
		DBG_TRACE("OnUnload", "Unregistering driver's symbolic link");
		RtlInitUnicodeString(&unicodeString, DeviceLinkBuffer);
		IoDeleteSymbolicLink(&unicodeString);
		//remove our device
		DBG_TRACE("OnUnload", "Unregistering driver's device name");
		IoDeleteDevice((*pDriverObject).DeviceObject);
	}
	return;
}
//This is the main driver entry. This code is executed when the driver is first created.
NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING regPath)
{
	int i;
	NTSTATUS ntStatus;

	DBG_TRACE("Driver Entry", "Driver is Booting...");
	DBG_TRACE("Driver Entry", "Establishing dispatch table");
	//set default dispatch function for all IRP's
	for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) 
	{
		(*pDriverObject).MajorFunction[i] = defaultDispatch;
	}
	//overwrite IRP's as needed,
	(*pDriverObject).MajorFunction[IRP_MJ_CLOSE] = Example_Close;
	(*pDriverObject).MajorFunction[IRP_MJ_CREATE] = Example_Create;
	(*pDriverObject).MajorFunction[IRP_MJ_READ] = Example_Read;
	(*pDriverObject).MajorFunction[IRP_MJ_WRITE] = Example_Write;
	//this one is for IOCTL
	(*pDriverObject).MajorFunction[IRP_MJ_DEVICE_CONTROL] = dispatchIOControl;
	//set our unload function
	DBG_TRACE("Driver Entry", "Establishing other DriverObject function pointers)");
	(*pDriverObject).DriverUnload = unload;
	//register a device
	DBG_TRACE("Driver Entry", "Registering driver's device name");
	ntStatus = RegisterDriverDeviceName(pDriverObject);

	if (!NT_SUCCESS(ntStatus))
	{
		DBG_TRACE("Driver Entry", "Failed to register driver's device name");
		return ntStatus;
	}
	//create a symbolic link for userland programs to interface with us through
	DBG_TRACE("Driver Entry", "Registering driver's symbolic link");
	ntStatus = RegisterDriverDeviceLink();

	if (!NT_SUCCESS(ntStatus))
	{
		DBG_TRACE("Driver Entry", "Failed to register driver's symbolic link");
		return ntStatus;
	}
	//set global, this global is referenced when specific driver functions are called.
	DriverObjectRef = pDriverObject;

	return(STATUS_SUCCESS);
}