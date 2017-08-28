# Fedora Media Writer on macOS

![Fedora Media Writer on macOS](/dist/screenshots/mac_main.png)

## Running

The process of running Fedora Media Writer on a Mac computer is very similar to every other Mac application.

### System configuration

The application has been developed and tested on macOS Sierra (version 10.12.1). We recommend you to use the latest release possible.

Oldest supported release is OS X Mavericks (10.9).

Resource-wise, every Intel-based Mac computer should be able to run Fedora Media Writer.

### Usage

Download the .dmg file from the [Releases](../../releases) section of this page or [getfedora.org](https://getfedora.org), then open it. The files downloaded from this page are not signed, which means you can get a prompt saying you won't be able to open them.

To avoid this problem, two-finger click the "Fedora Media Writer" application and use the "Open" option as seen on the picture below.

![File option menu used to open the unsigned application](/dist/screenshots/mac_open.png)


### Known issues 

There are several issues or things that are known to need to be improved in the future:

* `osascript` is used instead of the proper way of acquiring privileged access to the system ([#25](../../issues/25))
* The progressbar doesn't report the actual progress of writing ([#26](../../issues/26))

## Building

You can build Fedora Media Writer yourself. It has just a few dependencies and building it is a matter of just running two commands.

### Dependencies

* `Qt5` (`qtbase`, `qtdeclarative` and `qtquickcontrols`)
* `coreutils` - Provides `gtac` which is required to build `iso9660io`.

### Steps

You can use Qt Creator to build and work with the application. If not using that, the process is basically just these two commands you need to run inside the Fedora Media Writer source code directory:

```
qmake
make
```

There is also the [build.sh](/dist/mac/build.sh) script included that I use for building a dmg package for distribution on this site. There are some hardcoded paths which you can modify to match yours. It will then build the application and create a portable `.dmg` package that can be used on other computers.






