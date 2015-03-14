//-----------------------------------------------------------------
#include "Global.h"
//#include "Fuzon_Mp3.h"
//-----------------------------------------------------------------
///*****************************************************************
//GAME - THREAD - CONSTRUCTORS
///*****************************************************************
Thread_System::Thread_System(void)
{
	//-------------------------------------------------------------
	for (int i = 0; i<MAX_THREADS; i++)
	{
		bActive[i] = false;
		Thread_Handle[i] = NULL;
		Thread_Id[i] = i + 1;
		bStopRequest[i] = false;
	}
	//-------------------------------------------------------------
}

Thread_System::~Thread_System(void)
{
	//-------------------------------------------------------------
	//-------------------------------------------------------------
}
//*****************************************************************
//START
//*****************************************************************
void Thread_System::Start(int Id)
{
	//-------------------------------------------------------------
	if (!bActive[Id])
	{
		bStopRequest[Id] = false;
		//
		if (Id == 0)
		{
			Thread_Handle[Id] = (HANDLE)_beginthreadex(NULL, 0, &Thread_0, NULL, 0, &Thread_Id[Id]);
			SetPriorityClass(Thread_Handle[Id], ABOVE_NORMAL_PRIORITY_CLASS);
		}
		//
		else if (Id == 1)
		{
			Thread_Handle[Id] = (HANDLE)_beginthreadex(NULL, 0, &Thread_1, NULL, 0, &Thread_Id[Id]);
			SetPriorityClass(Thread_Handle[Id], BELOW_NORMAL_PRIORITY_CLASS);
		}
		//
		else if (Id == 2)
		{
			Thread_Handle[Id] = (HANDLE)_beginthreadex(NULL, 0, &Thread_2, NULL, 0, &Thread_Id[Id]);
			SetPriorityClass(Thread_Handle[Id], NORMAL_PRIORITY_CLASS);
		}
		bActive[Id] = true;
	}
	//-------------------------------------------------------------
}
//*****************************************************************
//STOP
//*****************************************************************
void Thread_System::Stop(int Id)
{
	//-------------------------------------------------------------
	//Stop active
	if (bActive[Id])
	{
		bStopRequest[Id] = true;
		WaitForSingleObject(Thread_Handle[Id], INFINITE);
		//WaitForSingleObject(Thread_Handle[Id], 1000);
		bActive[Id] = false;
		CloseHandle(Thread_Handle[Id]);
	}
	//-------------------------------------------------------------
}
//*****************************************************************
//EXIT
//*****************************************************************
void Thread_System::Exit(void)
{
	//-------------------------------------------------------------
	for (int i = 0; i < MAX_THREADS; i++)
	{
		Stop(i);
	}
	//-------------------------------------------------------------
}
//*****************************************************************
///Thread 0 - Scan Filesystem for music
//*****************************************************************
unsigned __stdcall Thread_0(void*)
{
	//-------------------------------------------------------------
	while (cThread.bActive[0])
	{
		//Find Files
		for (int i = 0; i < MAX_PLAYLIST_DIRECTORY; i++)
		{
			//Stop request, break
			if (cThread.bStopRequest[0])
			{
				break;
			}

			if (cFile.Playlist_Directory[i] != "")
			{
				cFile.Scan_Media(cFile.Playlist_Directory[i], "*.*", true);
			}
		}
		cThread.bActive[0] = false;
	}
	return 0;
	//-------------------------------------------------------------
}
//*****************************************************************
///Thread 1 - Tag Thread
//*****************************************************************
unsigned __stdcall Thread_1(void*)
{
	//-------------------------------------------------------------
	while (cThread.bActive[1])
	{
		//TagCompletionPercent = gTotalFiles - i
		for (long i = 0; i < gTotalFiles; i++)
		{
			//Stop request, break
			if (cThread.bStopRequest[1])
			{
				break;
			}

			if (gPlaylist_Entry[i].filepath != "")
			{
				cBass.ObtainTags(i, gPlaylist_Entry[i]);
			}
		}
		cThread.bActive[1] = false;
	}
	return 0;
	//-------------------------------------------------------------
}
//*****************************************************************
///Thread 2 - 
//*****************************************************************
unsigned __stdcall Thread_2(void*)
{
	//-------------------------------------------------------------
	while (cThread.bActive[2])
	{
		//cupdate.()
		//Stop request, break
		//if (cThread.bStopRequest[2])
		//{
		//	break;
		//}
		cThread.bActive[2] = false;
	}
	return 0;
	//-------------------------------------------------------------
}