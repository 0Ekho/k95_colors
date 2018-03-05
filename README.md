# Small simple program to change the colour of a corsair K95 (or K70) keyboard on Linux
## Corsair keyboard Color Changer
*probably broken and certainly badly written*

### Replaced my keyboard and no longer have a corsair so archiving this repository.

had a Corsair K95 for a while that I picked up on sale and decided that I was tired of the boring white backlight you get without the windows software  
It probably won't crash your system or make anything explode but I wouldn't trust me  
especially as it is intended to be run as root.

*I have come to relize I have no idea if it's supposed to be wrote color or colour and that I regularly use both..*

## Compilation / Usage
you will need libusb.  
compile: `make`;  
if you define K70 (ex: `K70=1 make`) it should work with the K70 instead but I don't have one to test with.  
usage: `sudo ckcc -r # -g # -b #` with red green and blue values of 0-7 (7 = off, 0 = max) to fill your KB with a given colour  
your can also use `sudo ckcc -f path/to/file` to set individual keys from a configuration file (more info below)  
`ckcc -h` to show help

## Configuration file

__see example_config.txt for a list of all the valid keys.__  
*any keys left out of the configuration file will default to white or if given, the value passed with -r -g -b*

key=r,g,b;  
whitespace is ignored  
'=' (equals) is used to assign a value to a key  
values are in Red, Green, Blue order seperated by ',' (commas)  
key=value pairs are terminated by ';' (semicolon)  

### K95 512 color RGB protocol Reverse Enginnering thanks to CalcProgrammer1, these posts saved me from a while of messing around with captued packets:
* <https://www.reddit.com/r/MechanicalKeyboards/comments/2ipync/corsair_k70_rgb_usb_protocol_reverse_engineering/>
* <https://www.reddit.com/r/MechanicalKeyboards/comments/2ipync/corsair_k70_rgb_usb_protocol_reverse_engineering/>

More information on the RGB protocol in rgb_protocol.md
