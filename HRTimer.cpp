//-----------------------------------------------------------------
#include "global.h"
//#include "HRTimer.h"
//-----------------------------------------------------------------
///*****************************************************************
//GAME - THREAD - CONSTRUCTORS
///*****************************************************************
HRTimer::HRTimer(void)
{
	//-------------------------------------------------------------
	HRT_TargetTime	= 1.0 / 60.0; //60 updates a second
	HRT_DeltaTime	= 0.0;
	HRT_CurrentTime = 0.0;
	HRT_NewTime		= 0.0;
	HRT_Offset		= 0.0;
	HRT_Accumulator = 0.0;
	iFrequency		= { };
	counter			= { };
	//-------------------------------------------------------------
}

HRTimer::~HRTimer(void)
{
	//-------------------------------------------------------------
	//-------------------------------------------------------------
}
//*****************************************************************
//Init
//*****************************************************************
void HRTimer::Init(void)
{
	//initialize the counter
	QueryPerformanceFrequency(&iFrequency);
	double frequency = 1.0 / iFrequency.QuadPart;

	// get the current time
	QueryPerformanceCounter(&counter);

	HRT_NewTime = counter.QuadPart * frequency;
	HRT_CurrentTime = HRT_NewTime;
}
//*****************************************************************
//HRTimer
//*****************************************************************
void HRTimer::Update(void)
{
	//initialize the counter
	QueryPerformanceFrequency(&iFrequency);
	double frequency = 1.0 / iFrequency.QuadPart;

	// get the current time
	QueryPerformanceCounter(&counter);

	HRT_NewTime = counter.QuadPart * frequency;
	HRT_DeltaTime = HRT_NewTime - HRT_CurrentTime;
	HRT_CurrentTime = HRT_NewTime;

	///Update Accumulator
	HRT_Accumulator += HRT_DeltaTime;
	if (HRT_Accumulator >= 1.0)
		HRT_Accumulator -= 1.0;

	//Grab the offset for: Delta time vs User update time
	HRT_Offset = HRT_DeltaTime - updateTimerSpeed;
	if (abs(HRT_Offset) >= updateTimerSpeed)
		HRT_Offset = 0.0;
	//Grab our Delta multiplier (for use in tendto etc)
	gET_Multi = HRT_DeltaTime * 60.0;
}