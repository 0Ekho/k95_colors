# Small simple program to change the colour of a corsair K96 keyboard on Linux
## Corsair K95 Color Changer
*probably broken and certainly badly written*

had a Corsair K95 for a while I picked up on sale and decided that I was tired of the boring white backlight you get without the windows software  

it probably won't crash your system or make anything explode but I wouldn't trust me  
especially as it is inteded to be run as root. I am not responsable it breaks/crashes something.

*I have come to relize I have no idea if it's supposed to be wrote color or colour and that I regularly use both..*

## Usage
needs libusb  
`mkdir -p bin`  
compile: `make ckcc`; it should work for the K70 also as long as you change the usb Product ID from 1B11 to 1B13  but I don't have one to test with  
run `sudo bin/ckcc -r # -g # -b #` with red green and blue values of 0-7 (7 = off, 0 = max) to fill your KB with given colour  
your can also use `sudo bin/ckcc -f path/to/file` to set individual keys from a config file (more info below)

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
