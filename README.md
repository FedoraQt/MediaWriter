# Fedora Media Writer

Fedora Media Writer is a tool that helps users put Fedora images on their portable drives such as flash disks.

It is able to automatically download the required image for them and write them in a `dd`-like fashion, using either `dd` itself or some other way to access the drive directly.

This overwrites the drive's partition layout though, so it also provides a way to restore a single-partition layout with a FAT32 partition.

![Fedora Media Writer running, with expanded image list](/dist/screenshots/expanded.png)

## Status

The tool is still in development, features are added to it over time.

To view the current development status, visit #1 .

## Building

You can build FMW using the default Qt `qmake` build system.

### Linux

You should specify the target directory using the `PREFIX` `qmake` variable. The default prefix path is `/usr/local`

It can be done like this:

`qmake PREFIX=/usr .`

The main binary, `mediawriter`, will be writen to `$PREFIX/bin` and the helper binary can be found on the path `$PREFIX/libexec/mediawriter/helper`.

#### Requirements

* `udisks2` or `storaged`

### Windows

Building FMW in Windows is just the matter of running `qmake` and `make`. There are no requirements now.

To create a standalone package, use the `windeployqt` tool, included in your Qt installation. You will probably have to include a bunch of not included DLLs.

### macOS

Again, you can just run `qmake` and `make`. No requirements so far.

To release a standalone package, use `macdeployqt`, supplied with your Qt installation.

## Translation

If you want to help with translating Fedora Media Writer, please visit our [Zanata project page](https://translate.zanata.org/iteration/view/MediaWriter/4.0).
