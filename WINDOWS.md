# Fedora Media Writer on Windows

![Fedora Media Writer on Windows](/dist/screenshots/win_main.png)

## Running

Fedora Media Writer is a regular Windows application. See dependencies section for required libraries.

### System configuration

The application has been developed for and tested on Windows 10 and Windows 11. Also Qt 6 is supported only on Windows 10 and 11.

You need to have about 200MB of free memory (RAM) available. It will take about 70MB of space on your drive.

### Usage

Download the .exe installer from the [Releases](../../releases) section of this page, then open it.

### Known issues 

There are several issues or things that are known to need to be improved in the future:

* The GUI part of the application has to run as administrator - it does not request permission right before writing.

## Building

You can build Fedora Media Writer yourself. It has just a few dependencies and building it is a matter of just running two commands.

### Dependencies

* `Qt6` (`qtbase`, `qtdeclarative`, `qtsvg` and `qtquickcontrols2`) - already part of the installer
* `Microsoft Visual C++ Redistributable` for C++ (MSVC) runtime libraries - the installer will prompt you to install this if it's not already on your system

### Steps

You can use Qt Creator to build and work with the application. If not using that, the process is basically just these two commands you need to run inside the Fedora Media Writer source code directory:

```
cmake
make
```

### Crosscompilation

There is also the [build.sh](/dist/win/build.sh) script included that I use for building a the installer for distribution on this site. It should do everything automatically if you're in Fedora. There are instructions on how to use it inside at the top of the file.

#### Visual C++ Redistributable Installation

The installer handles the [Microsoft Visual C++ Redistributable (2015-2022)](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170) installation on-demand, which is required for the application to run.

**How it works:**
1. During installation, the installer checks if VC++ Redistributable is already installed
2. If not installed, it prompts the user with a Yes/No dialog:
   ```
   Microsoft Visual C++ Redistributable is not installed on your system.
   
   It is required for Fedora Media Writer to run properly.
   
   Would you like to download and install it now? (approximately 25 MB)
   ```
3. If the user chooses **"Yes"**:
   - Downloads `vc_redist.x64.exe` from Microsoft's official servers
   - Installs it automatically with a progress bar (`/passive` mode)
   - Cleans up the downloaded file
4. If the user chooses **"No"**:
   - Shows a message with a direct download link for manual installation later

**Benefits:**
- Smaller installer size (no bundled redistributable)
- Only downloads when needed
- Always gets the latest version from Microsoft
- Requires internet connection during installation if VC++ is not already installed

**Manual installation:**
If you prefer to install it separately or if the download fails, you can download it manually from:
https://aka.ms/vs/17/release/vc_redist.x64.exe
