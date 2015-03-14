#pragma once
//-----------------------------------------------------------------
#define MAX_FILEPATH 260
#define MAX_FILEFILTERS 9
#define MAX_PLAYLIST_DIRECTORY 10
#define MAX_VST_FILES 20
//FileFilter[0] = ".mp3";
//FileFilter[1] = ".ogg";
//FileFilter[2] = ".flac";
//FileFilter[3] = ".wav";
//FileFilter[4] = ".wma";
//FileFilter[5] = ".it";
//FileFilter[6] = ".s3m";
//FileFilter[7] = ".xm";
//FileFilter[8] = ".opus";
//-----------------------------------------------------------------
class File_System
{
	//-------------------------------------------------------------
public:
	//---------------------------------------------------------
	///Application folder path
	string ProgramLocation;
	string Vis_Location_Path;
	string Vis_Location_Exe;
	string Vst_Path;
	///File browser location
	char FilePath[MAX_FILEPATH];

	//Playlist Directory Data
	string Playlist_Directory[MAX_PLAYLIST_DIRECTORY];
	int Playlist_Directory_Count;

	//VST File Locations
	string Vst_FileName[MAX_VST_FILES];
	int Vst_FileCount;
	//---------------------------------------------------------
	//constructors
	File_System(void);
	~File_System(void);
	//---------------------------------------------------------
	//functions
	void iGetExePath(void);
	void Scan_Media(string directory, string fileFilter, bool recursively);
	void Scan_Vst(void);
	bool changeDir(const char* path);
	void Init(void);
	streamoff FileExists(string filepath);
	//-------------------------------------------------------------
private:
	//---------------------------------------------------------
	//File Filters
	string FileFilter[MAX_FILEFILTERS];
	//---------------------------------------------------------
	//functions
	void Check_MediaType(string Directory, string Filename);
	void Check_VstType(string Directory, string Filename);
	//---------------------------------------------------------
};
