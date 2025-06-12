#include<windows.h>
#include<psapi.h>
#include<string.h>
#include<tlhelp32.h>
#include "resourcefile.h"
#include<string.h>



#define MAX_APPS 50
#define UPDATE_INTERVAL 1000

typedef struct{
    char processName[256];
    char appName[256];
    int timeSpent;
    int timeLimit;
    DWORD p_id;
    BOOL popUpShown;
} AppUsage;

AppUsage appUsages[MAX_APPS];
int appCount=0;
HWND hwndmain, hwndList, hwndTop5List, hwndIncButton, hwndDecButton;
HINSTANCE hInstance;
int selectedAppIndex=-1;

BOOL IsUserApplication(DWORD p_id){
    HANDLE process=OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, p_id);
    if (process){
        DWORD priorityClass = GetPriorityClass(process);
        CloseHandle(process);
        return (priorityClass == NORMAL_PRIORITY_CLASS || priorityClass == HIGH_PRIORITY_CLASS);
    }
    return FALSE;
}

void GetFriendlyName(const char *processName, char *friendlyName, int size){
    if(strcmp(processName, "notepad.exe") == 0){
        strncpy(friendlyName, "Notepad", size);
    } else if(strcmp(processName, "chrome.exe") == 0){
        strncpy(friendlyName, "GoogleChrome", size);
    } else if(strcmp(processName, "outlook.exe") == 0){
        strncpy(friendlyName, "Microsoft Outlook", size);
    } else if(strcmp(processName, "EXPLORER.EXE") == 0){
        strncpy(friendlyName, "File Explorer", size);
    } else if(strcmp(processName, "firefox.exe") == 0){
        strncpy(friendlyName, "Mozilla Firefox", size);
    } else if(strcmp(processName, "Spotify.exe") == 0){
        strncpy(friendlyName, "Spotify", size);
    } else if(strcmp(processName, "teams.exe") == 0){
        strncpy(friendlyName, "Microsoft Teams", size);
    } else if(strcmp(processName, "zoom.exe") == 0){
        strncpy(friendlyName, "Zoom", size);
    } else if(strcmp(processName, "vlc.exe") == 0){
        strncpy(friendlyName, "VLC Media Player", size);
    } else if(strcmp(processName, "discord.exe") == 0){
        strncpy(friendlyName, "Discord", size);
    } else if(strcmp(processName, "skype.exe") == 0){
        strncpy(friendlyName, "Skype", size);
    } else if(strcmp(processName, "mspmsn.exe") == 0){
        strncpy(friendlyName, "Windows Messenger", size);
    } else if(strcmp(processName, "steam.exe") == 0){
        strncpy(friendlyName, "Steam", size);
    } else if(strcmp(processName, "visualstudio.exe") == 0){
        strncpy(friendlyName, "Visual Studio", size);
    } else if(strcmp(processName, "wordpad.exe") == 0){
        strncpy(friendlyName, "WordPad", size);
    } else if(strcmp(processName, "msedge.exe") == 0){
        strncpy(friendlyName, "Microsoft Edge", size);
    } else if(strcmp(processName, "iexplore.exe") == 0){
        strncpy(friendlyName, "Internet Explorer", size);
    } else if(strcmp(processName, "Explorer.exe") == 0){
        strncpy(friendlyName, "Windows Explorer", size);
    } else if(strcmp(processName, "Code.exe") == 0){
        strncpy(friendlyName, "VS Code", size);
    } else if(strcmp(processName, "olk.exe") == 0){
        strncpy(friendlyName, "Outlook", size);
    } else{
        strncpy(friendlyName, processName, size);
    }
}

int GetActiveProcessName(char *processName, int size, DWORD *p_id){
    HWND hwnd= GetForegroundWindow();
    if(hwnd){
        GetWindowThreadProcessId(hwnd, p_id);
        HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, *p_id);
        if (process){
            HMODULE hMod;
            DWORD cbNeeded;
            if( EnumProcessModules(process, &hMod, sizeof(hMod), &cbNeeded)){
                GetModuleBaseName(process, hMod, processName, size);
            }
            CloseHandle(process);
            return -1;
        }
    }
    return 0;
}

AppUsage* GetOrAddAppUsage(const char *processName, DWORD p_id){
    if(stcmp(processName, "AppUsageTracker,exe") == 0 || strcmp(processName,"ShellExperienceHost.exe") ==0 || strcmp(processName, "WindowsTerminal.exe") == 0 || strcmp(processName, "SearchHost.exe") == 0) {
        return NULL;
    }
    for(int i=0;i<appCount;i++){
        if(strcmp(appUsages[i].processName, processName) == 0){
            return &appUsages[i];
        }
    }
    if(appCount<MAX_APPS){
        strncpy(appUsages[appCount].processName, processName, sizeof(appUsages[appCount].processName) -1);
        GetFriendlyName(processName, appUsages[appCount].appName, sizeof(appUsages[appCount].appName)-1);
        appUsages[appCount].timeSpent = 0;
        appUsages[appCount].timeLimit = 60;
        appUsages[appCount].p_id = p_id;
        appUsages[appCount].popUpShown = FALSE;
        return &appUsages[appCount++];
    }
    return NULL;
}