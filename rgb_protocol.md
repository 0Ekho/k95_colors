# 512 color RGB protocol *(as I understand it)*
*need to fix english and grammer in this at some point*  

The keyboard colours are set by 5 usb control messages.  
the first 4 contain the colour data and the 5th causes the keyboard to actually display the recived colours

for the first 4 packets
```
typedef unsigned char u8;

struct colour_packet {
	u8 id;
	u8 packet_num;
	u8 data_len;
	u8 pad;
	u8 data[60];
};
```
the id is always 0x7F, assuming this means it is a colour data packet  
packet_num is a value 1-4, telling what packet in the sequence it is  
data_len tells the length of the relavent data; 0x3C for first 3, 0x24 for fourth  
pad = 0x00, seems to just be a pad for header to 32 bits  
data contains the actual colour data split among the 4 packets (explained lower).  
60 bytes for the first 3 and 36 bytes of data padded to 60 for the fourth

for the 5th packet I'm not sure of the exact value meanings but it causes the update
```
struct end_packet {
	u8 id;
	u8 something;
	u8 pad[2];
	u8 command_maybe;
	u8 pad2[59];
};
```
id = 0x07;
something = 0x27;  
pad = { 0 }; pad to 32 bits  
command_maybe = 0xD8;
pad2 = { 0 }; pad to the full 64 bytes for the packet;  

colour data
```
u8 colour_data[3][72];
```
the colour data is made up of three 72 byte frames for the brightness levels of each colour  
with colour_data[0] being the red values  
colour_data[1] being the green values  
colour_data[2] being the blue values  

each byte can range from 0x00 to 0x77  
[assuming little endian (though with all 1 byte values this should not matter) and LSB 0 bit numbering]  
each byte is two seperate 4 bit nibbles with the right 3 bits being a colour value of 0 to 7 (0 means set the colour to max brightness and 7 means set the colour entirely off] and the 4th bit being a left pad (not sure why they don't use it for more color depth)

this allows for 512 possible colours, across 144 keys


# 16.7M color RGB protocol.

looking at the windows software in a VM and my own packet captures it does seem they have updated to be able to enable an actual 16m color mode with a different RGB protocol  
