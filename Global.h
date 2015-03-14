#pragma once
//------------------------------------------------------
// Check windows Envrioment Bit
#if _WIN32 || _WIN64
#if _WIN64
#define ENV_BIT 64
#else
#define ENV_BIT 32
#endif
#endif
//------------------------------------------------------
//Windows Includes
#include <windows.h>
#include <string>
#include <process.h>
#include <fstream>
using namespace std;
//------------------------------------------------------
//Structures
struct PL_ENTRY{
	string filepath;
	string displayname;
	string title;
	string artist;
	string album;
	string year;
	string comment;
	char rating;

	PL_ENTRY()
	{
		filepath = "";
		displayname = "";
		title = "No Title";
		artist = "No Artist";
		album = "No Album";
		year = "No Year";
		comment = "No Comment";
		rating = -1;
	}
};
//------------------------------------------------------
//Classes
//------------------------------------------------------
//Hi Res Timer
#include "HRTimer.h"
extern HRTimer cHRTimer;
//------------------------------------------------------
//FLUID
#include "Fuzon_Mp3.h"
//------------------------------------------------------
//Math
#include "Math_System.h"
extern Math_System cMath;
//------------------------------------------------------
//Network
#include "Network_System.h"
extern Network_System cNet;
//------------------------------------------------------
//File System
#include "File_System.h"
extern File_System cFile;
//------------------------------------------------------
//Bass Audio System
#include "bass_audio.h"
extern bass_audio cBass;
//------------------------------------------------------
//Thread System
#include "Thread_System.h"
extern Thread_System cThread;
//------------------------------------------------------
//VST WINDOW
extern HWND vstWindow;
//------------------------------------------------------
//Ui Class
#include "Ui.h"
//extern Ui cUi;
//------------------------------------------------------
//Playlist Data
extern PL_ENTRY *gPlaylist_Entry;
extern char gPlaylist_Rescan_State;
extern long gPlaylist_ArraySize;
extern long gPlaylist_ArrayIncreaseStepSize;
extern long gTotalFiles;
//playlist search
extern int gPlaylistSearchCharLength;
extern int gSearchEntries;
extern string gPlaylistSearchText;
extern int gPlaylistSearchId;
//------------------------------------------------------
//Ui Vars
extern char Text_Volume[12];
extern char Text_Seekbar[64];
extern char Text_AmpGain[64];

//intro animations
extern bool gInto_Animation_Completed;
extern double gInto_Animation_Timer;
extern const double gInto_Animation_Timer_MAX;

//Bottom Panel
extern const float gBottomPanel_Target_X;
extern float gBottomPanel_Target_Y;
extern float gBottomPanel_Current_Y;
extern float gBottomPanel_TendTo;

extern int BottomPanelID;
extern double Calc_Bpm_Value;
///Window states
extern bool gWindow_PlaylistManager;
///Update Rate
extern double gET_Multi;
extern double updateTimerSpeed_User;
extern double updateTimerSpeed;
///Play ID's
extern int gPlay_CurrentId;
extern int gPlay_PreviousId;
//Options Menu
extern int gOptions_Choice_SubMenu;
//Spectrum Colour
extern unsigned char gSpectrum_R;
extern unsigned char gSpectrum_G;
extern unsigned char gSpectrum_B;
//------------------------------------------------------
//Global Timers
extern double gTimer;
extern const double gTimer_Max;
extern bool gTimer_Activated;

//------------------------------------------------------
//Debug
extern double gDebug_ScanTime;
extern double gDebug_TagTime;
extern unsigned long gDebug_Tagged_ID3V1;

//------------------------------------------------------
//Functions
extern void ReplaceString(string &Input, string Find, string Replace);
//------------------------------------------------------
