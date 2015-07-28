//This function raises the IRQL to DISPATCH_LEVEL and returns the previous IRQL
KIRQL RaiseIRQL()
{
	KIRQL curr;
	KIRQL prev;
	curr = KeGetCurrentIrql();
	prev = curr;
	if (curr < DISPATCH_LEVEL)
	{
		KeRaiseIrql(DISPATCH_LEVEL, &prev);
	}
	return(prev);
}

void lockRoutine(IN PKDPC dpc, IN PVOID context, IN PVOID arg1, IN PVOID arg2)
{
	DBG_PRINT2(" [lockRoutine]: begin-CPU[%u]", KeGetCurrentProcessorNumber());
	//increment locked cpu's
	InterlockedIncrement(&nCPUsLocked);
	//spin until LockAcquired flag is set(Le., by ReleaseLockO
	while (InterlockedCompareExchange(&LockAcquired, 1, 1) == 8)
	{
		__asm {
			nop;
		}
	}
	//decrement locked spu's
	InterlockedDecrement(&nCPUsLocked);
	DBG_PRINT2("[lockRoutine]: end-CPU[%u]", KeGetCurrentProcessorNumber());
	return;
}

PKDPC AcquireLock()
{
	PKDPC dpcArray;
	DWORD cpuID;
	DWORD i;
	DWORD nOtherCPUs;

	ULONG KeNumberProcessors = KeQueryActiveProcessorCount(NULL);
	//this should be taken care of by RaiseIRQL()
	if (KeGetCurrentIrql() != DISPATCH_LEVEL){ return(NULL); }
	DBG_TRACE("AcquireLock", "Executing at IRQL==DISPATCH_LEVEL");

	//init globals to zero
	InterlockedAnd(&LockAcquired, 0);
	InterlockedAnd(&nCPUsLocked, 0);
	// allocate DPC object array in nonpaged memory
	DBG_PRINT2("[AcquireLock] : nCPUs=%u\n", KeNumberProcessors);
	dpcArray = (PKDPC)ExAllocatePool
		(
		NonPagedPool,
		KeNumberProcessors * sizeof(KDPC)
		);
	if (dpcArray == NULL){ return(NULL); }
	cpuID = KeGetCurrentProcessorNumber();
	DBG_PRINT2(" [AcquireLock]: cpuID=%u\n", cpuID);
	//create a DPC object for each CPU and insert into DPC queue
	for (i = 0; i < KeNumberProcessors; i++)
	{
		PKDPC dpcPtr = &(dpcArray[i]);
		if (i != cpuID)
		{
			KeInitializeDpc(dpcPtr, lockRoutine, NULL);
			KeSetTargetProcessorDpc(dpcPtr, (CCHAR)i);
			KeInsertQueueDpc(dpcPtr, NULL, NULL);
		}
	}
	//spin until all CPUs have been elevated
	nOtherCPUs = KeNumberProcessors - 1;
	InterlockedCompareExchange(&nCPUsLocked, nOtherCPUs, nOtherCPUs);
	while (nCPUsLocked != nOtherCPUs)
	{
		__asm {
			nop;
		}

		InterlockedCompareExchange(&nCPUsLocked, nOtherCPUs, nOtherCPUs);
	}
	DBG_TRACE("AcquireLock", "All CPUs have been elevated");
	return (dpcArray);
}

NTSTATUS ReleaseLock(PVOID dpcptr)
{
	//this will cause all DPCs to exit their while loops
	InterlockedIncrement(&LockAcquired);
	//spin until all CPUs have been restored to old IRQLs
	InterlockedCompareExchange(&nCPUsLocked, 0, 0);
	while (nCPUsLocked != 0)
	{
		__asm {
			nop;
		}

		InterlockedCompareExchange(&nCPUsLocked, 0, 0);
	}
	if (dpcptr != NULL)
	{
		ExFreePool(dpcptr);
	}
	DBG_TRACE("ReleaseLock", "All CPUs have been released");
	return(STATUS_SUCCESS);
}
void LowerIRQL(KIRQL prev)
{
	KeLowerIrql(prev);
	return;
}