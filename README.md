# Fedora Media Writer - Persistent storage

## Summer of code with Google

I worked on a feature for the Fedora Media Writer that makes it possible to
persistently store data while booted into a live system. The Fedora Media
Writer makes the portable media device bootable using an ISO image.

The Fedora Media Writer is written in C++ and targets Linux, Mac and even
Windows users.

## Work done

My work can be found for each repository at the following locations (ordered by
amount of work done):

* [MediaWriter - Fedora Media Writer Qt/C++](
https://github.com/MartinBriza/MediaWriter/commits/gsoc2017?since=2017-05-30&until=2017-08-29&author=squimrel)
* [iso9660io - Manipulate ISO 9660 images](
https://github.com/squimrel/iso9660io/commits/master?since=2017-05-30&until=2017-08-29&author=squimrel)
* [isomd5sum - Store and check checksum on ISO 9660 image](
https://github.com/squimrel/isomd5sum/commits/gsoc2017?since=2017-05-30&until=2017-08-29&author=squimrel)
* [udisks - DBus device management daemon](
https://github.com/storaged-project/udisks/commits/master?since=2017-05-30&until=2017-08-29&author=squimrel)
* [libblockdev - Used by udisks to manipulate block devices](
https://github.com/storaged-project/libblockdev/commits/master?since=2017-05-30&until=2017-08-29&author=squimrel)

All changes are also available in [this patch
directory](https://github.com/MartinBriza/MediaWriter/commits/gsoc2017/patches/).

* In total git says that I added 7.5k lines and removed 5k lines of code.
* I removed around 0.2k more lines from the MediaWriter than I added.

## Community Bonding

In the community bonding period I looked at other well known projects that can
create bootable devices with persistent storage. Most was Linux only and the
rest was horrible. The approach of the projects I've looked at is to create
a FAT partition on the disk, copy what's needed from the ISO image and make the
disk bootable.

My mentor suggested that we should rather not change the way how the portable
media device is currently made bootable with the Fedora Media Writer. Which is
by copying the ISO image directly to disk (dd-like).

## Manipulate an ISO 9660 image

Therefore we took an approach that I've not yet seen implemented anywhere.
I manipulated the ISO image, which is supposed to be a read-only file system
in-place without extracting and repacking. To do that that I wrote a library
that messes with the ISO 9660 file system.

The main task was to modify a couple `grub.cfg` files to make persistent
storage happen. Dracut does the rest for me. The difficulty was to edit
`grub.cfg` files which are inside of an HFS+ or FAT image that is stored on the
ISO 9660 image. Since I didn't find any cross-platform C or C++ library for
dealing with that either and I didn't want to write a library for stuff like
this again the result is a bunch of hacky code that at least works.

## Create writable space for the overlay

Since ISO 9660 is a read-only file system I needed to create and format another
partition behind the current one. This was trouble mainly due to the isohybrid
layout but also due to the fact that this had to work on all three platforms
mentioned above and that I couldn't find a library that just does this for me.

I ended up manually adding the partition to the MBR partition table using C++
and then wrote up some code that creates a FAT32 partition with an
`OVERLAY.IMG` file that is used to persistently store data. I made some
mistakes along the way which were fixed in the last week of summer of code.

## Additionally

Along the way I also submitted code to udisks and libblockdev since the initial
idea was to add the partition through their interface at least on Linux but
isohybrid stood in the way.

I tinkered a bit with isomd5sum in community bonding period. A project that is
used to store the checksum of an ISO image inside the ISO image itself which is
then used at boot time to check if the media is alright and it's also used by
the Fedora Media Writer.
It isn't a proper dependency though since its source code was simply dropped
into the FMW repository at some point which is against the Fedora Packaging
guidelines. To make it a proper dependency it needs to become a MinGW package
and it had to be maintained a bit after I refactored it in the Community
Bonding period so I also worked on that during summer of code.

## What's left

### Windows

This does not yet work on Windows. I started debugging Windows late but
eventually I figured out that on the file descriptor `_open_osfhandle` gave me
I have to perform 512-byte aligned operations which I don't do when dealing
with persistent storage. I'm not quite sure how to fix this properly but apart
from that enabling persistent storage would work on Windows too.

### isomd5sum

Still needs to appear as a MinGW package. bcl is a great developer who
maintains a lot of projects so he doesn't have much time for my isomd5sum PRs.
Therefore there's still a PR waiting to be merged which consists of MinGW
support and a build script for it.
This is important because if the code I wrote would be merged the Windows build
would not work without this package.

### iso9660io

Still needs to pass the package review process so that Fedora Media Writer can
link against it.
