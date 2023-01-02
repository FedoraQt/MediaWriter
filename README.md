# Fedora Media Writer

Fedora Media Writer is a  compact, lightweight, comprehensive open-source tool that simplifies linux experience for users by putting Fedora images on their portable drives such as flash disks or USB drives.

This told can automatically download, install, and help users write their flavored Fedora images in a `dd`-like fashion, using either `dd` itself or some other way to access the drive directly.

This overwrites the drive's partition layout though so it also provides a way to restore a single-partition layout with a FAT32 partition.

![Fedora Media Writer running](/dist/screenshots/linux_main.png)

## Troubleshooting

If you experience any problem with the application, like crashes or errors when writing to your drives, please open an issue here on Github.

Also, kindly remember to attach the FedoraMediaWriter.log file that will appear in your Documents folder ($HOME/Documents on Linux and Mac, C:\Users\<user>\Documents). It contains some non-sensitive information about your system and the log of all events happening during the runtime.

## Building

You can build FMW using the default Qt `cmake` build system. The gist for all three platforms is written below. For a more thorough look into how the releases are composed, you can read our [GitHub Actions configuration](https://github.com/FedoraQt/MediaWriter/blob/master/.github/workflows/ccpp.yml).

### Linux

You should specify the target directory using the `-DCMAKE_INSTALL_PREFIX` `cmake` option. The default prefix path is `/usr/local`

Users can run this action using:

`cmake [OPTIONS] .`

You can write the main binary, `mediawriter`, to `$PREFIX/bin` and find the helper binary on the path `$PREFIX/libexec/mediawriter/helper`.

#### Requirements

* `adwaita-qt`
* `udisks2` or `storaged`
* `xz-libs`

### Windows

Building FMW in Windows is just the matter of running `cmake` and `make` - as long as you have all dependencies in your include path. Only MinGW (both 32b and 64b variants) works at this moment.

To create a standalone package, use the `windeployqt` tool, included in your Qt installation. You will probably have to include a bunch of missing DLLs.

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

Users kind find more information about the individual Fedora flavors from the websites and translate them as a separate project.

## Other information

For details about cryptography, see [CRYPTOGRAPHY.md](CRYPTOGRAPHY.md).

Some brief privacy information (regarding User-Agent strings) is in [PRIVACY.md](PRIVACY.md).
