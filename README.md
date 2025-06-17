# Screen-time-Monitor

The **Screen Time Monitor** is a multi-platform application developed to help users track and control the duration of their device usage.The system enables users to keep an eye on active **application usage**, define **usage time limits**, and get **notified** when those limits are surpassed.Compatible with both **Windows** and **macOS**, it offers customized features specific to each OS.

---

## Features

- **Cross-Platform Application Tracking**: Monitors active usage of applications on both Windows and macOS.
- **Customizable Time Limits**: Users can set, adjust, and dynamically modify time restrictions for individual apps.
- **Usage Notifications and Enforcement**: Sends alerts when time limits are exceeded; on macOS, apps can be terminated via dialog box.
- **User-Friendly Application Names**: Displays readable names instead of process names for better clarity across platforms.
- **Usage Insights**: Provides real-time statistics on macOS and highlights top 5 most used applications on Windows.

---

## Installation

### Requirements

#### Windows  
- **Operating System**: Windows 7 or later.  
- **Compiler**: A C compiler (e.g., GCC).  
- **Library**: `psapi` for interacting with system processes.  

#### macOS  
- **Operating System**: macOS Mojave or later.  
- **Compiler**: A C++ compiler supporting Objective-C++ (e.g., `clang`).

---

### Installation Steps
#### Windows
1. Clone the repository or download the project files.  
2. Open a terminal/command prompt and navigate to the project directory.  
3. Compile the code:
   
    ```bash
    gcc -o Screen-Time-Monitor AppUsageTracker.c -lpsapi
    ```  
5. Run the program:  
    ```bash
    ./Screen-Time-Monitor
    ```

#### macOS
1. Clone the repository or download the project files.  
2. Open a terminal and navigate to the project directory.  
3. Compile the code with `clang`:
   
    ```bash
    clang++ -o Screen-Time-Monitor AppUsageTracker.mm -framework AppKit -framework Foundation
    ```  
5. Run the program:  
    ```bash
    ./Screen-Time-Monitor
    ```

---

## Supported Applications

### Windows
- Notepad  
- Google Chrome  
- File Explorer  
- Mozilla Firefox  
- Microsoft Outlook  
- Spotify  
- VLC Media Player  
- Microsoft Teams  
- Discord  
- Skype
- Microsoft Edge
- Internet Explorer

### macOS
All actively opened applications are supported.

---

## License  
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
