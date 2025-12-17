# Bazzite Media Writer

Bazzite Media Writer is a tool that helps users put Bazzite images on their portable drives such as flash disks.

It is able to automatically download the required image for them and write them in a `dd`-like fashion, using either `dd` itself or some other way to access the drive directly.

This overwrites the drive's partition layout though so it also provides a way to restore a single-partition layout with a FAT32 partition.

![Bazzite Media Writer running](/dist/screenshots/linux_main.png)

## Troubleshooting

If you experience any problem with the application, like crashes or errors when writing to your drives, please open an issue here on Github.

Please don't forget to attach the `BazziteMediaWriter.log` file that will appear in your Documents folder (`$HOME/Documents` on Linux and Mac, `%USERPROFILE%\Documents` on Windows). It contains some non-sensitive information about your system and the log of all events happening during the runtime.

## Building

You can build Bazzite Media Writer using the default Qt `cmake` build system. The gist for all three platforms is written below. For a more thorough look into how the releases are composed, you can read our GitHub Actions configuration.

### Linux

You should specify the target directory using the `-DCMAKE_INSTALL_PREFIX` `cmake` option. The default prefix path is `/usr/local`

It can be done like this:

`cmake [OPTIONS] .`

The main binary, `mediawriter`, will be written to `$PREFIX/bin` and the helper binary can be found on the path `$PREFIX/libexec/mediawriter/helper`.

#### Requirements

* `udisks2` or `storaged`
* `xz-libs`

### Windows

Building Bazzite Media Writer in Windows is just the matter of running `cmake` and `make` - as long as you have all dependencies in your include path.

To create a standalone package, use the `windeployqt` tool, included in your Qt installation. You will probably have to include a bunch of not included DLLs.

It is also possible to crosscompile the application using the `MinGW` compiler suite in Fedora (and probably some other distros).

#### Requirements

* `xz-libs`

### macOS

Again, you can just run `cmake` and `make`.

To release a standalone package, use `macdeployqt`, supplied with your Qt installation.

#### Requirements

* `xz-libs`

## Translation

If you want to help with translating Fedora Media Writer, please visit our [Weblate project page](https://translate.fedoraproject.org/projects/fedora-media-writer/mediawriter/).

Information about the individual Fedora flavors is retrieved from the websites and translated as a separate project.

## Other information

For details about cryptography, see [CRYPTOGRAPHY.md](CRYPTOGRAPHY.md).

Some brief privacy information (regarding User-Agent strings) is in [PRIVACY.md](PRIVACY.md).
