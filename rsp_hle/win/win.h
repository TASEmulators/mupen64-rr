BOOL CALLBACK ConfigDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
HINSTANCE dll_hInstance;
void SaveSettings();
void LoadSettings();
extern char AppPath[MAX_PATH];
extern char audioname[100];
extern BOOL AudioHle; // TRUE=audio lists aren't processed in the rsp plugin
extern BOOL GraphicsHle; // TRUE=gfx lists aren't processed in the rsp pluign
extern BOOL SpecificHle; // TRUE=audio lists are processed by a specific audio plugin
                         // otherwise it's processed by the audio plugin choosen in the emu
