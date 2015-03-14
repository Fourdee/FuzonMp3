
// define entry function
// use WinMain in Windows, otherwise main() on all other platforms
//#ifndef WIN64
#define FUZONMP3 int main(int argc, char **argv) 
//#else
//#define FUZONMP3 int __stdcall WinMain(void* hInstance, void* hPrevInstance, void* lpCmdLine, int nCmdShow)
//#endif


// forward declarations and constants
void CB_Update(void*);
void Timer_Update(void);

void Init_OptionsMenu(void);

void Play_Id(int PlaylistId);
void Play_NextFile(void);

void Info_Seekbar(void);
void Info_PlayButton(void);
void Info_Spectrum(void);
void Info_ShuffleButton(void);
void Info_OE2VisButton(void);
void Info_Volume(void);
void Info_Eq(void);
void Info_Vst(void);
void Info_Search(void);

void Embed_Vst_Window(void);
void Info_PlaylistDirectory(void);

void Intro_Animation(void);

void BottomPanel_Animation(void);
void BottomPanel_Controller(void);

void Playlist_Rescan(void);
void PlaylistEntries_FromScan(void);
void PlaylistEntries_FromSearch(void);

void Clear_Search(void);

void Debug(void);
