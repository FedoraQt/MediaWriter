# Bazzite Media Writer on macOS

![Bazzite Media Writer on macOS](/dist/screenshots/mac_main.png)

## Running

The process of running Bazzite Media Writer on a Mac computer is the same as every other Mac application.

### System configuration

The application has been developed and tested on macOS Tahoe (26). We recommend you to use the latest release possible.

Oldest supported release is macOS 13.

### Usage

Download the .dmg file from the [Releases](../../releases) section of this page for the latest, then drag-and-drop it to the Applications folder open it from there. 
The files downloaded from this page are not signed, which means you can get a prompt saying you won't be able to open them. In order to be able to
run our pre-built binaries you have to run `xattr -c BazziteMediaWriter-macos-x.x.x.dmg` to get rid of the warning about uknown source.

### Known issues 

There is a known issue in progress bar on macOS 26 in Qt 6.9.3.
Fedora Media Writer issue: https://github.com/FedoraQt/MediaWriter/issues/897

## Building

You can build Bazzite Media Writer yourself. It has just a few dependencies and building it is a matter of just running a few commands.

### Dependencies

* `Qt6` (`qtbase`, `qtdeclarative`, `qtsvg` and `qtquickcontrols2`)

### Steps

You can use Qt Creator to build and work with the application. If not using that, the process is basically just these two commands you need to run inside the Bazzite Media Writer source code directory:

```
cmake
make
```

There is also the [build.sh](/dist/mac/build.sh) script included that I use for building a dmg package for distribution on this site. There are some hardcoded paths which you can modify to match yours. It will then build the application and create a portable `.dmg` package that can be used on other computers.
