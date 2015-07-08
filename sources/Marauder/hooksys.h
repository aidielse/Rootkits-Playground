#include "ntddk.h"
#include "datatypes.h"
#include "debug.h"

#define IA32_SYSENTER_EIP 0x176
#define nCPUs 32
typedef struct _MSR
{
	DWORD loValue;
	DWORD hiValue;
}MSR, *PMSR;

typedef NTSTATUS (__stdcall * KeSetAffinityThreadPtr)(PKTHREAD thread, KAFFINITY affinity);

DWORD nActiveProcessors;
DWORD printFreq;
DWORD currentIndex;
DWORD originalMSRLowValue;

void __stdcall LogSystemCall(DWORD dispatchID, DWORD stackPtr);
void getMSR(DWORD regAddress, PMSR msr);
void setMSR(DWORD regAddress, PMSR msr);
DWORD HookCPU(DWORD procAddress);
void HookAllCPUs(DWORD procAddress);
void HookSYSENTER(DWORD procAddress);




