#pragma once
//-----------------------------------------------------------------
#include "bass.h"
#include "bass_fx.h"
#include "bass_vst.h"
//#include "bassmix.h"

#include "bassflac.h"
#include "bassopus.h"
#include "basswma.h"
//-----------------------------------------------------------------
#define MAX_EQ_BANDS 9
#define MAX_FFT 512 // Half of BASS_DATA_FFT512 BASS_DATA_FFT1024 etc
#define MAX_FFT_USED 32
#define MAX_HARDWARE_DEVICES 5
#define MAX_HARDWARE_FREQUENCIES 5

//#define MAX_VST_EFFECTS 20
//-----------------------------------------------------------------
class bass_audio
{
	//-------------------------------------------------------------
public:
	//---------------------------------------------------------
	//HardWare Devices
	BASS_DEVICEINFO			Hardware_Device_Info[MAX_HARDWARE_DEVICES];
	int						Hardware_Devices_Total;
	int						Hardware_Device_Selected;
	BASS_INFO				Hardware_Info;

	//Core
	BOOL					bBassInitilized;
    double                  FileLength;
    double                  Current_Position;
	DWORD					Stream_State;
	int						EndOfFileState;
	//User options
	int						Stream_Volume;
	bool					bShuffle_Enabled;
	float					fSpectrum_Falloff_TendTo;

	//VST
	DWORD					vstDSP;
	int						vstDSP_ParamCount;
	float*					fVst_UserParam;
	bool					bUser_Vst_Enabled;
	char					Vst_Selected_Name[MAX_FILEPATH];
	string					Vst_Selected_SaveData_FilePath;
	bool					bVst_EmbedRequested;
	BASS_VST_INFO*			Vst_Info;

	//Bass FX
	///8 Band EQ
	HFX						Eq8Fx[MAX_EQ_BANDS];
	BASS_BFX_BQF			Eq8Param[MAX_EQ_BANDS];
	bool					bUser_Eq8_Enabled;

	//fft Levels
	float					Output_Level_L;
	float					Output_Level_R;
	float					Output_Level_L_Smooth;
	float					Output_Level_R_Smooth;
	//Spectrum FFT
	bool					bFFT_Enabled;
	float					FFT[MAX_FFT];
	float					FFT_Smooth[MAX_FFT_USED];
	float					FFT_Scale[MAX_FFT_USED];
	//---------------------------------------------------------
	//constructors
	bass_audio(void);
	~bass_audio(void);
	//---------------------------------------------------------
	//System
    void Init(void);
	void Exit(void);

	//Settings
	void SetVolume(int inVolume);

	//Stream
    void StreamCreate(const char* filepath);
	void StreamStop(void);

	void StreamGetLength(void);
	void StreamSetPosition(double Position);
	void StreamGetPosition(void);

	//Tags
	void ObtainTags(int Index, PL_ENTRY &entry);

	//DSP
	void Eq_Reset(void);
	void DSP_Create(void);
	void DSP_Update(void);
	void DSP_Destroy(void);
	void Vst_Reset(void);

	//BassMix

	//Spectrum
	void Spectrum_Update(void);
	void Spectrum_Reset(void);
	//Controls
	void Play(void);
	//-------------------------------------------------------------
private:
	//-------------------------------------------------------------
	//streams - Main
	HSTREAM							Stream;
	////Stream - Mix
	//HSTREAM							Stream_Mix;
	//-------------------------------------------------------------
	void StreamGetState(void);
	void Settings_Save(void);
	void Settings_Load(void);
	//-------------------------------------------------------------
};
