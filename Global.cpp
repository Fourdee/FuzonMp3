//------------------------------------------------------
#include "Global.h"
//------------------------------------------------------
//Hi Res Timer
HRTimer cHRTimer;
//------------------------------------------------------
//Math
Math_System cMath;
//------------------------------------------------------
//File System
File_System cFile;
//------------------------------------------------------
//Network
Network_System cNet;
//------------------------------------------------------
// Bass Audio System
bass_audio cBass;
//------------------------------------------------------
//Thread System
Thread_System cThread;
//------------------------------------------------------
//Playlist Data (dynamic array)
PL_ENTRY *gPlaylist_Entry = NULL;
char gPlaylist_Rescan_State = -1;
long gPlaylist_ArraySize = 0;
long gPlaylist_ArrayIncreaseStepSize = 200;
long gTotalFiles = 0;
//playlist search
int gPlaylistSearchCharLength = 3;//minimum size of search text before search system is activated.
int gSearchEntries = 0;
string gPlaylistSearchText = "";
int gPlaylistSearchId = 0;
//------------------------------------------------------
//VST WINDOW
HWND vstWindow = NULL;
//------------------------------------------------------
//Ui Vars
char Text_Volume[12] = { '\0' };
char Text_Seekbar[64] = { '\0' };
char Text_AmpGain[64] = { '\0' };

//intro animations
bool gInto_Animation_Completed = false;
double gInto_Animation_Timer = 0.0;
const double gInto_Animation_Timer_MAX = 1.6;


//Bottom Panel
const float gBottomPanel_Target_X = 361.0f;
float gBottomPanel_Target_Y = 404.0f;
float gBottomPanel_Current_Y = 404.0f;
float gBottomPanel_TendTo = 0.2f;

int BottomPanelID = 0;
double Calc_Bpm_Value = 1.0;
///Window states
bool gWindow_PlaylistManager = false;
///Update Rate
double gET_Multi = 1.0;
double updateTimerSpeed_User = 49.0;
double updateTimerSpeed = 1.0 / updateTimerSpeed_User; // x times a second
///Play ID's
int gPlay_CurrentId = -1;
int gPlay_PreviousId = -1;
//Options Menu
int gOptions_Choice_SubMenu = 0;
//Spectrum Colour
unsigned char gSpectrum_R = 255;
unsigned char gSpectrum_G = 25;
unsigned char gSpectrum_B = 0;
//------------------------------------------------------
//Global Timers
double gTimer = 0.0f;
const double gTimer_Max = 0.5f; //1.0f = 1 second
bool gTimer_Activated = false;
//------------------------------------------------------
//Debug
double gDebug_ScanTime = 0.0;
double gDebug_TagTime = 0.0;
unsigned long gDebug_Tagged_ID3V1 = 0;


//------------------------------------------------------
//Global Functions
void ReplaceString(string &Input, string Find, string Replace)
{
    size_t pos = 0;
	while ((pos = Input.find(Find, pos)) != string::npos)
	{
		Input.replace(pos, Find.length(), Replace);
		pos += Replace.length();
    }
}
//------------------------------------------------------
