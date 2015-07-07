#include "hookidt.h"
#include "ntddk.h"
#include "debug.h"
//this is the target interrupt that we are hooking
#define SYSTEM_SERVICE_VECTOR 0x2e

DWORD nIDTHooked;
KEVENT syncEvent;
DWORD oldISRPtr;

void LogSystemCall(DWORD dispatchID, DWORD stackPtr)
{
	DbgPrint
		(
		"[RegisterSystemCall]: CPU[%u] of %u, (%s, pid=%u, dID=%x)\n",
		KeGetCurrentProcessorNumber(),
		KeNumberProcessors,
		(BYTE *)PsGetCurrentProcess()+0x16c,
		PsGetCurrentProcessId(),
		dispatchID
	);
	return;
}
//this function merges two words into a single DWORD
DWORD makeDWORD(WORD hi, WORD lo)
{
	DWORD value;
	value = 0;
	value = value | (DWORD)hi;
	value = value << 16;
	value = value | (DWORD)lo;
	return(value);
}

__declspec(naked) KiSystemServiceHook()
{
	__asm
	{
		pushad
		pushfd
		push fs 
		mov bx, 0x30
		mov fs, bx
		push ds
		push es

		push edx
		push eax
		call LogSystemCall;

		pop es
		pop ds
		pop fs
		popfd
		popad

		jmp oldISRPtr;
	}
}

void HookInt2E()
{
	IDTR idtr;
	PIDT_DESCRIPTOR idt;
	PIDT_DESCRIPTOR int2eDescriptor;
	DWORD addressISR;

	DBG_PRINT2("[HookInt2E]: On CPU[%u]\n", KeGetCurrentProcessorNumber);
	DBG_TRACE("HookInt2E", "Accessing 48-bit value in IDTR");

	__asm
	{
		cli;
		sidt idtr;
		sti;
	}

	idt = (PIDT_DESCRIPTOR)makeDWORD(idtr.baseAddressHi, idtr.baseAddressLow);

	addressISR = makeDWORD(idt[SYSTEM_SERVICE_VECTOR].offset16_31, idt[SYSTEM_SERVICE_VECTOR].offset00_15);

	if (addressISR == (DWORD)KiSystemServiceHook)
	{
		DBG_TRACE("HookInt2E", "BZZZZZT! IDT ALREADY HOOKED");
		KeSetEvent(&syncEvent, 0, FALSE);
		PsTerminateSystemThread(0);
	}

	DBG_PRINT2("[HookInt2E]: IDT[0x2E] originally at address=%x\n", addressISR);
	int2eDescriptor = &(idt[SYSTEM_SERVICE_VECTOR]);
	DBG_TRACE("HookInt2E", "Hooking IDT[0x2E]");

	__asm
	{
		cli;
		lea eax, KiSystemServiceHook;
		mov ebx, int2eDescriptor;
		mov[ebx], ax;
		shr eax, 16;
		mov[ebx + 6], ax;

		lidt idtr;

		sti;
	}
	DBG_PRINT2("[HookInt2E]: IDT[0x2E] now at %x\n", (DWORD)KiSystemServiceHook);
	DBG_PRINT2("[HookInt2E]: Hooked CPU[%u]\n", KeGetCurrentProcessorNumber());

	nIDTHooked++;

	KeSetEvent(&syncEvent, 0, FALSE);
	PsTerminateSystemThread(0);
	return;
};
//This function hooks the IDT of every processor
//It launches threads continually in an infinite while-loop
void HookAllCPUS()
{
	HANDLE threadHandle;
	IDTR idtr;
	PIDT_DESCRIPTOR idt;

	int nProcessors = KeQueryActiveProcessorCount(NULL);

	DBG_PRINT2("%d processors detected\n", nProcessors);
	DBG_TRACE("HookAllCPUs", "Accessing 48-bit value in IDTR");
	__asm
	{
		cli;
		sidt idtr;
		sti;
	}
	idt = (PIDT_DESCRIPTOR)makeDWORD(idtr.baseAddressHi, idtr.baseAddressLow);
	oldISRPtr = makeDWORD(idt[SYSTEM_SERVICE_VECTOR].offset16_31, idt[SYSTEM_SERVICE_VECTOR].offset00_15);

	DBG_PRINT2("[HookAllCPUs]:nt!KiSystemService at address=%x\n", oldISRPtr);

	threadHandle = NULL;
	nIDTHooked = 0;

	DBG_TRACE("HookAllCPUs", "Launch threads until we patch every IDT");
	KeInitializeEvent(&syncEvent, SynchronizationEvent, FALSE);

	while (TRUE)
	{
		PsCreateSystemThread
			(
			&threadHandle,
			(ACCESS_MASK)0L,
			NULL,
			NULL,
			NULL,
			(PKSTART_ROUTINE)HookInt2E,
			NULL
			);

		KeWaitForSingleObject
			(
			&syncEvent,
			Executive,
			KernelMode,
			FALSE,
			NULL
			);
		if (nIDTHooked == nProcessors) { break; }

	}
	KeSetEvent(&syncEvent, 0, FALSE);
	DBG_PRINT2("[HookAllCPUs]: number of IDT's hooked = %x\n", nIDTHooked);
	DBG_TRACE("HookAllCPUs", "Done patching all IDTs");

	return;
}
