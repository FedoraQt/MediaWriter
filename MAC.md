# Fedora Media Writer on macOS

![Fedora Media Writer on macOS](/dist/screenshots/mac_main.png)

## Running

The process of running Fedora Media Writer on a Mac computer is the same as every other Mac application.

### System configuration

The application has been developed and tested on macOS Monterey. We recommend you to use the latest release possible.

Oldest supported release is OS X 10.15.

Resource-wise, every Intel-based Mac computer should be able to run Fedora Media Writer.

### Usage

Download the .dmg file from the [Releases](../../releases) section of this page for the latest, then drag-and-drop it to the Applications folder open it from there. The files downloaded from this page are not signed, which means you can get a prompt saying you won't be able to open them.
To avoid this problem, control-click the "Fedora Media Writer" application and use the "Open" option.

![Move the icon to the applications folder and open from there](/dist/screenshots/mac_open.png)


### Known issues 

There is currently no known issue.

## Building

You can build Fedora Media Writer yourself. It has just a few dependencies and building it is a matter of just running a few commands.

### Dependencies

* `Qt6` (`qtbase`, `qtdeclarative`, `qtsvg` and `qtquickcontrols2`)

### Steps

You can use Qt Creator to build and work with the application. If not using that, the process is basically just these two commands you need to run inside the Fedora Media Writer source code directory:

```
cmake
make
```

There is also the [build.sh](/dist/mac/build.sh) script included that I use for building a dmg package for distribution on this site. There are some hardcoded paths which you can modify to match yours. It will then build the application and create a portable `.dmg` package that can be used on other computers.
