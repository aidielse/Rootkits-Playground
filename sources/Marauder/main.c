//Most of this code belongs to Reverend Bill Blunden.
//It comes from his book: 'The Rootkit Arsenal: Escape and Evasion in the Dark Corners of the System' First Edition
//some modifications were mady by Aaron Sedlacek
//Written 07/07/2015

#include "hooksys.h"

PDRIVER_OBJECT DriverObjectRef;
//Toggles Debug logging
//#define LOG_OFF

//Default IRP Dispatch

void __stdcall LogSystemCall(DWORD dispatchID, DWORD stackPtr)
{
	if (currentIndex == printFreq)
	{
		DbgPrint("[LogSystemCall]: on CPU[%u] of %u, (%s, pid=%u, dispatchID=%x\n");
		currentIndex = 0;
	}
	currentIndex++;
	return;
}

void __declspec(naked) KiFastSystemCallHook()
{
	__asm
	{
		pushad
			pushfd
			mov ecx, 0x23
			push 0x30
			mov ds, cx
			mov es, cx

			push edx
			push eax
			call LogSystemCall

			popfd
			popad
			jmp[originalMSRLowValue]
	}
}

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
	DBG_TRACE("OnUnload", "Received signal to unload the driver");
	//DBG_TRACE("OnUnload", "Restoring original MSR");

	//HookSYSENTER(originalMSRLowValue);
	DBG_TRACE("OnUnload", "Cleanup complete");
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
	//set our unload function
	DBG_TRACE("Driver Entry", "Establishing other DriverObject function pointers)");
	(*pDriverObject).DriverUnload = unload;
	
	//set global, this global is referenced when specific driver functions are called.
	DriverObjectRef = pDriverObject;

	//hook Sysenter
	HookSYSENTER((DWORD)KiFastSystemCallHook);
	return(STATUS_SUCCESS);
}