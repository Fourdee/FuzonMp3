//-----------------------------------------------------------------
#include "global.h"
//#include "bass_audio.h"
//-----------------------------------------------------------------
///*****************************************************************
//bass_audio
///*****************************************************************
bass_audio::bass_audio(void)
{
	//-------------------------------------------------------------
	//HardWare Devices
	for (int i = 0; i < MAX_HARDWARE_DEVICES; i++)
	{
		Hardware_Device_Info[i] = {};
	}
	Hardware_Devices_Total = 0;
	Hardware_Device_Selected = 1;

	Hardware_Info = {};
	//-------------------------------------------------------------
	//Core
	bBassInitilized = false;
	Stream = NULL;
	Stream_State = NULL;
	FileLength = 1.0;
	Current_Position = 0.0;
	EndOfFileState = -1; // -1 = not played, 0 = playing, 1= end of file reached
	//-------------------------------------------------------------
	//User Options
	Stream_Volume = 100; ///10000 (multipled on output)
	bShuffle_Enabled = false;
	fSpectrum_Falloff_TendTo = 0.05f;
	//--------------------------`-----------------------------------
	//DSP
	///vst
	vstDSP = NULL;
	vstDSP_ParamCount = 0;
	fVst_UserParam = NULL;
	bUser_Vst_Enabled = false;
	memset(Vst_Selected_Name, '\0', sizeof(Vst_Selected_Name));
	bVst_EmbedRequested = false;
	Vst_Selected_SaveData_FilePath = "";
	Vst_Info = NULL;


	bUser_Eq8_Enabled = false;
	Eq_Reset();
	//-------------------------------------------------------------
	//Spectrum FFT
	bFFT_Enabled = true;
	Spectrum_Reset();
	//-------------------------------------------------------------
}

bass_audio::~bass_audio(void)
{
	//-------------------------------------------------------------
	//-------------------------------------------------------------
}
///*****************************************************************
//Init
///*****************************************************************
void bass_audio::Init(void)
{
	//-------------------------------------------------------------
	//Get Device Hardware Info
	for (int i = 0; i < MAX_HARDWARE_DEVICES; i++)
	{
		if (BASS_GetDeviceInfo(i + 1, &Hardware_Device_Info[i]))
		{
		}
		else
		{
			Hardware_Devices_Total = i;
			break;
		}
	}
	//-------------------------------------------------------------
	//Load Settings
	Settings_Load();
	//-------------------------------------------------------------
	//init BASS
	////Check if previously selected hardware device is supported
	if (!BASS_Init(Hardware_Device_Selected, 44100, BASS_DEVICE_FREQ, NULL, NULL))
	{
		Hardware_Device_Selected = 1;
		bBassInitilized = BASS_Init(Hardware_Device_Selected, 44100, 0, NULL, NULL);
	}
	//Get Supported Info on Current Device
	BASS_GetInfo(&Hardware_Info);
	
	BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, Stream_Volume * 100);
	BASS_SetConfig(BASS_CONFIG_FLOATDSP, true);
	BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD,50);		
	BASS_SetConfig(BASS_CONFIG_SRC, 3);
	//BASS_SetConfig(BASS_CONFIG_BUFFER, 1000);
	///Additional Plugins
	BASS_PluginLoad("bassflac.dll", NULL);
	BASS_PluginLoad("basswma.dll", NULL);
	BASS_PluginLoad("bassopus.dll", NULL);
	//-------------------------------------------------------------
	//Scan VST directory
	cFile.Scan_Vst();
	if (cFile.Vst_FileCount == 0)
	{
		memset(Vst_Selected_Name, '\0', sizeof(Vst_Selected_Name));
	}
	//-------------------------------------------------------------
	//Start DSP - for VST setup
	DSP_Create();
	//-------------------------------------------------------------
}
///*****************************************************************
//Exit
///*****************************************************************
void bass_audio::Exit(void)
{
	//-------------------------------------------------------------
	DSP_Destroy();
	//-------------------------------------------------------------
	BASS_Free();
	bBassInitilized = false;
	//-------------------------------------------------------------
	Settings_Save();
	//-------------------------------------------------------------
}
///*****************************************************************
///Settings_Save
///*****************************************************************
void bass_audio::Settings_Save(void)
{
	//-------------------------------------------------------------
	HANDLE hFile;
	DWORD dwDone;
	string FilePath = cFile.ProgramLocation + "settings.bin";

	hFile = CreateFile((const char*)FilePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		//---------------------------------------------------------
		//Hardware		
		WriteFile(hFile, &Hardware_Device_Selected, sizeof(int), &dwDone, NULL);
		
		//Audio
		WriteFile(hFile, &Stream_Volume, sizeof(int), &dwDone, NULL);

		//DSP
		///EQ
		WriteFile(hFile, &bUser_Eq8_Enabled, sizeof(bool), &dwDone, NULL);
		for (int i = 0; i < MAX_EQ_BANDS; i++)
		{
			WriteFile(hFile, &Eq8Param[i], sizeof(BASS_BFX_BQF), &dwDone, NULL);
		}
		///VST
		WriteFile(hFile, &bUser_Vst_Enabled, sizeof(bool), &dwDone, NULL);
		WriteFile(hFile, &Vst_Selected_Name, sizeof(Vst_Selected_Name), &dwDone, NULL);

		//Shuffle
		WriteFile(hFile, &bShuffle_Enabled, sizeof(bool), &dwDone, NULL);

		//UI
		WriteFile(hFile, &bFFT_Enabled, sizeof(bool), &dwDone, NULL);
		WriteFile(hFile, &gSpectrum_R, sizeof(unsigned char), &dwDone, NULL);
		WriteFile(hFile, &gSpectrum_G, sizeof(unsigned char), &dwDone, NULL);
		WriteFile(hFile, &gSpectrum_B, sizeof(unsigned char), &dwDone, NULL);

		WriteFile(hFile, &updateTimerSpeed_User, sizeof(double), &dwDone, NULL);
		WriteFile(hFile, &fSpectrum_Falloff_TendTo, sizeof(float), &dwDone, NULL);

		//Playlist
		WriteFile(hFile, &cFile.Playlist_Directory_Count, sizeof(int), &dwDone, NULL);
		for (int i = 0; i < MAX_PLAYLIST_DIRECTORY; i++)
		{
			char buffer[MAX_FILEPATH] = { '\0' };
			strcpy_s(buffer, cFile.Playlist_Directory[i].c_str());
			WriteFile(hFile, &buffer, MAX_FILEPATH, &dwDone, NULL);
		}

		//---------------------------------------------------------
		CloseHandle(hFile);
	}
	//-------------------------------------------------------------
}
///*****************************************************************
///Settings_Load
///*****************************************************************
void bass_audio::Settings_Load(void)
{
	//-------------------------------------------------------------
	//Load Data
	HANDLE hFile;
	DWORD dwDone;
	string FilePath = cFile.ProgramLocation + "settings.bin";

	hFile = CreateFile((const char*)FilePath.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		//---------------------------------------------------------
		//Hardware		
		ReadFile(hFile, &Hardware_Device_Selected, sizeof(int), &dwDone, NULL);
		//Audio
		ReadFile(hFile, &Stream_Volume, sizeof(int), &dwDone, NULL);

		//DSP
		///EQ
		ReadFile(hFile, &bUser_Eq8_Enabled, sizeof(bool), &dwDone, NULL);
		for (int i = 0; i < MAX_EQ_BANDS; i++)
		{
			ReadFile(hFile, &Eq8Param[i], sizeof(BASS_BFX_BQF), &dwDone, NULL);
		}
		///VST
		ReadFile(hFile, &bUser_Vst_Enabled, sizeof(bool), &dwDone, NULL);
		ReadFile(hFile, &Vst_Selected_Name, sizeof(Vst_Selected_Name), &dwDone, NULL);

		//Shuffle
		ReadFile(hFile, &bShuffle_Enabled, sizeof(bool), &dwDone, NULL);

		//UI
		ReadFile(hFile, &bFFT_Enabled, sizeof(bool), &dwDone, NULL);
		ReadFile(hFile, &gSpectrum_R, sizeof(unsigned char), &dwDone, NULL);
		ReadFile(hFile, &gSpectrum_G, sizeof(unsigned char), &dwDone, NULL);
		ReadFile(hFile, &gSpectrum_B, sizeof(unsigned char), &dwDone, NULL);
		ReadFile(hFile, &updateTimerSpeed_User, sizeof(double), &dwDone, NULL);
		ReadFile(hFile, &fSpectrum_Falloff_TendTo, sizeof(float), &dwDone, NULL);

		//Playlist
		ReadFile(hFile, &cFile.Playlist_Directory_Count, sizeof(int), &dwDone, NULL);
		for (int i = 0; i < MAX_PLAYLIST_DIRECTORY; i++)
		{
			char buffer[MAX_FILEPATH] = { 0 };
			ReadFile(hFile, &buffer, MAX_FILEPATH, &dwDone, NULL);
			cFile.Playlist_Directory[i] = string(buffer);
		}
		//---------------------------------------------------------
		CloseHandle(hFile);
	}
	//-------------------------------------------------------------
}
///*****************************************************************
//SetVolume
///*****************************************************************
void bass_audio::SetVolume(int inVolume)
{
	//-------------------------------------------------------------
    Stream_Volume = inVolume;
    BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, Stream_Volume * 100);
	//-------------------------------------------------------------
}
///*****************************************************************
//StreamCreate
///*****************************************************************
void bass_audio::StreamCreate(const char* filepath)
{
	//-------------------------------------------------------------
	//Clear DSP (vst save info etc)
	DSP_Destroy();

	//Release Streams
	EndOfFileState = 0;
	
	BASS_StreamFree(Stream);
	BASS_MusicFree(Stream);

	//Create Stream_Decode
	if (!(Stream = BASS_StreamCreateFile(FALSE, filepath, 0, 0, BASS_SAMPLE_FLOAT | BASS_ASYNCFILE | BASS_STREAM_DECODE)) &&
		!(Stream = BASS_MusicLoad(FALSE, filepath, 0, 0, BASS_MUSIC_PRESCAN | BASS_MUSIC_SINCINTER | BASS_MUSIC_RAMP | BASS_SAMPLE_FLOAT | BASS_STREAM_DECODE, 0)))
	{
		///Error, unable to load file
	}
	///Create Output stream
	Stream = BASS_FX_TempoCreate(Stream, BASS_SAMPLE_FLOAT | BASS_FX_FREESOURCE); ///| BASS_SAMPLE_LOOP
	//Set Volume?
	//BASS_ChannelSetAttribute(Stream, BASS_ATTRIB_VOL, 5.98f);


	//Apply DSP if active
	DSP_Create();
	//Obtain File length
	StreamGetLength();
	//Reset Spectrums
	Spectrum_Reset();
	//-------------------------------------------------------------
}
///*****************************************************************
//StreamStop
///*****************************************************************
void bass_audio::StreamStop(void)
{
	//-------------------------------------------------------------
	//Set state to Not played
	EndOfFileState = -1;
	//Stop channel
	BASS_ChannelStop(Stream);
	//-------------------------------------------------------------
}
///*****************************************************************
//StreamGetState
///*****************************************************************
void bass_audio::StreamGetState(void)
{
	//-------------------------------------------------------------
	//Check for current state
	Stream_State = BASS_ChannelIsActive(Stream);
	//-------------------------------------------------------------
}
///*****************************************************************
//StreamSetPosition
///*****************************************************************
void bass_audio::StreamSetPosition(double Position)
{
	//-------------------------------------------------------------
	BASS_ChannelSetPosition(Stream, BASS_ChannelSeconds2Bytes(Stream, Position), BASS_POS_BYTE);
	StreamGetPosition();
	//-------------------------------------------------------------
}
///*****************************************************************
//StreamGetLength
///*****************************************************************
void bass_audio::StreamGetLength(void)
{
	//-------------------------------------------------------------
    QWORD Length = BASS_ChannelGetLength(Stream, BASS_POS_BYTE);
    FileLength = BASS_ChannelBytes2Seconds(Stream, Length);
	//-------------------------------------------------------------
}
///*****************************************************************
//StreamGetPosition
///*****************************************************************
void bass_audio::StreamGetPosition(void)
{
	//-------------------------------------------------------------
	StreamGetState();
	QWORD Position = BASS_ChannelGetPosition(Stream, BASS_POS_BYTE);
	Current_Position = BASS_ChannelBytes2Seconds(Stream, Position);
	//-------------------------------------------------------------
	//Check for end of file
	if (EndOfFileState == 0 &&
		Stream_State == BASS_ACTIVE_STOPPED &&
		gTotalFiles > 0)
	{
		EndOfFileState = 1;
	}
	//-------------------------------------------------------------
	//FFT Spectrum
	if (gInto_Animation_Completed)
	{
		///Update
		if (bFFT_Enabled)
		{
			Spectrum_Update();
		}
	}
	//-------------------------------------------------------------
}
///*****************************************************************
//Eq_Reset
///*****************************************************************
void bass_audio::Eq_Reset(void)
{
	//-------------------------------------------------------------
	//Clear
	for (int i = 0; i < MAX_EQ_BANDS; i++)
	{
		//Eq8Fx[i] = NULL;
		Eq8Param[i].lFilter = BASS_BFX_BQF_PEAKINGEQ;
		Eq8Param[i].fCenter = 80.0f;
		Eq8Param[i].lChannel = BASS_BFX_CHANALL;
		Eq8Param[i].fBandwidth = 2.0f;
		//Eq8Param[i].fQ = 0.25f;
		Eq8Param[i].fGain = 0.0f;
	}
	///Bell
	Eq8Param[0].fCenter = 63.0f;
	Eq8Param[1].fCenter = 125.0f;
	Eq8Param[2].fCenter = 250.0f;
	Eq8Param[3].fCenter = 500.0f;
	Eq8Param[4].fCenter = 1000.0f;
	Eq8Param[5].fCenter = 2000.0f;
	Eq8Param[6].fCenter = 4000.0f;
	Eq8Param[7].fCenter = 8000.0f;
	///High pass (eg: Low cut)
	Eq8Param[8].lFilter = BASS_BFX_BQF_HIGHPASS;
	//Eq8Param[8].fGain = -15.0f;
	//Eq8Param[8].fBandwidth = 1.0f;
	Eq8Param[8].fQ = 0.5f;
	//Eq8Param[8].fS = 0.1f;
	Eq8Param[8].fCenter = 20.0f;
	//-------------------------------------------------------------
}
///*****************************************************************
//DSP_Create
///*****************************************************************
void bass_audio::DSP_Create(void)
{
	//-------------------------------------------------------------
	//Create BASS_FX DSP
	///EQ
	if (bUser_Eq8_Enabled)
	{
		for (int i = 0; i < MAX_EQ_BANDS; i++)
		{
			Eq8Fx[i] = BASS_ChannelSetFX(Stream, BASS_FX_BFX_BQF, 1 + i);
		}
	}
	//-------------------------------------------------------------
	//VST DSP
	if (bUser_Vst_Enabled)
	{
		//check a VST is selected
		if (Vst_Selected_Name[0] >= 32)
		{
			string Location = cFile.Vst_Path + string(Vst_Selected_Name);
			vstDSP = BASS_VST_ChannelSetDSP(Stream, (const char*)Location.c_str(), 0, 0);
			vstDSP_ParamCount = BASS_VST_GetParamCount(vstDSP);
			//Get info
			Vst_Info = new BASS_VST_INFO;
			BASS_VST_GetInfo(vstDSP, Vst_Info);

			//generate float array
			fVst_UserParam = new float[vstDSP_ParamCount];
			for (int i = 0; i < vstDSP_ParamCount; i++)
			{
				fVst_UserParam[i] = BASS_VST_GetParam(vstDSP, i);
			}
				
			//Check existing save file for settings
			Vst_Selected_SaveData_FilePath = cFile.Vst_Path + string(Vst_Selected_Name);
			ReplaceString(Vst_Selected_SaveData_FilePath, ".dll", ".bin");
			HANDLE hFile = CreateFile((const char*)Vst_Selected_SaveData_FilePath.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			DWORD dwDone;
			if (hFile != INVALID_HANDLE_VALUE)
			{
				for (int i = 0; i < vstDSP_ParamCount; i++)
				{
					ReadFile(hFile, &fVst_UserParam[i], sizeof(float), &dwDone, NULL);
					BASS_VST_SetParam(vstDSP, i, fVst_UserParam[i]);
				}
			}
			CloseHandle(hFile);
		}
		else
		{
			bUser_Vst_Enabled = false;
		}
	}
	//-------------------------------------------------------------
	//Tell UI we need to embed VST to window
	bVst_EmbedRequested = true;
	//-------------------------------------------------------------
	//Update
	DSP_Update();
	//-------------------------------------------------------------
}
///*****************************************************************
//DSP_Destroy
///*****************************************************************
void bass_audio::DSP_Destroy(void)
{
	//-------------------------------------------------------------
	//Save VST Settings
	if (vstDSP_ParamCount > 0)//Ensure Vst is loaded
	{
		HANDLE hFile = CreateFile((const char*)Vst_Selected_SaveData_FilePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		DWORD dwDone;

		if (hFile != INVALID_HANDLE_VALUE)
		{
			for (int i = 0; i < vstDSP_ParamCount; i++)
			{
				fVst_UserParam[i] = BASS_VST_GetParam(vstDSP, i);
				WriteFile(hFile, &fVst_UserParam[i], sizeof(float), &dwDone, NULL);
			}
		}
		CloseHandle(hFile);

		//Clear arrays
		delete[] Vst_Info;
		delete[] fVst_UserParam;
	}
	//-------------------------------------------------------------
	//Clear All Current DSP's
	for (int i = 0; i < MAX_EQ_BANDS; i++)
	{
		BASS_ChannelRemoveFX(Stream, Eq8Fx[i]);
	}
	BASS_VST_ChannelRemoveDSP(Stream, vstDSP);
	vstDSP_ParamCount = 0;
}
///*****************************************************************
//Vst_Reset
///*****************************************************************
void bass_audio::Vst_Reset(void)
{
	//-------------------------------------------------------------
	if (bUser_Vst_Enabled &&
		vstDSP_ParamCount > 0)
	{
		//BASS_VST_INFO info = {};
		//BASS_VST_GetInfo(vstDSP, &info);

		BASS_VST_PARAM_INFO Vst_Param_Info = {};
		for (int i = 0; i < vstDSP_ParamCount; i++)
		{
			BASS_VST_GetParamInfo(vstDSP, i, &Vst_Param_Info);
			fVst_UserParam[i] = Vst_Param_Info.defaultValue;
			BASS_VST_SetParam(vstDSP, i, fVst_UserParam[i]);
		}
		////Delete Vst Data file
		//string Location_VstSettings = cFile.Vst_Path + string(Vst_Selected_Name);
		//ReplaceString(Location_VstSettings, ".dll", ".bin");
		//DeleteFile((const char*)Location_VstSettings.c_str());
	}
	//-------------------------------------------------------------
}
///*****************************************************************
//DSP_Update
///*****************************************************************
void bass_audio::DSP_Update(void)
{
	//-------------------------------------------------------------
	//Update DSP
	///EQ
	for (int i = 0; i < MAX_EQ_BANDS; i++)
	{
		BASS_FXSetParameters(Eq8Fx[i], &Eq8Param[i]);
	}
	//-------------------------------------------------------------
}
///*****************************************************************
//Play
///*****************************************************************
void bass_audio::Play(void)
{
	//-------------------------------------------------------------
	//Check for current state
	StreamGetState();
	switch (Stream_State)
	{
		//Play
		case BASS_ACTIVE_STOPPED:
			BASS_ChannelPlay(Stream, TRUE);
			//ian To avoid a delay when changing files, you can clear that playback buffer by calling BASS_ChannelPlay with restart=TRUE or BASS_ChannelSetPosition with pos=0.
            StreamGetPosition();
			break;
		//Resume
		case BASS_ACTIVE_PAUSED:
			BASS_ChannelPlay(Stream, FALSE);
            StreamGetPosition();
			break;
		//Pause
		case BASS_ACTIVE_PLAYING:
			BASS_ChannelPause(Stream);
            StreamGetPosition();
			break;
		default:
			break;
	}
	StreamGetState();
	//-------------------------------------------------------------
}
///*****************************************************************
//ObtainTags
///*****************************************************************
void bass_audio::ObtainTags(int Index, PL_ENTRY &entry)
{
	//-------------------------------------------------------------
	//Convert Filepath to lowercase (for extension check)
	string Filename_LowerCase = entry.filepath;
	transform(Filename_LowerCase.begin(), Filename_LowerCase.end(), Filename_LowerCase.begin(), ::tolower);
	//-------------------------------------------------------------
	//MP3
	if (Filename_LowerCase.find(".mp3") != string::npos)
	{
		//ID3_V1
		static char Buffer[31] = { '\0' };

		ifstream myfile((const char*)entry.filepath.c_str(), ios::in | ios::binary | ios::ate);
		streamoff size = myfile.tellg();
		myfile.seekg(size - 128);
		memset(Buffer, '\0', sizeof(Buffer));
		myfile.read(Buffer, 3);
		if (Buffer[0] == 'T' &&
			Buffer[1] == 'A' &&
			Buffer[2] == 'G')
		{
			gDebug_Tagged_ID3V1++;
			//Title
			myfile.seekg(size - 125);
			memset(Buffer, '\0', sizeof(Buffer));
			myfile.read(Buffer, 30);
			for (char i = sizeof(Buffer)-1; i > 0; i--)
			{
				if (Buffer[i] <= 32 ||///Clear These
					Buffer[i] == 94)///FLTK uses ^J as a new line
				{
					Buffer[i] = '\0';
				}
				else if (i>0)
				{
					//Add tag entry
					entry.title = string(Buffer);
					break;
				}
			}
			//Artist
			myfile.seekg(size - 95);
			memset(Buffer, '\0', sizeof(Buffer));
			myfile.read(Buffer, 30);
			for (char i = sizeof(Buffer)-1; i > 0; i--)
			{
				if (Buffer[i] <= 32 ||///Clear These
					Buffer[i] == 94)///FLTK uses ^J as a new line
				{
					Buffer[i] = '\0';
				}
				else if (i>0)
				{
					//Add tag entry
					entry.artist = string(Buffer);
					break;
				}
			}
			//Album
			myfile.seekg(size - 65);
			memset(Buffer, '\0', sizeof(Buffer));
			myfile.read(Buffer, 30);
			for (char i = sizeof(Buffer)-1; i > 0; i--)
			{
				if (Buffer[i] <= 32 ||///Clear These
					Buffer[i] == 94)///FLTK uses ^J as a new line
				{
					Buffer[i] = '\0';
				}
				else if (i>0)
				{
					entry.album = string(Buffer);
					break;
				}
			}
			//Year
			myfile.seekg(size - 35);
			memset(Buffer, '\0', sizeof(Buffer));
			myfile.read(Buffer, 4);
			bool bHasYear = true;
			for (char i = 0; i < 4; i++)
			{
				if (Buffer[i] < '0' || ///Check its a digit
					Buffer[i] > '9')
				{
					bHasYear = false;
					break;
				}
			}
			if (bHasYear)
			{
				entry.year = string(Buffer);
			}
			//Comment
			myfile.seekg(size - 31);
			memset(Buffer, 0, sizeof(Buffer));
			myfile.read(Buffer, 30);
			for (char i = sizeof(Buffer)/* + 1*/; i > 0; i--)
			{
				if (Buffer[i] <= 32 ||///Clear These
					Buffer[i] == 94)///FLTK uses ^J as a new line
				{
					Buffer[i] = '\0';
				}
				else if (i>0)
				{
					//Add tag entry
					entry.comment = string(Buffer);
					break;
				}
			}
		}
		myfile.close();
	}
	//-------------------------------------------------------------
	//Update Playlist Browser
	if (entry.artist != "No Artist" &&
		entry.title != "No Title")
	{
		entry.displayname = entry.artist;
		entry.displayname += " - ";
		entry.displayname += entry.title;

		if (entry.year != "No Year")
		{
			entry.displayname += " (";
			entry.displayname += entry.year;
			entry.displayname += ")";
		}

		//if (entry.album != "No Album")
		//{
		//	entry.displayname += " - ";
		//	entry.displayname += entry.album;
		//}

		//Update UI
		//Fl::lock();
		if (gPlaylistSearchText.length() < 3)//Not Searching
		{
			Playlist_Browser->replace(Index + 1, (const char*)entry.displayname.c_str());
		}
		//Fl::unlock();
		//Fl::awake();
	}
	//-------------------------------------------------------------
}

////ID3_V2
//char Header_Buffer[10] = { 0 };
//ifstream myfile((const char*)entry.filepath.c_str(), ios::in | ios::binary);
//myfile.seekg(0);
//myfile.read(Header_Buffer, 10);
//if (Header_Buffer[0] == 73 && ///I
//	Header_Buffer[1] == 68 && ///D
//	Header_Buffer[2] == 51) ///3
//{
//	int TagVersion = Header_Buffer[3];
//	//Tag Version
//	//if (Header_Buffer[3]>=0 && 
//	//	Header_Buffer[3]<=4)
//	// Get the ID3 tag size and flags
//	const int tagsize = (Header_Buffer[9] & 0xFF) | ((Header_Buffer[8] & 0xFF) << 7) | ((Header_Buffer[7] & 0xFF) << 14) | ((Header_Buffer[6] & 0xFF) << 21) + 10;
//	// Read the whole tag
//	char* buffer = new char[tagsize];
//	myfile.seekg(10);
//	myfile.read(buffer, tagsize);
//	entry.artist = string(buffer);
//	entry.title = "x";
//	// Set some params
//	int length = tagsize;
//	int pos = 0;
//	int ID3FrameSize = TagVersion < 3 ? 6 : 10;
//	//// Parse the tags
//	//bool bRun = true;
//	//while (bRun)
//	//{
//	//	int rembytes = length - pos;
//	//	// Do we have the frame header?
//	//	if (rembytes < ID3FrameSize)
//	//		bRun = false;
//	//		break;
//	//	// Is there a frame?
//	//	if (buffer[pos] < 'A' || buffer[pos] > 'Z')
//	//		bRun = false;
//	//	break;
//	//	// Frame name is 3 chars in pre-ID3v3 and 4 chars after
//	//	string framename = "";
//	//	int framesize = 0;
//	//	if (TagVersion < 3)
//	//	{
//	//		framename = string(buffer, pos, 3);
//	//		framesize = ((buffer[pos + 5] & 0xFF) << 8) | ((buffer[pos + 4] & 0xFF) << 16) | ((buffer[pos + 3] & 0xFF) << 24);
//	//	}
//	//	else
//	//	{
//	//		framename = string(buffer, pos, 4);
//	//		framesize = (buffer[pos + 7] & 0xFF) | ((buffer[pos + 6] & 0xFF) << 8) | ((buffer[pos + 5] & 0xFF) << 16) | ((buffer[pos + 4] & 0xFF) << 24);
//	//	}
//	//	if (pos + framesize > length)
//	//		bRun = false;
//	//		break;
//	//	if (framename == ("TPE1") || framename == ("TPE2") || framename == ("TPE3") || framename == ("TPE"))
//	//	{
//	//		entry.artist = "GOTART";// ID3V2_ParseText(buffer, pos + ID3FrameSize, framesize);
//	//	}
//	//	if (framename == ("TIT2") || framename == ("TIT"))
//	//	{
//	//		entry.title = "GOTIT";//ID3V2_ParseText(buffer, pos + ID3FrameSize, framesize);
//	//	}
//	//	pos += framesize + ID3FrameSize;
//	//}
//	delete buffer;
//}
//myfile.close();

///*****************************************************************
///Spectrum_Reset
///*****************************************************************
void bass_audio::Spectrum_Reset(void)
{
	//-------------------------------------------------------------
	for (int i = 0; i < MAX_FFT; i++)
	{
		FFT[i] = 0.0f;
	}

	for (int i = 0; i < MAX_FFT_USED; i++)
	{
		FFT_Smooth[i] = 0.0f;
		FFT_Scale[i] = 0.01f;
	}
	//-------------------------------------------------------------
	//Output Level
	Output_Level_L = 0.0f;
	Output_Level_R = 0.0f;
	Output_Level_L_Smooth = 0.0f;
	Output_Level_R_Smooth = 0.0f;
	//Tell Network of change
	cNet.bSpectrum_Reset = true;
	//-------------------------------------------------------------
}
///*****************************************************************
///Spectrum_Update
///*****************************************************************
void bass_audio::Spectrum_Update(void)
{
	//-------------------------------------------------------------
	//Clear
	for (int i = 0; i < MAX_FFT; i++)
	{
		FFT[i] = 0.0f;
	}
	//FFT Data
	if (Stream_State == BASS_ACTIVE_PLAYING)
	{
		BASS_ChannelGetData(Stream, FFT, BASS_DATA_FFT1024 | BASS_DATA_FLOAT | BASS_DATA_FFT_REMOVEDC);
	}

	//Use 32 values - 150 is 13.0khz
	int StepSize = 1;
	float fValue = 0.0f;
	int PreviousIndex = 13;
	for (int i = 0; i < MAX_FFT_USED; i++)
	{
		fValue = 0.0f;
		if (i <= 12)
		{
			fValue += FFT[i];
		}
		else
		{
			StepSize = 2 + (i - 13);
			for (int j = PreviousIndex; j < PreviousIndex + StepSize; j++)
			{
				fValue += FFT[j];
			}
			fValue /= float(StepSize);
			PreviousIndex += StepSize;
		}


		//Convert pulled FFT data to 32 floats
		FFT[i] = fValue;

		///Create a Smooth FFT value
		if (FFT[i] >= FFT_Smooth[i])
		{
			FFT_Smooth[i] = FFT[i];
		}
		else
		{
			cMath.TendTo(FFT_Smooth[i], 0.0f, fSpectrum_Falloff_TendTo);
		}
		///Obtain the Scale of Each FFT (eg: min max, normalises the spectrum)
		if (FFT_Scale[i] < FFT[i])
		{
			FFT_Scale[i] = FFT[i];
		}
	}
	//-------------------------------------------------------------
	//L & R - Output Levels
	DWORD level;
	level = BASS_ChannelGetLevel(Stream);
	Output_Level_L = 0.0f;
	Output_Level_R = 0.0f;
	if (Stream_State == BASS_ACTIVE_PLAYING)
	{
		Output_Level_L = LOWORD(level);
		Output_Level_R = HIWORD(level);
	}
	///Output Levels Update
	if (Output_Level_L >= Output_Level_L_Smooth)
	{
		Output_Level_L_Smooth = Output_Level_L;
	}
	else
	{
		cMath.TendTo(Output_Level_L_Smooth, 0.0f, fSpectrum_Falloff_TendTo);
	}

	if (Output_Level_R >= Output_Level_R_Smooth)
	{
		Output_Level_R_Smooth = Output_Level_R;
	}
	else
	{
		cMath.TendTo(Output_Level_R_Smooth, 0.0f, fSpectrum_Falloff_TendTo);
	}
	//-------------------------------------------------------------
}

