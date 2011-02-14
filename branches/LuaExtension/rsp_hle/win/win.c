#include <windows.h>
#include <dirent.h>
#include "win.h"
#include "../Rsp_#1.1.h"
#include "../Audio_#1.1.h"
#include "../winproject/resource.h"

extern RSP_INFO rsp;


static AUDIO_INFO audio_info;
static BOOL (*initiateAudio)(AUDIO_INFO Audio_Info);
void (*getDllInfo)(PLUGIN_INFO *PluginInfo);
static HMODULE audiohandle = NULL;
char audioname[100];


char AppPath[MAX_PATH];

void ShowMessage(LPSTR lpszMessage) 
{ 
   MessageBox(NULL, lpszMessage, "Info", MB_OK); 
}

void getAppFullPath (char *ret) {
    char drive[_MAX_DRIVE], dirn[_MAX_DIR] ;
	char fname[_MAX_FNAME], ext[_MAX_EXT] ;
	char path_buffer[_MAX_DIR] ;
	
	GetModuleFileName(dll_hInstance, path_buffer, sizeof(path_buffer)); 
    _splitpath(path_buffer, drive, dirn, fname, ext);
    strcpy(ret, drive);
    strcat(ret, dirn);
}

typedef struct _plugins plugins;
struct _plugins
{
    char *file_name;
    char *plugin_name;
    HMODULE handle;
    int type;
    plugins *next;
};
static plugins *liste_plugins = NULL, *current;

void insert_plugin(plugins *p, char *file_name,
		   char *plugin_name, void *handle, int type,int num)
{
    if (p->next)
        insert_plugin(p->next, file_name, plugin_name, handle, type, 
                               (p->type == type) ? num+1 : num);
    else
    {
        p->next = malloc(sizeof(plugins));
        p->next->type = type;
        p->next->handle = handle;
        p->next->file_name = malloc(strlen(file_name)+1);
        strcpy(p->next->file_name, file_name);
        p->next->plugin_name = malloc(strlen(plugin_name)+7);
        sprintf(p->next->plugin_name, "%s", plugin_name);
        p->next->next=NULL;
    }
}

void rewind_plugin()
{
   current = liste_plugins;
}

char *next_plugin()
{
   if (!current->next) return NULL;
   current = current->next;
   return current->plugin_name;
}

int get_plugin_type()
{
   if (!current->next) return -1;
   return current->next->type;
}

void *get_handle(plugins *p, char *name)
{
   if (!p->next) return NULL;
   if (!strcmp(p->next->plugin_name, name))
         return p->next->handle;
   else  
         return get_handle(p->next, name);
}
char* getExtension(char *str)
{
    if (strlen(str) > 3) return str + strlen(str) - 3;
    else return NULL;
}

void search_plugins()
{
    DIR *dir;
    char cwd[MAX_PATH];
    char name[MAX_PATH];
    struct dirent *entry;
   
    liste_plugins = malloc(sizeof(plugins));
    liste_plugins->type = -1;
    liste_plugins->next = NULL;
    
    sprintf(cwd, AppPath);
    
    dir = opendir(cwd);
    while((entry = readdir(dir)) != NULL)
    {
        HMODULE handle;
       
        strcpy(name, cwd);
        strcat(name, "\\");
        strcat(name, entry->d_name);
       
        if (getExtension(entry->d_name) != NULL && strcmp(getExtension(entry->d_name),"dll")==0) {
        handle = LoadLibrary(name);
        if (handle)
        {
            PLUGIN_INFO PluginInfo;
            getDllInfo = (void(__cdecl*)(PLUGIN_INFO *PluginInfo))GetProcAddress(handle, "GetDllInfo");
            if (getDllInfo)
            {
                getDllInfo(&PluginInfo);
                if (PluginInfo.Type == PLUGIN_TYPE_AUDIO) {
                insert_plugin(liste_plugins, name, PluginInfo.Name, 
                                             handle, PluginInfo.Type, 0);
                }                             
                         
            }
        }
      }
    }
    current = liste_plugins;
}


void (*processAList)();

BOOL APIENTRY
DllMain (
  HINSTANCE hInst     /* Library instance handle. */ ,
  DWORD reason        /* Reason this function is being called. */ ,
  LPVOID reserved     /* Not used. */ )
{
    switch (reason)
    {
      case DLL_PROCESS_ATTACH:
      case DLL_THREAD_ATTACH:
        dll_hInstance = hInst  ;
        getAppFullPath(AppPath);    
        search_plugins();
        LoadConfig();
        audiohandle = get_handle(liste_plugins, audioname);
        break;

      case DLL_PROCESS_DETACH:
        break;
      case DLL_THREAD_DETACH:
        break;
    }

    /* Returns TRUE on success, FALSE on failure */
    return TRUE;
}

static BYTE fake_header[0x1000];
static DWORD fake_AI_DRAM_ADDR_REG;
static DWORD fake_AI_LEN_REG;
static DWORD fake_AI_CONTROL_REG;
static DWORD fake_AI_STATUS_REG;
static DWORD fake_AI_DACRATE_REG;
static DWORD fake_AI_BITRATE_REG;

void loadPlugin() 
{
   if (!audiohandle) return;
   audio_info.hinst = rsp.hInst;
   audio_info.MemoryBswaped = TRUE;
   audio_info.HEADER = fake_header;
   audio_info.RDRAM = rsp.RDRAM;
   audio_info.DMEM = rsp.DMEM;
   audio_info.IMEM = rsp.IMEM;
   audio_info.MI_INTR_REG = rsp.MI_INTR_REG;
   audio_info.AI_DRAM_ADDR_REG = &fake_AI_DRAM_ADDR_REG;
   audio_info.AI_LEN_REG = &fake_AI_LEN_REG;
   audio_info.AI_CONTROL_REG = &fake_AI_CONTROL_REG;
   audio_info.AI_STATUS_REG = &fake_AI_STATUS_REG;
   audio_info.AI_DACRATE_REG = &fake_AI_DACRATE_REG;
   audio_info.AI_BITRATE_REG = &fake_AI_BITRATE_REG;
   audio_info.CheckInterrupts = rsp.CheckInterrupts;
   initiateAudio = (BOOL (__cdecl *)(AUDIO_INFO))GetProcAddress(audiohandle, "InitiateAudio");
   processAList = (void (__cdecl *)(void))GetProcAddress(audiohandle, "ProcessAList");
   initiateAudio(audio_info);
}

   
BOOL CALLBACK ConfigDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    int index;
    switch(Message)
    {
        case WM_INITDIALOG:
        rewind_plugin();           
            while(get_plugin_type() != -1) {
                switch (get_plugin_type())
                {
                case PLUGIN_TYPE_AUDIO:
                    SendDlgItemMessage(hwnd, IDC_COMBO_AUDIO, CB_ADDSTRING, 0, (LPARAM)next_plugin());
                    break;
                default:
                    next_plugin();
                }
             } 
         
         index = SendDlgItemMessage(hwnd, IDC_COMBO_AUDIO, CB_FINDSTRINGEXACT, 0, (LPARAM)audioname);
            if (index!=CB_ERR) {
                SendDlgItemMessage(hwnd, IDC_COMBO_AUDIO, CB_SETCURSEL, index, 0);
            }
            else   {
                SendDlgItemMessage(hwnd, IDC_COMBO_AUDIO, CB_SETCURSEL, 0, 0);
                SendDlgItemMessage(hwnd, IDC_COMBO_AUDIO, CB_GETLBTEXT, 0, (LPARAM)audioname);
                }
         
         if   (AudioHle==FALSE) {
               CheckDlgButton(hwnd, IDC_ALISTS_INSIDE_RSP, BST_CHECKED);
               EnableWindow( GetDlgItem(hwnd,IDC_COMBO_AUDIO), FALSE );
               break;
         }
         else if (SpecificHle==FALSE){
               CheckDlgButton(hwnd, IDC_ALISTS_EMU_DEFINED_PLUGIN, BST_CHECKED);
               EnableWindow( GetDlgItem(hwnd,IDC_COMBO_AUDIO), FALSE );
               break;    
         }
         else {
               CheckDlgButton(hwnd, IDC_ALISTS_RSP_DEFINED_PLUGIN, BST_CHECKED);
               EnableWindow( GetDlgItem(hwnd,IDC_COMBO_AUDIO), TRUE );
               break;   
         }
                 
        return TRUE;
        
        case WM_CLOSE:
              EndDialog(hwnd, IDOK);  
        break;
        
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                    index = SendDlgItemMessage(hwnd, IDC_COMBO_AUDIO, CB_GETCURSEL, 0, 0);
                    SendDlgItemMessage(hwnd, IDC_COMBO_AUDIO, CB_GETLBTEXT, index, (LPARAM)audioname); 
                    audiohandle = get_handle(liste_plugins, audioname);
                    SaveConfig();
                    EndDialog(hwnd, IDOK);
                break;
                case IDC_ALISTS_INSIDE_RSP:
                    AudioHle = FALSE; 
                    SpecificHle = FALSE;
                    EnableWindow( GetDlgItem(hwnd,IDC_COMBO_AUDIO), FALSE );
                break;
                case IDC_ALISTS_EMU_DEFINED_PLUGIN:
                    AudioHle = TRUE; 
                    SpecificHle = FALSE; 
                    EnableWindow( GetDlgItem(hwnd,IDC_COMBO_AUDIO), FALSE );
                break;
                case IDC_ALISTS_RSP_DEFINED_PLUGIN:
                    AudioHle = TRUE; 
                    SpecificHle = TRUE;  
                    MessageBox(NULL, 
                               "Warning: use this feature at your own risk\n"
                               "It allows you to use a second audio plugin to process alists\n"
                               "before they are sent\n"
                               "Example: jabo's sound plugin in emu config to output sound\n"
                               "        +azimer's sound plugin in rsp config to process alists\n"
                               "Do not choose the same plugin in both place, or it'll probably crash\n"
                               "For more informations, please read the README",
                               "Warning", MB_OK);
                    EnableWindow( GetDlgItem(hwnd,IDC_COMBO_AUDIO), TRUE );
                break;
            }
        break;
        default:
            return FALSE;
    }
    return TRUE;
}  
