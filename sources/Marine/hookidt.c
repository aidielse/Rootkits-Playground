#include "hookidt.h"
#include "ntddk.h"
#include "debug.h"
//this is the target interrupt that we are hooking
#define SYSTEM_SERVICE_VECTOR 0x2e

DWORD nIDTHooked;
KEVENT syncEvent;
DWORD oldISRPtr;
//This is the function that actually gets inserted before each interrupt
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
//This function is our hook that is executed before the original interrupt code
//A naked function compiles to pure assembly
__declspec(naked) KiSystemServiceHook()
{
	__asm
	{
		//store old register values on the stack
		pushad
		//store flags
		pushfd
		push fs 
		mov bx, 0x30
		mov fs, bx
		push ds
		push es

		//set our hook code
		push edx //stackPtr
		push eax //dispatchID
		call LogSystemCall;
		//restore registers, flags, and stack to original values
		pop es
		pop ds
		pop fs
		popfd
		popad
		//jump to the correct address
		jmp oldISRPtr;
	}
}
//This function hooks the IDT of a specific processor
void HookInt2E()
{
	IDTR idtr;
	PIDT_DESCRIPTOR idt;
	PIDT_DESCRIPTOR int2eDescriptor;
	DWORD addressISR;

	DBG_PRINT2("[HookInt2E]: On CPU[%u]\n", KeGetCurrentProcessorNumber);
	DBG_TRACE("HookInt2E", "Accessing 48-bit value in IDTR");
	//get IDT base, store in idtr
	__asm
	{
		cli;
		sidt idtr;
		sti;
	}
	//conver idt base into a DWORD
	idt = (PIDT_DESCRIPTOR)makeDWORD(idtr.baseAddressHi, idtr.baseAddressLow);
	//get the address that execution will start at for this interrupt
	addressISR = makeDWORD(idt[SYSTEM_SERVICE_VECTOR].offset16_31, idt[SYSTEM_SERVICE_VECTOR].offset00_15);
	//if address = our hook (AKA already hooked)
	if (addressISR == (DWORD)KiSystemServiceHook)
	{
		DBG_TRACE("HookInt2E", "IDT ALREADY HOOKED");
		KeSetEvent(&syncEvent, 0, FALSE);
		PsTerminateSystemThread(0);
	}
	
	DBG_PRINT2("[HookInt2E]: IDT[0x2E] originally at address=%x\n", addressISR);
	//get the IDT address for INT 0x2E
	int2eDescriptor = &(idt[SYSTEM_SERVICE_VECTOR]);
	DBG_TRACE("HookInt2E", "Hooking IDT[0x2E]");

	//set our hook
	__asm
	{
		cli;
		//load our hook
		lea eax, KiSystemServiceHook;
		mov ebx, int2eDescriptor;
		//move the first half of the hook function's address into the IDT
		mov[ebx], ax;
		shr eax, 16;
		//move the second half of the hook function's address into the IDTs
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
	//get processor count
	int nProcessors = KeQueryActiveProcessorCount(NULL);

	DBG_PRINT2("%d processors detected\n", nProcessors);
	DBG_TRACE("HookAllCPUs", "Accessing 48-bit value in IDTR");
	//get IDT location
	__asm
	{
		cli;
		sidt idtr;
		sti;
	}
	//base address of IDT
	idt = (PIDT_DESCRIPTOR)makeDWORD(idtr.baseAddressHi, idtr.baseAddressLow);
	//For unload or whatever screw it i'm lazy lol
	oldISRPtr = makeDWORD(idt[SYSTEM_SERVICE_VECTOR].offset16_31, idt[SYSTEM_SERVICE_VECTOR].offset00_15);

	DBG_PRINT2("[HookAllCPUs]:nt!KiSystemService at address=%x\n", oldISRPtr);

	//We have to hook every CPU, we did this using brute force
	threadHandle = NULL;
	nIDTHooked = 0;

	DBG_TRACE("HookAllCPUs", "Launch threads until we patch every IDT");
	//initialize an event for synchro
	KeInitializeEvent(&syncEvent, SynchronizationEvent, FALSE);

	while (TRUE)
	{	//Create a thread containing Hook function
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
		//Wait for the thread we just launched
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
