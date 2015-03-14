//-----------------------------------------------------------------
#include "global.h"
#include <direct.h>
//#include "File_System.h"
//-----------------------------------------------------------------
///*****************************************************************
//GAME - THREAD - CONSTRUCTORS
///*****************************************************************
File_System::File_System(void)
{
	//-------------------------------------------------------------
	ProgramLocation = "";
	Vis_Location_Path = "";
	Vis_Location_Exe = "";
	Vst_Path = "";
	///File browser location
	memset(FilePath, '\0', sizeof(FilePath));

	//File Filters
	for (int i = 0; i < MAX_FILEFILTERS; i++)
	{
		FileFilter[i] = "NULL";
	}
	FileFilter[0] = ".mp3";
	FileFilter[1] = ".ogg";
	FileFilter[2] = ".flac";
	FileFilter[3] = ".wav";
	FileFilter[4] = ".wma";
	FileFilter[5] = ".it";
	FileFilter[6] = ".s3m";
	FileFilter[7] = ".xm";
	FileFilter[8] = ".opus";

	//Playlist Directory Data
	for (int i = 0; i < MAX_PLAYLIST_DIRECTORY; i++)
	{
		Playlist_Directory[i] = "";
	}
	Playlist_Directory_Count = 0;

	//VST File Locations
	for (int i = 0; i < MAX_VST_FILES; i++)
	{
		Vst_FileName[i] = "";
	}
	Vst_FileCount = 0;
	//-------------------------------------------------------------
}

File_System::~File_System(void)
{
	//-------------------------------------------------------------
	//-------------------------------------------------------------
}
//*****************************************************************
//Init
//*****************************************************************
void File_System::Init(void)
{
	iGetExePath();
	changeDir(FilePath);
}
//*****************************************************************
//changeDir
//*****************************************************************
bool File_System::changeDir(const char* path)
{
	return (_chdir(path) == 0);
}
//*****************************************************************
//iGetExePath
//*****************************************************************
void File_System::iGetExePath(void)
{
	char result[MAX_PATH] = { '\0' };
	ProgramLocation = string(result, GetModuleFileName(NULL, result, MAX_PATH));
	if (ENV_BIT == 64)
	{
		ReplaceString(ProgramLocation, "FuzonMp3_x64.exe", "");
	}
	else if (ENV_BIT == 32)
	{
		ReplaceString(ProgramLocation, "FuzonMp3_x32.exe", "");
	}
	//Get Vis directories
	Vis_Location_Path = ProgramLocation + "Fuzon_Vis\\";
	Vis_Location_Exe = Vis_Location_Path + "FuzonMp3_Vis.exe";
	//get vst filepath
	Vst_Path = ProgramLocation + "vst\\";
}
//*****************************************************************
//Scan_Media
//*****************************************************************
void File_System::Scan_Media(string directory, string fileFilter, bool recursively)
{
	if (recursively)
		Scan_Media(directory, fileFilter, false);

	directory += "/";

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	string filter = directory + (recursively ? "*" : fileFilter);

	hFind = FindFirstFile(filter.c_str(), &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		return;
	}
	else
	{
		//Found 1st file
		if (!recursively)
		{
			//Check File Type, apply to array if ok.
			Check_MediaType(directory, string(FindFileData.cFileName));
		}
		//Find next files
		while (FindNextFile(hFind, &FindFileData) != 0 &&
			gPlaylist_Rescan_State == 0)//Scan still in progress - Required for thread stop request
		{
			if (!recursively)
			{
				//Check File Type, apply to array if ok.
				Check_MediaType(directory, string(FindFileData.cFileName));
			}
			///Goto new directory
			else
			{
				if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)>0 && FindFileData.cFileName[0] != '.')
				{
					Scan_Media(directory + string(FindFileData.cFileName), fileFilter, recursively);
				}
			}
		}

		DWORD dwError = GetLastError();
		FindClose(hFind);
		if (dwError != ERROR_NO_MORE_FILES)
		{
			//Error
		}
	}
}
//*****************************************************************
//Check_MediaType
//*****************************************************************
void File_System::Check_MediaType(string Directory, string Filename)
{
	//Set Filename to lowercase for extension filter checks
	string Filename_LowerCase = Filename;
	transform(Filename_LowerCase.begin(), Filename_LowerCase.end(), Filename_LowerCase.begin(), ::tolower);

	for (int i = 0; i < MAX_FILEFILTERS; i++)
	{
		if (Filename_LowerCase.find(FileFilter[i]) != string::npos)
		{
			//Check if array is over limit
			if (gTotalFiles >= gPlaylist_ArraySize)
			{
				///Create a temp array to hold current data
				PL_ENTRY *temp = new PL_ENTRY[gTotalFiles];
				for (long j = 0; j < gTotalFiles; j++)
				{
					temp[j] = gPlaylist_Entry[j];
				}
				///Delete and recreate our bigger array
				delete[] gPlaylist_Entry;
				gPlaylist_ArraySize += gPlaylist_ArrayIncreaseStepSize;
				gPlaylist_Entry = new PL_ENTRY[gPlaylist_ArraySize];
				///Copy data from the temp array to new bigger array
				for (long j = 0; j < gTotalFiles; j++)
				{
					gPlaylist_Entry[j] = temp[j];
				}
				///Delete the temp array
				delete[] temp;
			}

			//Add to array
			gPlaylist_Entry[gTotalFiles].filepath = Directory + Filename;
			ReplaceString(gPlaylist_Entry[gTotalFiles].filepath, "//", "/");
			gPlaylist_Entry[gTotalFiles].displayname = Filename;

			//Update UI
			Fl::lock();
			Playlist_Browser->add((const char*)gPlaylist_Entry[gTotalFiles].displayname.c_str());
			Fl::unlock();

			gTotalFiles++;
			break;
		}
	}
}
//*****************************************************************
//Scan_Vst
//*****************************************************************
void File_System::Scan_Vst(void)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	string filter = Vst_Path + "*.dll";

	hFind = FindFirstFile(filter.c_str(), &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		return;
	}
	else
	{
		//Found 1st file
		Check_VstType(Vst_Path, string(FindFileData.cFileName));
		//if (Vst_FileCount < MAX_VST_FILES)
		//{
		//	Vst_FileName[Vst_FileCount] = string(FindFileData.cFileName);
		//	Vst_FileCount++;
		//}
		//Find next files
		while (FindNextFile(hFind, &FindFileData) != 0)
		{
			Check_VstType(Vst_Path, string(FindFileData.cFileName));
			//if (Vst_FileCount < MAX_VST_FILES)
			//{
			//	Vst_FileName[Vst_FileCount] = string(FindFileData.cFileName);
			//	Vst_FileCount++;
			//}
		}

		DWORD dwError = GetLastError();
		FindClose(hFind);
		if (dwError != ERROR_NO_MORE_FILES)
		{
			//Error
		}
	}
}
//*****************************************************************
//Check_VstType
//*****************************************************************
void File_System::Check_VstType(string Directory, string Filename)
{
	string FilePath = Vst_Path + Filename;
	if (Vst_FileCount < MAX_VST_FILES)
	{
		DWORD vst = BASS_VST_ChannelSetDSP(0, (const char*)FilePath.c_str(), 0, 0);
		int vstDSP_ParamCount = BASS_VST_GetParamCount(vst);
		//Check VST compatability
		if (vstDSP_ParamCount > 0)
		{
			//Check info
			BASS_VST_INFO *Info = new BASS_VST_INFO();
			if (BASS_VST_GetInfo(vst, Info) &&
				Info->isInstrument == 0)
			{
				//Add
				Vst_FileName[Vst_FileCount] = Filename;
				Vst_FileCount++;
			}
			delete Info;
		}
		//Free Memory
		BASS_VST_ChannelRemoveDSP(0, vst);
	}
}
//*****************************************************************
//FileExists
///- If file exists, Returns the size of the file
///- else, Returns -1
//*****************************************************************
streamoff File_System::FileExists(string filepath)
{
	streamoff size = -1;
	ifstream myfile((const char*)filepath.c_str(), ios::in | ios::binary | ios::ate);
	if (myfile.is_open())
	{
		size = myfile.tellg();
	}
	myfile.close();
	return size;
}