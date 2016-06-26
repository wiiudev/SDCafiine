# SDCafiine (HBL version)

## Usage Instructions
Load the elf with the homebrew launcher (https://github.com/dimok789/homebrew_launcher)

To log the accessed paths, put a file named "ip.txt" on to your sd card at "sd:/wiiu/ip.txt". The file needs to contain ONLY an local IP4 address (e.g. "192.168.0.1") of the PC running the logging server.
Use the udg logger https://github.com/dimok789/loadiine_gx2/tree/master/udp_debug_reader to see the logs. The logging will start when the SD is initialized for the first time (When the game is booting).

Only works with titles that allow access to the SD card !!! Put your files into "SD:/TITLEID" where TITLEID is your 16 byte long TileID.
Example: To replace files eg Smash USA you put it in SD:/0005000010144F00/ which maps to /content, so in TitleID would be /movies, /patch, etc

## Credits
Cafiine creation - chadderz (and MrBean35000vr ?)
Inital SDCafiine creation - golden45 (see https://gbatemp.net/goto/post?id=5680630)
HBL support and further improvements - Maschell
