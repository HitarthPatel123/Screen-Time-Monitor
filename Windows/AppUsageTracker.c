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
HWND hwndMain, hwndList, hwndTop5List, hwndIncButton, hwndDecButton;
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

BOOL InputBox(HWND hwndOwner, const char *title, const char *prompt, int *timeLimit){
    char inputBuffer[32];
    sprintf(inputBuffer, "%d", *timeLimit);

    char message[256];
    sprintf(message, "%s\n\nCurrent limit: %d seconds", prompt, *timeLimit);

    int result=messageBox(hwndOwner, message, title, MB_OKCANCEL);
    if(result == IDOK){
        *timeLimit+=60;
        return TRUE;
    }
    return FALSE;
}

void UpdateTop5List(){
    AppUsage top5[MAX_APPS];
    memcpy(top5, appUsages, appCount * sizeof(AppUsage));

    for(int i=0;i<appCount-1;i++){
        for(int j=i+1;j<appCount;j++){
            if(top5[i].timeSpent < top5[j].timeSpent){
                AppUsage temp = top5[i];
                top5[i] = top5[j];
                top5[j] = temp;
            }
        }
    }

    SendMessage(hwndTop5List, LB_RESETCONTENT, 0, 0);
    for(int i=0;i<5 && i<appCount;i++){
        char buffer[256];
        sprintf(buffer, "%s: %d sec", top5[i].appName, top5[i].timeSpent);
        SendMessage(hwndTop5List, LB_ADDSTRING, 0, (LPARAM)buffer);
    }
}

void UpdateUsageList(){
    char buffer[256];
    SendMessage(hwndTop5List, LB_RESETCONTENT, 0, 0);
    for (int i = 0; i < appCount; i++) {
        sprintf(buffer, "%s: %d sec (Limit: %d sec)", appUsages[i].appName, appUsages[i].timeSpent, appUsages[i].timeLimit);
        SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)buffer);
    }
    UpdateTop5List();
}

void AdjustTimeLimit(int adjustment){
    if (selectedAppIndex >= 0 && selectedAppIndex < appCount){
        appUsages[selectedAppIndex].timeLimit+=adjustment;
        UpdateUsageList();
    }
}

void HandleAppWarnings(AppUsage *appUsage){
    if(appUsage->timeSpent >= appUsage->timeLimit && !appUsage->popUpShown){
        appUsage->popUpShown = TRUE;

        SetForegroundWindow(hwndMain);
        KillTimer(hwndMain ,1);

        int result=MessageBox(hwndMain, "Time limit exceeded. Do you want to close the app?",
                              "Warning", MB_YESNOCANCEL | MB_ICONEXCLAMATION);

        if(result == IDYES){
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, appUsage->p_id);
            if(hProcess){
                TerminateProcess(hProcess, 0);
                CloseHandle(hProcess);
            }
        }else if(result == IDCANCEL){
            InputBox(hwndMain, "Adjust Time Limit", "Increase the time limit:", &appUsage->timeLimit);
        }

        SetTimer(hwndMain, 1, UPDATE_INTERVAL, NULL);
        appUsage->popUpShown = FALSE;
    }
}

void UpdateSelectionAndAdjustTimeLimit(HWND hwnd, WPARAM wParam){
    if(HIWORD(wParam) == LBN_SELCHANGE){
        selectedAppIndex = SendMessage(hwndList, LB_GETCURSEL, 0, 0);
    }else if(LOWORD(wParam) == 1){
        if(selectedAppIndex >= 0 && selectedAppIndex < appCount){
            appUsages[selectedAppIndex].timeLimit -= 10;
            UpdateUsageList();
        }
    }
}

LRESULT CALLBACK WindowProcess(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    switch(uMsg){
        case WM_CREATE:
    CreateWindow(
         "STATIC", "App Usage Details",
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        10, 0, 400, 20,
        hwnd, NULL, hInstance, NULL
    );

    hwndList = CreateWindow(
        "LISTBOX", NULL,
        WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_NOTIFY,
        10, 20, 400, 180,
        hwnd, NULL, hInstance, NULL
    );
    
    CreateWindow(
        "STATIC", "Most Used Apps",
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        10, 210, 400, 20,
        hwnd, NULL, hInstance, NULL
    );

    hwndTop5List = CreateWindow(
        "LISTBOX", NULL,
        WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_NOTIFY,
        10, 230, 400, 100,
        hwnd, NULL, hInstance, NULL
    );

            hwndIncButton = CreateWindow("BUTTON", "+", WS_VISIBLE | WS_CHILD,
                                        420, 10, 30, 30, hwnd, (HMENU)1, hInstance, NULL);
            hwndDecButton = CreateWindow("BUTTON", "-", WS_VISIBLE | WS_CHILD,
                                        420, 50, 30, 30, hwnd, (HMENU)2, hInstance, NULL);  
            SetTimer(hwnd, 1, UPDATE_INTERVAL, NULL);
            break;
        case WM_COMMAND:
            UpdateSelectionAndAdjustTimeLimit(hwnd, wParam);
            break;
        case WM_TIMER:
            {
                char processName[256]="";
                DWORD p_id;
                if(GetActiveProcessName(processName, sizeof(processName), &p_id) && IsUserApplication(p_id)){
                    AppUsage *appUsage = GetOrAddAppUsage(processName, p_id);
                    if(appUsage){
                        appUsage->timeSpent++;
                        HandleAppWarnings(appUsage);
                        UpdateUsageList();
                    }
                }
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}