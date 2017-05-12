# SDCafiine (HBL version)

## Usage Instructions
Load the elf with the [homebrew launcher](https://github.com/dimok789/homebrew_launcher)

To log the accessed paths, put a file named "ip.txt" on to your sd card at "sd:/wiiu/ip.txt". The file needs to contain ONLY an local IP4 address (e.g. "192.168.0.1" without the quotes) of the PC running the logging server.
Use the [udp logger](https://github.com/dimok789/loadiine_gx2/tree/master/udp_debug_reader) to see the logs. The logging will start when the elf is loaded.

Only works with titles that allow access to the SD card unless you run this version of [mocha](https://twitter.com/Zarklore/status/861812121465745409) !!! Put your files into "SD:/sdcafiine/TITLEID/content" or "SD:/sdcafiine/TITLEID/aoc" where TITLEID is your 16 byte long TileID.
Example: To replace files eg Smash USA you put it in SD:/sdcafiine/0005000010144F00/content which maps to /content or SD:/sdcafiine/0005000010144F00/aoc which maps to /aoc, so in content would be /movies, /patch, etc

## Credits
Cafiine creation - chadderz (and MrBean35000vr ?)
Inital SDCafiine creation - golden45 (see https://gbatemp.net/goto/post?id=5680630)
HBL support and further improvements - Maschell
minor improvements - Zarklord
