# SDCafiine (HBL version) (experimental libfat version)

#EXPERIMENTAL
This version is very experimental and only for testing.!!!! May or may not work.

## Usage Instructions
- Load mocha for full support, otherwise only SSBU is working. HAXCHI/CBHC is NOT supported.  
- Load the elf with the [homebrew launcher](https://github.com/dimok789/homebrew_launcher)  

To use SDCafiine with any game or load mods from USB, you need to load mocha first.  

You can put your mods into the sd card or a FAT32 USB drive. If you have mods on SD Card AND USB, you have to choose which one will be used while starting the game.  
  
Put your files into "root:/sdcafiine/TITLEID/content" or "root:/sdcafiine/TITLEID/aoc" where TITLEID is your 16 byte long TileID (and root is the root of your SD/USB device).  
  
Example: To replace files eg Smash USA you put it in root:/sdcafiine/0005000010144F00/content which maps to /content or root:/sdcafiine/0005000010144F00/aoc which maps to /aoc, so in content would be /movies, /patch, etc  

#Multipe mod packs
It is possible have multiple mod packs on your SDCard/USB device.  
For each modpack you have to create a subfolder in "sdcafiine/TITLEID/" which contain it's own content/aoc folder. Example:

```
sd:/sdcafiine/0005000010144F00/ModPack1/content/  
sd:/sdcafiine/0005000010144F00/ModPack2/content/  
usb:/sdcafiine/0005000010144F00/ModPack2/content/ 
```

You can choose the modpack while starting the game.


## Building
For building you need: 
- [libfat](https://github.com/aliaspider/libfat/)
- [libiosuhax](https://github.com/dimok789/libiosuhax) (Build WITHOUT the WUT flag set.)

## Credits
Cafiine creation - chadderz (and MrBean35000vr ?)  
Inital SDCafiine creation - golden45 (see https://gbatemp.net/goto/post?id=5680630)  
HBL support and further improvements - Maschell  
minor improvements - Zarklord 
