#pragma once
//-----------------------------------------------------------------
#define MAX_THREADS 3
unsigned __stdcall Thread_0(void*);
unsigned __stdcall Thread_1(void*);
unsigned __stdcall Thread_2(void*);
//-----------------------------------------------------------------
class Thread_System
{
	//-------------------------------------------------------------
public:
	//---------------------------------------------------------
	bool						bActive[MAX_THREADS];
	HANDLE						Thread_Handle[MAX_THREADS];
	unsigned					Thread_Id[MAX_THREADS];
	//---------------------------------------------------------
	//constructors
	Thread_System(void);
	~Thread_System(void);
	//---------------------------------------------------------
	bool						bStopRequest[MAX_THREADS];
	//---------------------------------------------------------
	//functions
	void Start(int Id);
	void Stop(int Id);
	void Exit(void);
	//-------------------------------------------------------------
};
