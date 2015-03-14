#include "Global.h"
#include "main.h"
#include <FL/x.H>

// our main() function, see main.h
FUZONMP3
{
	//INIT-----------------------------------------------
	//Start Rand Seed
	srand((unsigned int)time(NULL));
	//FileSystem init
	cFile.Init();
	// create Bass sound device
	cBass.Init();

	//create HRTimer
	cHRTimer.Init();
	// create windows
	MainWindow = Create_WindowPlayer();
	//MainWindow->border(0);
	Window2 = Create_Window2();
	Window3 = Create_Window3();

	//Playlist
	Info_PlaylistDirectory();
	//Init UI stuff
	Info_ShuffleButton();
	Info_Volume();
	Volume->value(cBass.Stream_Volume);
	Info_Seekbar();
	Info_Eq();
	Info_Vst();
	Init_OptionsMenu();

	// show and run player
	Fl::visual(FL_RGB);
	MainWindow->show();
	BottomPanel_Controller();

	//UPDATE-----------------------------------------------
	// initialize callback timer
	Fl::lock();
	Fl::add_timeout(updateTimerSpeed, CB_Update);
	int ret = Fl::run();

	//EXIT-----------------------------------------------
	//Stop network
	if (cNet.State > 0)
	{
		cNet.State = -1;
		cNet.Update();
	}
	//Stop audio (just incase threads have to wait for close)
	cBass.StreamStop();
	//Threads
	cThread.Exit();
	//Exit Vis Network
	cNet.StopServer();
	// exit Bass sound device
	cBass.Exit();
	//Delete windows
	delete MainWindow;
	delete Window2;
	delete Window3;

	return ret;
}

void CB_Update(void*) 
{
	//Get user desired update rate
	updateTimerSpeed = 1.0 / updateTimerSpeed_User;
	//Set / Get Delta Times
	cHRTimer.Update();
	//Repeat Update timer
	Fl::repeat_timeout(updateTimerSpeed, CB_Update);

	//Update gTimer 0.0f - 1.0f (1.0f = 1 update per second) (used for animations/flickers etc)
	Timer_Update();

	//Update Playlist Scanned Files
	if (gPlaylist_Rescan_State == -1)//Request for start scan
	{
		//stop threads
		cThread.Stop(1);
		cThread.Stop(0);
		//Clear Playlist
		Playlist_Rescan();
		//run thread
		gDebug_ScanTime = 0.0;
		gDebug_TagTime = 0.0;
		gDebug_Tagged_ID3V1 = 0;
		cThread.Start(0);
		gPlaylist_Rescan_State++;
	}
	else if (gPlaylist_Rescan_State == 0)//Waiting for scan to finish
	{
		gDebug_ScanTime += cHRTimer.HRT_DeltaTime;

		//Inform user
		Playlist_Entries_Total->value((double)gTotalFiles);
		char buffer[48] = { '\0' };
		sprintf_s(buffer, "Scanning Directories (%d)", gTotalFiles);

		MainWindow->label(buffer);
		CurrentlyPlaying->value(buffer);
		CurrentlyPlaying->color((Fl_Color)34);
		if (gTimer_Activated)
		{
			CurrentlyPlaying->color(FL_RED);
		}

		//Check for thread completion
		if (!cThread.bActive[0])
		{
			CurrentlyPlaying->value("Scan Completed");
			CurrentlyPlaying->color((Fl_Color)34);
			MainWindow->label("Scan Completed");
			if (gTotalFiles == 0)
			{
				CurrentlyPlaying->value("No Files - Goto Playlist, Add");
				MainWindow->label("Fuzon Mp3");
			}

			//update playlist entries
			PlaylistEntries_FromScan();

			gPlaylist_Rescan_State++;
		}
	}
	else if (gPlaylist_Rescan_State == 1)//Start tags
	{
		cThread.Start(1);
		gPlaylist_Rescan_State++;
	}
	else if (gPlaylist_Rescan_State == 2)//Tags Running
	{
		gDebug_TagTime += cHRTimer.HRT_DeltaTime;
		if (!cThread.bActive[1])
		{
			gPlaylist_Rescan_State++;
		}
	}

	//UI Updates
	///Update Play Button State
	Info_PlayButton();
	///Update SeekBar positions
	cBass.StreamGetPosition();
	Info_Seekbar();

	//intro
	Intro_Animation();

	///Update Spectrum
	if (cBass.bFFT_Enabled)
	{
		Info_Spectrum();
	}

	///Check for end of file
	if (cBass.EndOfFileState == 1)
	{
		Play_NextFile();
	}

	//Update Vst Window
	if (cBass.bVst_EmbedRequested)
	{
		Embed_Vst_Window();
	}

	///Update Net
	cNet.Update();
	///OE2 Vis
	Info_OE2VisButton();

	//Redraw main window
	if (MainWindow->visible() > 0)
	{
		Debug();
		BottomPanel_Animation();
		MainWindow->redraw();
	}
	//Monitor Main Window Closing
	else
	{
		Window2->hide();
		Window3->hide();
	}
}

void Timer_Update(void)
{
	gTimer += cHRTimer.HRT_DeltaTime;
	while (gTimer > gTimer_Max)
	{
		gTimer_Activated = !gTimer_Activated;
		gTimer -= gTimer_Max;
	}
}

void Play_Id(int PlaylistId)
{
	//Check Playlist IDs exists before playing
	if (gTotalFiles > 0 &&
		!cThread.bActive[0])//Playlist Search in Progress - Dont play
	{
		//Cap and switch Playlist Id at end of total files
		if (PlaylistId >= gTotalFiles)
		{
			PlaylistId = 0;
		}
		else if (PlaylistId < 0)
		{
			PlaylistId = gTotalFiles - 1;
		}
		gPlay_PreviousId = gPlay_CurrentId;
		gPlay_CurrentId = PlaylistId;

		cBass.StreamCreate((const char*)gPlaylist_Entry[gPlay_CurrentId].filepath.c_str());
		cBass.Play();
		Info_PlayButton();
		Playlist_Browser->select(gPlay_CurrentId+1);
		CurrentlyPlaying->value((const char*)gPlaylist_Entry[gPlay_CurrentId].displayname.c_str());
		MainWindow->label((const char*)gPlaylist_Entry[gPlay_CurrentId].displayname.c_str());

		//Tell network of new Track
		cNet.bNewTrack = true;
	}
	//No files
	else
	{
		///Reset
		gPlay_CurrentId = -1;
		gPlay_PreviousId = -1;
		Info_PlayButton();
	}
}

void Play_NextFile(void)
{
	///Shuffle Play
	if (cBass.bShuffle_Enabled &&
		gTotalFiles>0)
	{
		int Random = gPlay_CurrentId;
		while (Random == gPlay_CurrentId)
		{
			Random = rand() % gTotalFiles;
		}
		Play_Id(Random);
		///Put browser location to current
		Playlist_Browser->bottomline(gPlay_CurrentId + 1);
	}
	else
	{
		///Normal Play
		Play_Id(gPlay_CurrentId + 1);
	}
}

void CB_Play(Fl_Light_Button*, void*) 
{
	if (gPlay_CurrentId == -1)//init, play something
	{
		Play_Id(0);
	}
	else if(gTotalFiles>0)//Toggle pause
	{
		cBass.Play();
		Info_PlayButton();
	}
}

void CB_Shuffle(class Fl_Light_Button *, void *)
{
	cBass.bShuffle_Enabled = !cBass.bShuffle_Enabled;
	Info_ShuffleButton();
}

void CB_Button_OE2_Vis(class Fl_Light_Button *, void *)
{
	//Start server
	if (cNet.State == 0)
	{
		//Check if Program exists
		HANDLE hFile = CreateFile((const char*)cFile.Vis_Location_Exe.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		bool bProgramExists = false;
		if (hFile != INVALID_HANDLE_VALUE)
		{
			bProgramExists = true;
		}
		CloseHandle(hFile);

		if (bProgramExists)
		{
			cNet.State = 1;
			ShellExecuteA(GetDesktopWindow(), "open", (const char*)cFile.Vis_Location_Exe.c_str(), NULL, (const char*)cFile.Vis_Location_Path.c_str(), SW_SHOWNORMAL);
		}
		else
		{
			//File Doesnt Exist
		}
	}
	//Stop server
	else if (cNet.State==2)
	{
		cNet.State = -1;
	}
	Info_OE2VisButton();
}

void CB_Play_Next(Fl_Button*, void*)
{
	Play_NextFile();
}

void CB_Play_Previous(Fl_Button*, void*)
{
	//Shuffle - Play previous file
	if (cBass.bShuffle_Enabled)
	{
		Play_Id(gPlay_PreviousId);
	}
	//Play previous File
	else
	{
		Play_Id(gPlay_CurrentId-1);
	}
}

void CB_Volume(class Fl_Dial*, void*)
{
	//if (Fl::event_inside(Volume) && abs(Fl::event_dy()) > 0)
	//{
	//	Volume->value((int)Volume->value(0.0));
	//}
	cBass.SetVolume((int)Volume->value());
	Info_Volume();
}

void CB_PlayList(Fl_Browser*, void*)
{
	int selected = Playlist_Browser->value();
	int Total = Playlist_Browser->size();

	if (selected > 0 &&
		selected <= Total &&
		Total>0)//0 = empty
	{
		///Check if this is a search play request
		if (int(gPlaylistSearchText.length()) >= gPlaylistSearchCharLength)
		{
			///match the name to obtain a play ID
			const char* TextSelected = Playlist_Browser->text(selected);
			string TempString = string(TextSelected);
			for (long i = 0; i < gTotalFiles; i++)
			{
				if (TempString == gPlaylist_Entry[i].displayname)
				{
					Play_Id(i);
					break;
				}
			}
		}
		///Normal Select Play
		else
		{
			Play_Id(selected - 1);
		}
	}
}

void CB_Playlist_Search(Fl_Input*, void*)
{
	const char* Input = Playlist_Search->value();
	gPlaylistSearchText = string(Input);
	if (int(gPlaylistSearchText.length()) < gPlaylistSearchCharLength)
	{
		PlaylistEntries_FromScan();
	}
	else
	{
		PlaylistEntries_FromSearch();
	}
}

void CB_Button_PlaylistManager(Fl_Button*, void*)
{
	///toggle
	gWindow_PlaylistManager = false;
	if (Window2->visible() == 0)
	{
		gWindow_PlaylistManager = true;
	}
	//gWindow_PlaylistManager = !gWindow_PlaylistManager;
	///hide everything
	Window2->hide();
	Filebrowser->hide();
	Filebrowser_Label->hide();
	Filebrowser_Drive->hide();
	///Show new window
	if (gWindow_PlaylistManager)
	{
		///get main window location
		int mw_x = MainWindow->x_root();
		int mw_y = MainWindow->y_root();
		///set window location
		Window2->position(mw_x + 370, mw_y);
		Window2->size(370, 370);

		Window2->show();
		Filebrowser->show();
		Filebrowser_Label->show();
		Filebrowser_Drive->show();
	}
}

void CB_Filebrowser(Fl_File_Browser*, void*) 
{
	int selected = Filebrowser->value();
	int Total = Filebrowser->size();

	if (selected > 0 &&
		selected <= Total)
	{
		const char* NewLocation = Filebrowser->text(selected);
		char name[MAX_FILEPATH] = { '\0' };
		fl_filename_absolute(name, NewLocation);
		//Folder
		if (fl_filename_isdir(name) != 0)
		{
			cFile.changeDir(name);
			Filebrowser->load(name);
			Filebrowser->redraw();
			Filebrowser_Label->value(name);
			Filebrowser_Label->redraw();
			//Copy to global path varible
			memset(cFile.FilePath, '\0', MAX_FILEPATH);///Clear char
			strcpy_s(cFile.FilePath, name);
		}
	}
}

void CB_Seekbar(Fl_Slider*, void*)
{
	cBass.StreamSetPosition(Seekbar->value());
	//if (Fl::event() == FL_RELEASE)
	//Redraw main window
	//MainWindow->redraw();
}

void CB_Button_Playlist(Fl_Light_Button*, void*)
{
	if (BottomPanelID==0)
	{
		BottomPanelID = -1;
	}
	else
	{
		BottomPanelID = 0;
	}
	BottomPanel_Controller();
}

void CB_Button_Options(Fl_Light_Button*, void*)
{
	if (BottomPanelID == 3)
	{
		BottomPanelID = -1;
	}
	else
	{
		BottomPanelID = 3;
	}
	BottomPanel_Controller();
}

void CB_Filebrowser_Drive(Fl_Button*, void*)
{
	memset(cFile.FilePath, '\0', MAX_FILEPATH);///Clear char
	Filebrowser_Label->value("Drives");
	Filebrowser_Label->redraw();
	Filebrowser->load(cFile.FilePath);
	Filebrowser->redraw();
}

void CB_Button_Calc(Fl_Light_Button*, void*)
{
	if (BottomPanelID == 1)
	{
		BottomPanelID = -1;
	}
	else
	{
		BottomPanelID = 1;
	}
	BottomPanel_Controller();
}

void CB_Button_Eq(Fl_Light_Button*, void*)
{
	if (BottomPanelID == 2)
	{
		BottomPanelID = -1;
	}
	else
	{
		BottomPanelID = 2;
	}
	BottomPanel_Controller();
}

void CB_Calc_Bpm(Fl_Value_Input*, void*)
{
	if (Calc_Bpm->value() > 0.0)
	{
		Calc_Bpm_Value = (60000 / Calc_Bpm->value()) * 4.0;
	}
	else
	{
		Calc_Bpm_Value = 0.0;
	}

	//
	char buffer[128] = { '\0' };
	double Output = 0.0;
	double OutputD = 0.0;
	double OutputT = 0.0;
	//STD
	Output = Calc_Bpm_Value;
	OutputD = Output * 1.5;
	OutputT = Output * 0.66667;
	sprintf_s(buffer, "%0.2f ms | Dot: %0.2f ms | Trip: %0.2f ms", Output,OutputD,OutputT);
	Calc_0->value(buffer);

	Output = Calc_Bpm_Value / 2.0;
	OutputD = Output * 1.5;
	OutputT = Output * 0.66667;
	sprintf_s(buffer, "%0.2f ms | Dot: %0.2f ms | Trip: %0.2f ms", Output, OutputD, OutputT);
	Calc_1->value(buffer);

	Output = Calc_Bpm_Value / 4.0;
	OutputD = Output * 1.5;
	OutputT = Output * 0.66667;
	sprintf_s(buffer, "%0.2f ms | Dot: %0.2f ms | Trip: %0.2f ms", Output, OutputD, OutputT);
	Calc_2->value(buffer);

	Output = Calc_Bpm_Value / 8.0;
	OutputD = Output * 1.5;
	OutputT = Output * 0.66667;
	sprintf_s(buffer, "%0.2f ms | Dot: %0.2f ms | Trip: %0.2f ms", Output, OutputD, OutputT);
	Calc_3->value(buffer);

	Output = Calc_Bpm_Value / 16.0;
	OutputD = Output * 1.5;
	OutputT = Output * 0.66667;
	sprintf_s(buffer, "%0.2f ms | Dot: %0.2f ms | Trip: %0.2f ms", Output, OutputD, OutputT);
	Calc_4->value(buffer);

	Output = Calc_Bpm_Value / 32.0;
	OutputD = Output * 1.5;
	OutputT = Output * 0.66667;
	sprintf_s(buffer, "%0.2f ms | Dot: %0.2f ms | Trip: %0.2f ms", Output, OutputD, OutputT);
	Calc_5->value(buffer);

	Output = Calc_Bpm_Value / 64.0;
	OutputD = Output * 1.5;
	OutputT = Output * 0.66667;
	sprintf_s(buffer, "%0.2f ms | Dot: %0.2f ms | Trip: %0.2f ms", Output, OutputD, OutputT);
	Calc_6->value(buffer);
}

void CB_Eq(Fl_Slider*, void*)
{
	///Bell (end, *giggle*)
	cBass.Eq8Param[0].fGain = (float)Eq_0->value();
	cBass.Eq8Param[1].fGain = (float)Eq_1->value();
	cBass.Eq8Param[2].fGain = (float)Eq_2->value();
	cBass.Eq8Param[3].fGain = (float)Eq_3->value();
	cBass.Eq8Param[4].fGain = (float)Eq_4->value();
	cBass.Eq8Param[5].fGain = (float)Eq_5->value();
	cBass.Eq8Param[6].fGain = (float)Eq_6->value();
	cBass.Eq8Param[7].fGain = (float)Eq_7->value();
	///High Pass
	cBass.Eq8Param[8].fCenter = (float)Eq_8->value();

	//
	//char buffer[10] = { 0 };

	//sprintf_s(buffer, "%0.1f db", cBass.Eq8Param[0].fGain);
	//Eq_0->tooltip(buffer);

	//Fl_Tooltip::enable;
	//Fl_Tooltip::hoverdelay(0.01f);
	

	cBass.DSP_Update();
}

void CB_Eq_On(Fl_Light_Button*, void*)
{
	//Toggle On Off
	cBass.bUser_Eq8_Enabled = !cBass.bUser_Eq8_Enabled;
	cBass.DSP_Destroy();
	cBass.DSP_Create();
	cBass.Spectrum_Reset();
	Info_Eq();
}

void CB_Eq_Reset(Fl_Button*, void*)
{
	cBass.Eq_Reset();
	cBass.DSP_Update();
	cBass.Spectrum_Reset();
	Info_Eq();
}

void CB_Vst_On(Fl_Light_Button*, void*)
{
	cBass.bUser_Vst_Enabled = !cBass.bUser_Vst_Enabled;
	cBass.DSP_Destroy();
	cBass.DSP_Create();
	cBass.Spectrum_Reset();
	Info_Vst();
}

void CB_Vst_Reset(Fl_Button*, void*)
{
	cBass.Vst_Reset();
	cBass.Spectrum_Reset();
}

void CB_Vst_Show(Fl_Button*, void*)
{
	if (Window3->visible() > 0)
	{
		Window3->hide();
	}
	//Visual
	else if (cBass.bUser_Vst_Enabled)
	{
		Window3->show();
		Window3->set_output();
		Info_Vst();
		cBass.bVst_EmbedRequested = true;
	}
}

void CB_Vst_Choice(Fl_Choice*, void*)
{
	memset(cBass.Vst_Selected_Name, '\0', sizeof(cBass.Vst_Selected_Name));

	////memcpy(cBass.Vst_Selected_Name, Vst_Choice->text(), sizeof(Vst_Choice->text()));
	////sizeof(Vst_Choice->text()) does nothing, so lets string it.
	string Temp = string(Vst_Choice->text());
	memcpy(cBass.Vst_Selected_Name, (char*)Temp.c_str(), Temp.size());

	cBass.DSP_Destroy();
	cBass.DSP_Create();
	Info_Vst();
}

void CB_Button_Playlist_Directory_Add(Fl_Button*, void*)
{
	bool bAlreadyExists = false;
	//Add cFile.FilePath
	if (cFile.Playlist_Directory_Count < MAX_PLAYLIST_DIRECTORY && 
		string(cFile.FilePath) != "")
	{
		cFile.Playlist_Directory[cFile.Playlist_Directory_Count] = string(cFile.FilePath);
		///Check if already exists
		for (int i = 0; i < cFile.Playlist_Directory_Count; i++)
		{
			if (i != cFile.Playlist_Directory_Count &&
				cFile.Playlist_Directory[cFile.Playlist_Directory_Count] == cFile.Playlist_Directory[i])
			{
				bAlreadyExists = true;
				break;
			}
		}

		///Add new path
		if (!bAlreadyExists)
		{
			Clear_Search();
			cFile.Playlist_Directory_Count++;

			//Update User directory UI
			Info_PlaylistDirectory();
			//Update playlist
			gPlaylist_Rescan_State = -1;
		}
	}
}

void CB_Button_Playlist_Directory_Remove(Fl_Button*, void*)
{
	if (cFile.Playlist_Directory_Count > 0 &&
		Playlist_Directories->value()>0)///ensure we have a selection
	{
		Clear_Search();
		//Stop current stream
		cBass.StreamStop();

		//Remove
		cFile.Playlist_Directory[Playlist_Directories->value()-1] = "";
		cFile.Playlist_Directory_Count--;
		//cFile.Playlist_Directory[cFile.Playlist_Directory_Count] = "";

		//re-organise the list
		for (int i = 0; i < MAX_PLAYLIST_DIRECTORY; i++)
		{
			if (cFile.Playlist_Directory[i] == "")
			{
				for (int j = i+1; j < MAX_PLAYLIST_DIRECTORY; j++)
				{
					if (cFile.Playlist_Directory[j] != "")
					{
						cFile.Playlist_Directory[i] = cFile.Playlist_Directory[j];
						cFile.Playlist_Directory[j] = "";
						break;
					}
				}
			}
		}
				
		//cFile.Playlist_Directory[cFile.Playlist_Directory_Count] = "";
		//Update User directory UI
		Info_PlaylistDirectory();
		
		//Update playlist
		gPlaylist_Rescan_State = -1;
	}
}

void CB_Options_Browser_Menu(Fl_Browser*, void*)
{
	gOptions_Choice_SubMenu = Options_Browser_Menu->value() - 1;
	BottomPanel_Controller();
}

void CB_Options_Slider_Updaterate(Fl_Value_Slider*, void*)
{
	updateTimerSpeed_User = Options_Slider_Updaterate->value();
}

void CB_Options_Slider_SpectrumFalloffSpeed(Fl_Value_Slider*, void*)
{
	cBass.fSpectrum_Falloff_TendTo = float(Options_Slider_SpectrumFalloffSpeed->value());

	//Tell Network of changes
	cNet.bInterfaceUpdate = true;
}

void CB_Options_Slider_SpectrumColour(Fl_Value_Slider*, void*)
{
	gSpectrum_R = unsigned char(Options_Slider_SpectrumColourR->value());
	gSpectrum_G = unsigned char(Options_Slider_SpectrumColourG->value());
	gSpectrum_B = unsigned char(Options_Slider_SpectrumColourB->value());

	//Tell Network of changes
	cNet.bInterfaceUpdate = true;
}

void CB_Options_Button_SpectrumEnable(Fl_Light_Button*, void*)
{
	cBass.bFFT_Enabled = false;
	cBass.Spectrum_Reset();
	Info_Spectrum();
	if (Options_Button_SpectrumEnable->value() == 1)
	{
		cBass.bFFT_Enabled = true;
	}
}

void CB_Options_HardwareChange(Fl_Choice*, void*)
{
	cBass.Hardware_Device_Selected = Options_Choice_OutputDevice->value() + 1;
}

void Init_OptionsMenu(void)
{
	Options_Browser_Menu->clear();
	Options_Browser_Menu->add("General");
	Options_Browser_Menu->add("Sound");
	Options_Browser_Menu->add("Spectrum");
	Options_Browser_Menu->add("Debug");

	Options_Browser_Menu->select(gOptions_Choice_SubMenu+1);

	//General
	Options_Slider_Updaterate->value(updateTimerSpeed_User);

	//Sound
	Options_Choice_OutputDevice->clear();
	for (int i = 0; i < cBass.Hardware_Devices_Total; i++)
	{
		Options_Choice_OutputDevice->add(cBass.Hardware_Device_Info[i].name);
	}
	Options_Choice_OutputDevice->value(cBass.Hardware_Device_Selected - 1);

	Options_Choice_OutputFreq->clear();
	//for (char i = 0; i < MAX_HARDWARE_FREQUENCIES; i++)
	//{
	//	char Temp[64] = { 0 };
	//	if (cBass.bHardware_Frequency_Available[i])
	//	{
	//		sprintf_s(Temp, "%d hz", cBass.Hardware_Frequency_Choice[i]);
	//	}
	//	else
	//	{
	//		sprintf_s(Temp, "%d - Not Supported", cBass.Hardware_Frequency_Choice[i]);
	//	}
	//	Options_Choice_OutputFreq->add(Temp);
	//}
	//Options_Choice_OutputFreq->value(cBass.Hardware_Frequency_Selected);

	char Temp[64] = { '\0' };
	sprintf_s(Temp, "%d hz", cBass.Hardware_Info.freq);
	Options_Choice_OutputFreq->add(Temp);
	Options_Choice_OutputFreq->value(0);


	//Spectrum
	Options_Slider_SpectrumFalloffSpeed->value(double(cBass.fSpectrum_Falloff_TendTo));
	Options_Slider_SpectrumColourR->value(double(gSpectrum_R));
	Options_Slider_SpectrumColourG->value(double(gSpectrum_G));
	Options_Slider_SpectrumColourB->value(double(gSpectrum_B));
	Options_Button_SpectrumEnable->value(0);
	if (cBass.bFFT_Enabled)
	{
		Options_Button_SpectrumEnable->value(1);
	}
}

void Info_Seekbar(void) 
{
	Seekbar->maximum(cBass.FileLength);
	Seekbar->value(cBass.Current_Position);
	Seekbar->label("                          No File");
	if (cBass.Stream_State >= 1)//Playing
	{
		//Playback Total Time
		double Seconds = cBass.FileLength;
		double Minutes = 0.0;
		while (Seconds >= 60.0)
		{
			Minutes += 1.0;
			Seconds -= 60.0;
		}
		//Playback Current Time
		double Seconds2 = cBass.Current_Position;
		double Minutes2 = 0.0;
		while (Seconds2 >= 60.0)
		{
			Minutes2 += 1.0;
			Seconds2 -= 60.0;
		}
		sprintf_s(Text_Seekbar, "                          %0.0fm : %02.0fs // %0.0fm : %02.0fs", Minutes2, Seconds2, Minutes, Seconds);
		if (cBass.Stream_State == 3)//paused
		{
			sprintf_s(Text_Seekbar, "                          %0.0fm : %02.0fs // Paused", Minutes2, Seconds2);
		}
		Seekbar->label(Text_Seekbar);
	}
}

void Info_PlayButton(void)
{
	Play->value(0);
	if (cBass.Stream_State == 1)//Playing
	{
		Play->selection_color((Fl_Color)87);
		Play->value(1);
	}
	else if (cBass.Stream_State == 3)//Paused
	{
		if (gTimer_Activated)
		{
			Play->selection_color((Fl_Color)184);
			Play->value(1);
		}
	}
	Play->clear_changed();
}

void Info_ShuffleButton(void)
{
	Shuffle->value(0);
	if (cBass.bShuffle_Enabled)
	{
		Shuffle->value(1);
	}
}

void Info_Spectrum(void)
{
	//FFT Spectrum
	int FFT_ID = 0;
	///Set Min Max
	Slider_FFT_0->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	Slider_FFT_1->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	Slider_FFT_2->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	Slider_FFT_3->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	Slider_FFT_4->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	Slider_FFT_5->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	Slider_FFT_6->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	Slider_FFT_7->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	Slider_FFT_8->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	Slider_FFT_9->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	Slider_FFT_10->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	Slider_FFT_11->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	Slider_FFT_12->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	Slider_FFT_13->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	Slider_FFT_14->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	Slider_FFT_15->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	Slider_FFT_16->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	Slider_FFT_17->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	Slider_FFT_18->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	Slider_FFT_19->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	Slider_FFT_20->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	Slider_FFT_21->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	Slider_FFT_22->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	Slider_FFT_23->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	Slider_FFT_24->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	Slider_FFT_25->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	Slider_FFT_26->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	Slider_FFT_27->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	Slider_FFT_28->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	Slider_FFT_29->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	Slider_FFT_30->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	Slider_FFT_31->minimum(cBass.FFT_Scale[FFT_ID]); FFT_ID++;
	///Apply Values
	FFT_ID = 0;
	Slider_FFT_0->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	Slider_FFT_1->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	Slider_FFT_2->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	Slider_FFT_3->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	Slider_FFT_4->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	Slider_FFT_5->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	Slider_FFT_6->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	Slider_FFT_7->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	Slider_FFT_8->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	Slider_FFT_9->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	Slider_FFT_10->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	Slider_FFT_11->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	Slider_FFT_12->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	Slider_FFT_13->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	Slider_FFT_14->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	Slider_FFT_15->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	Slider_FFT_16->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	Slider_FFT_17->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	Slider_FFT_18->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	Slider_FFT_19->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	Slider_FFT_20->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	Slider_FFT_21->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	Slider_FFT_22->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	Slider_FFT_23->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	Slider_FFT_24->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	Slider_FFT_25->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	Slider_FFT_26->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	Slider_FFT_27->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	Slider_FFT_28->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	Slider_FFT_29->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	Slider_FFT_30->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	Slider_FFT_31->value(cBass.FFT_Smooth[FFT_ID]); FFT_ID++;
	//Output Peaks
	Slider_OutputLevel_L->value(cBass.Output_Level_L_Smooth);
	Slider_OutputLevel_R->value(cBass.Output_Level_R_Smooth);

	//Colours
	Slider_FFT_0->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_FFT_1->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_FFT_2->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_FFT_3->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_FFT_4->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_FFT_5->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_FFT_6->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_FFT_7->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_FFT_8->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_FFT_9->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_FFT_10->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_FFT_11->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_FFT_12->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_FFT_13->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_FFT_14->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_FFT_15->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_FFT_16->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_FFT_17->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_FFT_18->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_FFT_19->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_FFT_20->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_FFT_21->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_FFT_22->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_FFT_23->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_FFT_24->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_FFT_25->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_FFT_26->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_FFT_27->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_FFT_28->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_FFT_29->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_FFT_30->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_FFT_31->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));

	Slider_OutputLevel_L->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
	Slider_OutputLevel_R->selection_color(fl_rgb_color(gSpectrum_R, gSpectrum_G, gSpectrum_B));
}

void Info_OE2VisButton(void)
{
	//Connected - Vis Online
	Button_OE2_Vis->value(0);
	if (cNet.State == 2)
	{
		Button_OE2_Vis->value(1);
		Button_OE2_Vis->selection_color((Fl_Color)87);
	}
	//Started - Waiting for connection
	else if (cNet.State==1 &&
		gTimer_Activated)
	{
		Button_OE2_Vis->selection_color((Fl_Color)184);
		Button_OE2_Vis->value(1);
	}
	//Prevent Callback
	Button_OE2_Vis->clear_changed();
}

void Info_Volume(void)
{
	if (cBass.Stream_Volume == 0)
	{
		Volume->label("Mute");
	}
	else if (cBass.Stream_Volume == 100)
	{
		Volume->label("Max");
	}
	else
	{
		int scale = (cBass.Stream_Volume / 10) + 1;
		for (int i = 0; i < 12; i++)
		{
			Text_Volume[i] = 0;
			if (i < scale)
			{
				Text_Volume[i] = 124;
			}
		}
		Volume->label(Text_Volume);
	}
}

void Info_Eq(void)
{
	//Update light button
	Eq_On->value(0);
	if (cBass.bUser_Eq8_Enabled)
	{
		Eq_On->value(1);
	}
	//Obtain EQ values - Update slider
	///Bell
	Eq_0->value((float)cBass.Eq8Param[0].fGain);
	Eq_1->value((float)cBass.Eq8Param[1].fGain);
	Eq_2->value((float)cBass.Eq8Param[2].fGain);
	Eq_3->value((float)cBass.Eq8Param[3].fGain);
	Eq_4->value((float)cBass.Eq8Param[4].fGain);
	Eq_5->value((float)cBass.Eq8Param[5].fGain);
	Eq_6->value((float)cBass.Eq8Param[6].fGain);
	Eq_7->value((float)cBass.Eq8Param[7].fGain);
	///High Pass
	Eq_8->value((float)cBass.Eq8Param[8].fCenter);
}

void Info_Vst(void)
{
	//Add VST file to list
	Vst_Choice->clear();
	for (int i = 0; i < cFile.Vst_FileCount; i++)
	{
		Vst_Choice->add((const char*)cFile.Vst_FileName[i].c_str());
	}
	//Reselect previous 
	if (cBass.Vst_Selected_Name[0] >= 32)
	{
		//find it
		for (int i = 0; i < cFile.Vst_FileCount; i++)
		{
			if (string(cBass.Vst_Selected_Name) == cFile.Vst_FileName[i])
			{
				Vst_Choice->value(i);
				break;
			}
		}
	}

	//Update light button
	Vst_On->value(0);
	if (cBass.bUser_Vst_Enabled)
	{
		Vst_On->value(1);
		//Resize Window
		int x = 500;
		int y = 400;
		if (int(cBass.Vst_Info->editorWidth) > 0)
		{
			x = int(cBass.Vst_Info->editorWidth);
			y = int(cBass.Vst_Info->editorHeight);
		}
		Window3->size(x, y);
		//Set Title
		Window3->label(cBass.Vst_Info->effectName);
	}
}

void Info_Search(void)
{
	//Search Indicator
	Playlist_Entries_Total->color((Fl_Color)34);
	Playlist_Entries_Total->textcolor((Fl_Color)79);

	Playlist_Search->color((Fl_Color)34);
	Playlist_Search->textcolor((Fl_Color)79);
	//Search Active
	if (int(gPlaylistSearchText.length()) >= gPlaylistSearchCharLength)
	{
		//Not found
		Playlist_Entries_Total->color(fl_rgb_color(255, 0, 0));
		Playlist_Entries_Total->textcolor(fl_rgb_color(255, 255, 255));

		Playlist_Search->color(fl_rgb_color(255, 0, 0));
		Playlist_Search->textcolor(fl_rgb_color(255, 255, 255));
		//Found
		if (gSearchEntries > 0)
		{
			Playlist_Entries_Total->color((Fl_Color)79);
			Playlist_Entries_Total->textcolor(fl_rgb_color(0, 0, 0));

			Playlist_Search->color((Fl_Color)79);
			Playlist_Search->textcolor(fl_rgb_color(0, 0, 0));
		}
	}
}

void Embed_Vst_Window(void)
{
	//Clear VST window
	BASS_VST_EmbedEditor(cBass.vstDSP, NULL);
	//Setup window if showing
	if (cBass.bUser_Vst_Enabled && 
		Window3->visible() > 0)
	{
		vstWindow = fl_xid(Window3);
		BASS_VST_EmbedEditor(cBass.vstDSP, vstWindow);
		//Redraw window
		Window3->redraw();
	}
	//Hide Vst window if inactive or invalid VST
	else
	{
		Window3->hide();
	}

	cBass.bVst_EmbedRequested = false;
}

void Info_PlaylistDirectory(void)
{
	///remove all
	Playlist_Directories->clear();
	/// add all
	for (int i = 0; i < cFile.Playlist_Directory_Count; i++)
	{
		Playlist_Directories->add((const char*)cFile.Playlist_Directory[i].c_str());
	}
	//Auto Select
	if (Playlist_Directories->size()>0)
	{
		Playlist_Directories->value(Playlist_Directories->size());
	}
}

void Intro_Animation(void)
{
	if (gInto_Animation_Timer < gInto_Animation_Timer_MAX)
	{
		double iD = cMath.Interpolate(gInto_Animation_Timer, 0.0, gInto_Animation_Timer_MAX, 0.0, double(MAX_FFT_USED) - 1.0);

		cBass.FFT_Smooth[int(iD)] = 0.1f;
		cBass.Spectrum_Update();
		gInto_Animation_Timer += cHRTimer.HRT_DeltaTime;
	}
	else
	{
		gInto_Animation_Completed = true;
	}
}

void BottomPanel_Animation(void)
{
	//Resize window
	cMath.TendTo(gBottomPanel_Current_Y, gBottomPanel_Target_Y, gBottomPanel_TendTo);
	MainWindow->size(int(gBottomPanel_Target_X), int(gBottomPanel_Current_Y));
	//MainWindow->redraw();
}

void BottomPanel_Controller(void)
{
	//---------------------------------------------------------------
	gBottomPanel_Target_Y = 149.0f;
	if (BottomPanelID>=0)
	{
		gBottomPanel_Target_Y = 404.0f;
	}
	//---------------------------------------------------------------
	//Reset everything
	///Lights
	Button_Playlist->value(false);
	Button_Calc->value(false);
	Button_Eq->value(false);
	Button_Options->value(false);

	//Playlist
	Playlist_Browser->hide();
	Playlist_Entries_Total->hide();
	Playlist_Search->hide();
	Button_PlaylistManager->hide();

	//calc
	Calc_Bpm->hide();
	Calc_0->hide();
	Calc_1->hide();
	Calc_2->hide();
	Calc_3->hide();
	Calc_4->hide();
	Calc_5->hide();
	Calc_6->hide();

	//dsp
	Eq_0->hide();
	Eq_1->hide();
	Eq_2->hide();
	Eq_3->hide();
	Eq_4->hide();
	Eq_5->hide();
	Eq_6->hide();
	Eq_7->hide();
	Eq_8->hide();
	Eq_On->hide();
	Eq_Reset->hide();
	Vst_On->hide();
	Vst_Reset->hide();
	Vst_Show->hide();
	Vst_Choice->hide();

	//Options
	Options_Browser_Menu->hide();
	Options_Seperator->hide();

	Options_Slider_Updaterate->hide();

	Options_Choice_OutputDevice->hide();
	Options_Choice_OutputFreq->hide();

	Options_Button_SpectrumEnable->hide();
	Options_Slider_SpectrumFalloffSpeed->hide();
	Options_Slider_SpectrumColourR->hide();
	Options_Slider_SpectrumColourG->hide();
	Options_Slider_SpectrumColourB->hide();

	Options_Debug->hide();
	//---------------------------------------------------------------
	//Show
	if (BottomPanelID == 0)///Playlist
	{
		Button_Playlist->value(true);
		Playlist_Browser->show();
		Playlist_Entries_Total->show();
		Playlist_Search->show();
		Button_PlaylistManager->show();
	}
	else if (BottomPanelID == 1)///Calc
	{
		Button_Calc->value(true);
		Calc_Bpm->show();
		Calc_0->show();
		Calc_1->show();
		Calc_2->show();
		Calc_3->show();
		Calc_4->show();
		Calc_5->show();
		Calc_6->show();
	}
	else if (BottomPanelID == 2)///DSP
	{
		Button_Eq->value(true);
		Eq_0->show();
		Eq_1->show();
		Eq_2->show();
		Eq_3->show();
		Eq_4->show();
		Eq_5->show();
		Eq_6->show();
		Eq_7->show();
		Eq_8->show();
		Eq_On->show();
		Eq_Reset->show();
		Vst_On->show();
		Vst_Reset->show();
		Vst_Show->show();
		Vst_Choice->show();
	}
	else if (BottomPanelID == 3)///Options
	{
		Button_Options->value(true);
		Options_Seperator->show();
		Options_Browser_Menu->show();

		if (gOptions_Choice_SubMenu == 0)//General
		{
			Options_Slider_Updaterate->show();
		}
		else if (gOptions_Choice_SubMenu == 1)//Sound
		{
			Options_Choice_OutputDevice->show();
			Options_Choice_OutputFreq->show();
		}
		else if (gOptions_Choice_SubMenu == 2)//Spectrum
		{
			Options_Button_SpectrumEnable->show();
			Options_Slider_SpectrumFalloffSpeed->show();
			Options_Slider_SpectrumColourR->show();
			Options_Slider_SpectrumColourG->show();
			Options_Slider_SpectrumColourB->show();
		}
		else if (gOptions_Choice_SubMenu == 3)//Debug
		{
			Options_Debug->show();
		}
	}
}

void Playlist_Rescan(void)
{
	//Playlist UI clear
	gTotalFiles = 0;
	Playlist_Browser->clear();
	Playlist_Entries_Total->value((double)gTotalFiles);
	//Clear array and setup
	if (gPlaylist_ArraySize > 0)
	{
		delete[] gPlaylist_Entry;
	}
	gPlaylist_ArraySize = gPlaylist_ArrayIncreaseStepSize;
	gPlaylist_Entry = new PL_ENTRY[gPlaylist_ArraySize];
	//for (long i = 0; i < gPlaylist_ArraySize; i++)
	//{
	//	gPlaylist_Entry[i] = { };
	//}
}

void PlaylistEntries_FromScan(void)
{
	Playlist_Browser->clear();
	//Update Playlist UI
	Playlist_Entries_Total->value((double)gTotalFiles);
	Playlist_Entries_Total->label("Total");

	//static int widths[] = { 150, 150, 150, 150,  0 };  // widths for each column
	//Playlist_Browser->column_widths(widths); // assign array to widget
	//Playlist_Browser->column_char('\t');     // use tab as the column character

	for (long i = 0; i < gTotalFiles; i++)
	{
		///Apply to playlist browser
		Playlist_Browser->add((const char*)gPlaylist_Entry[i].displayname.c_str());
	}
	//Search Indicator
	Info_Search();
}

void PlaylistEntries_FromSearch(void)
{
	Playlist_Browser->clear();
	//Update UI
	Playlist_Entries_Total->label("Found");
	///Convert to lower case
	transform(gPlaylistSearchText.begin(), gPlaylistSearchText.end(), gPlaylistSearchText.begin(), ::tolower);

	gSearchEntries = 0;
	for (long i = 0; i < gTotalFiles; i++)
	{
		///Conver to lower case
		string TempString = gPlaylist_Entry[i].displayname;
		transform(TempString.begin(), TempString.end(), TempString.begin(), ::tolower);

		size_t FindPosition = 0;
		FindPosition = TempString.find(gPlaylistSearchText);
		if (FindPosition != string::npos)
		{
			gSearchEntries++;
			Playlist_Browser->add((const char*)gPlaylist_Entry[i].displayname.c_str());
		}
	}
	Playlist_Entries_Total->value((double)gSearchEntries);

	//Search Indicator
	Info_Search();
}

void Clear_Search(void)
{
	Playlist_Search->value("");
	gPlaylistSearchText = "";
	//Search Indicator
	Info_Search();
}

void Debug(void)
{
	if (BottomPanelID == 3 &&
		gOptions_Choice_SubMenu == 3)
	{
		Options_Debug->clear();
		static int widths[] = { 150, 100, 0 };  // widths for each column
		Options_Debug->column_widths(widths); // assign array to widget
		Options_Debug->column_char('\t');     // use tab as the column character

		char Buffer[128] = { '\0' };
		Options_Debug->add("@C22 >> High Resolution Timer");
		sprintf_s(Buffer, "Delta - Target\t%0.4f", updateTimerSpeed);
		Options_Debug->add(Buffer);

		sprintf_s(Buffer, "Delta - Current\t%0.4f", cHRTimer.HRT_DeltaTime);
		Options_Debug->add(Buffer);

		sprintf_s(Buffer, "Delta - Variance\t%0.4f", cHRTimer.HRT_Offset);
		Options_Debug->add(Buffer);

		Options_Debug->add("@C22 >> File Scan");
		sprintf_s(Buffer, "FileScan (Seconds)\t%0.2f", gDebug_ScanTime);
		Options_Debug->add(Buffer);

		sprintf_s(Buffer, "Tag Scan (Seconds)\t%0.2f", gDebug_TagTime);
		Options_Debug->add(Buffer);

		sprintf_s(Buffer, "Tags Read (ID3_V1)\t%d / %d", gDebug_Tagged_ID3V1, gTotalFiles);
		Options_Debug->add(Buffer);
	}
}
