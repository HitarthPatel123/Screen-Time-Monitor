#import<Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#include <iostream>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <chrono>
#include <string>

using namespace std;
using namespace std::chrono;

// Structure for running application
struct AppUsage {
    string appName;
    int usageLimit =- 1;
    int totalSeconds = 0;
    bool limitReached = false;

    AppUsage() = default;
    AppUsage(const string& name) : appName(name){}
};

unordered_map<string, AppUsage> appUsageMap;
string currentApp = "Terminal";
auto lastSwitchTime = steady_clock::now();
bool running = true;
mutex appUsageMutex;

// Notification sent when application's usage limit is reached
void Notify(const string& appName){
    string script = "osascript -e 'display notification \"" + appName + 
                    " has reached its usage limit.\" with title \"App Usage Limit Reached\"'";
    system(script.c_str());
}

// Terminates particular application
void terminateApp(const string& appName){
    NSArray<NSRunningApplication *> *runningApps = [[NSWorkspace sharedWorkspace] runningApplications];
    for (NSRunningApplication *app in runningApps){
        if([[app localizedName] isEqualToString:[NSString stringWithUTF8String:appName.c_str()]]){
            [app terminate];
            cout << "Terminated application: "<< appName << endl;
            break;
        }
    }
}

// Display dialog box and terminate the application
void showDialogAndTerminate(const string& appName){
    dispatch_async(dispatch_get_main_queue(), ^{
        @autoreleasepool{
            NSAlert *alert = [[NSAlert alloc] init];
            alert.messageText = @"Application usage limit has been reached";
            alert.informativeText = [NSString stringWithFormat:@"%s has reached its usage limit. The app will now terminate.", appName.c_str()];
            [alert addButtonWithTitle:@"OK"];

            if ([alert runModal] == NSAlertFirstButtonReturn){
                terminateApp(appName);
            }
        }
    });
}

// Calculates time limit for application and taks necessary action if required
void checkUsage(){
    auto current = steady_clock::now();
    int duration = duration_cast<seconds>(current - lastSwitchTime).count();

    lock_guard<mutex> lock(appUsageMutex);

    if(!currentApp.empty() && duration > 0){
        if(appUsageMap.find(currentApp) == appUsageMap.end()) {
            appUsageMap[currentApp] = AppUsage(currentApp);
        }

        appUsageMap[currentApp].totalSeconds += duration;
        lastSwitchTime = now;

        if(appUsageMap[currentApp].usageLimit !=-1 &&
           appUsageMap[currentApp].totalSeconds >= appUsageMap[currentApp].usageLimit &&
           !appUsageMap[currentApp].limitReached){

            appUsageMap[currentApp].limitReached = true;
            Notify(currentApp);
            showDialogAndTerminate(currentApp);
           }
    }
}

// To display live app usage details
void liveDetails() {
    while(running){
        checkUsage();

        system("clear");

        lock_guard<mutex> lock(appUsageMutex);
        cout << "\nLive application usage details" << endl;
        cout << "-------------------------------" << endl;
        for(const auto& entry : appUsageMap){
            cout << entry.second.appName << ": " << entry.second.totalSeconds << " seconds";
            if(entry.second.usageLimit != -1){
                cout << " (Limit: " << entry.second.usageLimit << " seconds)";
            }
            cout << endl;
        }
        cout << "--------------------------------" << endl; 

        this_thread::sleep_for(chrono::seconds(1));
    }
}

//  Handle App switch events and update active app
void activateAppDidChange(NSNotification *notification){
    auto current = steady_clock::now();
    int duration = duration_cast<seconds>(current - lastSwitchTime).count();

    lock_guard<mutex> lock(appUsageMutex);

    if(!currentApp.empty() && currentApp != "Terminal"){
        appUsageMap[currentApp].totalSeconds += duration;
    }

    NSRunningApplication *app = notification.userInfo[NSWorkspaceApplicationKey];
    currentApp = [app.localizedName UTF8String];

    if(appUsageMap.find(currentApp) == appUsageMap.end()){
        appUsageMap[currentApp] = Appusage(currentApp);
    }

    lastSwitchTime = now;
}

// Main function
int main(){
    @autoreleasepool {
        cout << "Application started" << endl;

        cout <<"Enter Application usage limits in seconds (type 'finish' to finish)" << endl;
        while(true){
            string appName;
            int limit;
            cout << "App Name: ";
            getline(cin, appName);
            if (appName == 'finish') break;
            cout << "Usage limit (in seconds): ";
            cin >> limit;
            cin.ignore();
            appUsageMap[appName] = AppUsage(appName);
            appUsageMap[appName].usageLimit = limit; 
        }

        lastSwitchTime = steady_clock::now();

        thread statsThread(liveDetails);

        [[[NSWorkspace sharedWorkspace] notificationCenter]
            addObserverForName:NSWorkspaceDidActivateApplicationNotification
                    object:nil
                    queue:[NSOperationQueue mainQueue]
                    usingBlock:^(NSNotification *notification) {
                        activateAppDidChange(notification);
        }];

        [[NSRunLoop mainRunLoop] run];

        running = false;
        statsThread.join();
    }
    return 0;
}