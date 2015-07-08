#include "hooksys.h"

void getMSR(DWORD regAddress, PMSR msr)
{
	DWORD loValue;
	DWORD hiValue;

	__asm
	{
		mov ecx, regAddress;
		rdmsr;
		mov hiValue, edx;
		mov loValue, eax;
	}

	(*msr).hiValue = hiValue;
	(*msr).loValue = loValue;
	return;
}
void setMSR(DWORD regAddress, PMSR msr)
{
	DWORD loValue;
	DWORD hiValue;

	hiValue = (*msr).hiValue;
	loValue = (*msr).loValue;

	//this section causes IRQL_LESS_THAN_OR_EQUAL 0x0000000A error
	__asm
	{
		push ecx;
		push edx;
		push eax;

		mov ecx, regAddress;
		mov edx, hiValue;
		mov eax, loValue;
		//INT 3;
		wrmsr;

		pop ecx;
		pop edx;
		pop eax;
	}
	return;
}

DWORD HookCPU(DWORD procAddress) {
	
	MSR oldMSR;
	MSR newMSR;

	getMSR(IA32_SYSENTER_EIP, &oldMSR);
	newMSR.loValue = oldMSR.loValue;
	newMSR.hiValue = oldMSR.hiValue;

	newMSR.loValue = procAddress;

	DBG_PRINT2("[HOOKCPU]: Existing IA32_SYSENTER_EIP: %08x\n", oldMSR.loValue);
	DBG_PRINT2("[HOOKCPU]: New IA32_SYSENTER_EIP: %08x\n", newMSR.loValue);

	setMSR(IA32_SYSENTER_EIP, &newMSR);

	return(oldMSR.loValue);
}

void HookAllCPUs(DWORD procAddress) {

	KeSetAffinityThreadPtr KeSetAffinityThread;
	UNICODE_STRING procName;
	KAFFINITY cpuBitMap;
	PKTHREAD pKThread;
	DWORD i = 0;

	RtlInitUnicodeString(&procName, L"KeSetAffinityThread");
	KeSetAffinityThread = (KeSetAffinityThreadPtr)MmGetSystemRoutineAddress(&procName);
	cpuBitMap = KeQueryActiveProcessors();
	pKThread = KeGetCurrentThread();

	DBG_TRACE("HookAllCPUs", "Performing a sweep of all CPUs");
	for (i = 0; i < nCPUs; i++)
	{
		KAFFINITY currentCPU = cpuBitMap & (1 << i);
		if (currentCPU != 0)
		{	
			DBG_PRINT2("[HookAllCPUs]: CPU[%u] is being hooked\n", i);
			KeSetAffinityThread(pKThread, currentCPU);
			if (originalMSRLowValue == 0)
			{
				originalMSRLowValue = HookCPU(procAddress);
			}
			else
			{
				HookCPU(procAddress);
			}
			DBG_PRINT2("[HookAllCPUs]: CPU[%u] has been hooked\n", i);
			
		}
	}

	KeSetAffinityThread(pKThread, cpuBitMap);
	PsTerminateSystemThread(STATUS_SUCCESS);
	return;
}


void HookSYSENTER(DWORD procAddress) {

	HANDLE hThread;
	OBJECT_ATTRIBUTES initializedAttributes;
	PVOID pkThread;
	LARGE_INTEGER timeout;

	InitializeObjectAttributes(&initializedAttributes, NULL, 0, NULL, NULL);
	PsCreateSystemThread(&hThread, THREAD_ALL_ACCESS, &initializedAttributes, NULL, NULL, (PKSTART_ROUTINE)HookAllCPUs, (PVOID)procAddress);

	ObReferenceObjectByHandle(hThread, THREAD_ALL_ACCESS, NULL, KernelMode, &pkThread, NULL);

	timeout.QuadPart = 500;

	while (KeWaitForSingleObject(pkThread, Executive, KernelMode, FALSE, &timeout) != STATUS_SUCCESS){/*wait*/}
	ZwClose(hThread);
	return;
}

