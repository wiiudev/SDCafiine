# SDCafiine (HBL version)  [![Build Status](https://api.travis-ci.org/Maschell/SDCafiine.svg?branch=master)](https://travis-ci.org/Maschell/SDCafiine)


## What is SDCafiine
SDCafiine is a Homebrew Application for the Nintendo Wii U, that can be loaded with the [homebrew launcher](https://github.com/dimok789/homebrew_launcher). The main feature of this application is the **on-the-fly replacing of files**, which can be used used to loaded modified content from external media (**SD/USB**). It hooks into the file system functions of the WiiU. Whenever a file is accessed, SDCafiine checks if a (modified) version of it present on the SD/US device, and redirect the file operations if needed.

# Which games are supported
In the past SDCafiine only supported games which already had access to the SD Card (for example Super Smash Bros. for Wii U), but **since version 1.4 all games are supported**.
To achieve this, SDCafiine has a built-in a version of [mocha](https://github.com/dimok789/mocha), which is a Custom Firmware with [libiosuhax](https://github.com/dimok789/libiosuhax) support. This allows support for FAT32 devices via [libfat](https://github.com/aliaspider/libfat) and experimental support for NTFS devices via [libntfs](https://github.com/Maschell/libntfs-wiiu).

# Features
- On the fly file **replacing of game files**.
- Support for **replacing files from downloadable content**
- **Built in libiosuhax support** via mocha
- Supports loading files from **SD and USB** (FAT32 and NTFS (highly experimental, can lead to random crashes))
- Support for **multiple modpacks** for as single game.
## How to use it

### Installation of SDCafiine
Like many other homebrew applications for the Wii U, it can't be installed. The application is only installed temporarily, and has to loaded again after each reboot (or entering the system settings). It is enough to copy the files on to the SDCard in a way it can be accessed by the [homebrew launcher](https://github.com/dimok789/homebrew_launcher), or simply download it from the [homebrew app store](https://www.wiiubru.com/appstore/#/)

Example path of the elf on the SD:
```
SD:/wiiu/apps/sdcafiine/sdcafiine.elf
```

### Starting SDCafiine

When the files are on the SDCard, use your prefered method to get into the [homebrew launcher](https://github.com/dimok789/homebrew_launcher) and start SDCafiine. **No** starting of any CFW is required before you can use SDCafiine, it has a built in fork of mocha.
On success, the system menu should load. Now simply start any game and the mods should load.

### Installation of the mods
Before the mods can be loaded, they need to be copied to a SD or USB device. Since version 1.4 also USB devices (FAT32 and NTFS only) are supported via libfat/libntfs.
**In the following "root:/" is corresponding to the root of your SD/USB device**. The basic filepath structure is this:

```
root:/sdcafiine/[TITLEID]/[MODPACK]/content/  <-- for game files. Maps to /vol/content/
root:/sdcafiine/[TITLEID]/[MODPACK]/aoc/      <-- for downloadable content files. Maps to /vol/aocXXXXXXXX/
```
Replace the following:
- "[TITLEID]" need to be replaced the TitleID of the games that should be modded. A list of can be found [here](http://wiiubrew.org/w/index.php?title=Title_database#00050000:_eShop_and_disc_titles) (without the "-"). Example for SSBU "0005000010145000". Make sure to use the ID of the fullgame and not the update title ID. 
- "[MODPACK]" name of the modpack. This folder name can be everything but "content" or "aoc".

Example path for the EUR version of SuperSmashBros for Wii U:
```
root:/sdcafiine/0005000010145000/SpecialChars/content/  <-- for game files. Maps to /vol/content/
```

For replacing the file /vol/content/movie/intro.mp4, put a modified file into:
```
root:/sdcafiine/0005000010145000/SpecialChars/content/movie/intro.mp4
```

*NOTES: paths like "root:/sdcafiine/0005000010145000/content/" are still supported for compatibility, but **not recommended** *
### Handling multiple mod packs
SDCafiine supports multiple different mods for a single game on the same SDCard/USB. Each mod has an own subfolder.
Example:
```
sd:/sdcafiine/0005000010145000/ModPack1/content/  
sd:/sdcafiine/0005000010145000/ModPack2/content/  
usb:/sdcafiine/0005000010145000/ModPack3/content/ 
usb:/sdcafiine/0005000010145000/ModPack4/content/ 
```
When multiple folders are detected, you need to choose which one to use when starting the game. To swap to another mod, you need restart the game.

## Building
Make sure you download the complete repo, including the submodules:  

- git submodule update --init --recursive

For building you need: 
- [libiosuhax](https://github.com/dimok789/libiosuhax) (Build WITHOUT the WUT flag set.)
- [libfat](https://github.com/aliaspider/libfat/)
- [libntfs](https://github.com/Maschell/libntfs-wiiu) (Build with make wiiu-install)
- [dynamic_libs](https://github.com/Maschell/dynamic_libs/tree/lib) for access to the functions.
- [libutils](https://github.com/Maschell/libutils) for common functions.
- [libfswrapper](https://github.com/Maschell/fs_wrapper) to replace the fs functions.

Install them (in this order) according to their README's. Don't forget the dependencies of the libs itself.

*note: when you change something in the mocha folder, you need to use make clean before building.*

## Credits
HBL support, code rewrite and further improvements - Maschell  
minor improvements - Zarklord 
[inital SDCafiine creation](https://gbatemp.net/goto/post?id=5680630) - golden45  
Cafiine creation - chadderz (and MrBean35000vr ?)  
mocha,libiosuhax - dimok789
libfat - devkitPro team
libntfs - libntfs team
