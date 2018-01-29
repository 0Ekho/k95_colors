/* MIT License

 Copyright (c) 2017 Ekho

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
*/

#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <usb.h>
/* ------------------------------------------------------------------------- */


/* ---------------------------- usb ---------------------------------------- */
void craft_pkts();
static struct usb_device *find_device(uint16_t v, uint16_t p);
void send_control();
/* ---------------------------- colour ------------------------------------- */
void fill_colour(uint8_t r, uint8_t g, uint8_t b);
int set_key_colour(char *k, uint8_t r, uint8_t g, uint8_t b);
/* ---------------------------- file --------------------------------------- */
int parse_file(char *file);
/* ------------------------------------------------------------------------- */

#define U_VID 0x1B1C
#ifndef KB_K70
#define U_PID 0x1B11 /* K95 */
#else
#define U_PID 0x1B13 /* K70 */
#endif /* KB_K70 */

/* set to the last interface, small chance this may need to change between
 * systems. */
#define BINTERFACENUMBER 0x02

static char version[] = "0.0.1";

struct rgb {
    uint8_t r[144];
    uint8_t g[144];
    uint8_t b[144];
} key_colour;

struct key_value {
    char key[8];
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

/* lookup table keyboard keys to key_colour order */
static char keys[144][8] = {
    "~", "ESC", "C_LOCK", "TAB",
    "L_CTRL", "L_SHIFT", "+", "F12",
    "NP_7", "W_LOCK", "MR", "G1",
    "1", "F1", "A", "Q",
    "L_SUPER", "\0", "\0", "PTRSCN",
    "NP_8", "MUTE", "M1", "G2",
    "2", "F2", "S", "W",
    "L_ALT", "Z", "BACK", "S_LOCK",
    "NP_9", "M_STOP", "M2", "G3",
    "3", "F3", "D", "E",
    "\0", "X", "DEL", "BREAK",
    "\0", "M_PREV", "M3", "G4",
    "4", "F4", "F", "R",
    "SPACE", "C", "END", "INSRT",
    "NP_4", "M_PLAY", "G11", "G5",
    "5", "F5", "G", "T",
    "\0", "V", "P_DOWN", "HOME",
    "NP_5", "M_NEXT", "G12", "G6",
    "6", "F6", "H", "Y",
    "\0", "B", "R_SHIFT", "P_UP",
    "NP_6", "N_LOCK", "G13", "G7",
    "7", "F7", "J", "U",
    "R_ALT", "N", "R_CTRL", "]",
    "NP_1", "NP_/", "G14", "G8",
    "8", "F8", "K", "I",
    "R_SUPER", "M", "A_UP", "|",
    "NP_2", "NP_*", "G15", "G9",
    "9", "F9", "L", "O",
    "MENU", "<", "A_LEFT", "NP_-",
    "NP_3", "\0", "G16", "G10",
    "0", "F10", ":", "P",
    "\0", ">", "A_DOWN", "ENTER",
    "NP_0", "NP_+", "G17", "\0",
    "_", "F11", "\"", "[",
    "B_LVL", "?", "A_RIGHT", "\0",
    "NP_DEL", "NP_ENTR", "G18", "\0",
};
/* packets for colour data */
int8_t colour_pkts[4][64] = {
    { 0x7F, 0x01, 0x3C, 0 }, { 0x7F, 0x02, 0x3C, 0 },
    { 0x7F, 0x03, 0x3C, 0 }, { 0x7F, 0x04, 0x24, 0 }
};

struct usb_dev_handle *handle;

/* ---------------------------- usb ---------------------------------------- */

void craft_pkts()
{
    int i, j;
    // buffer makes spliting the data easy
    uint8_t colour_buf[216] = { 0 };

    // craft the colour value data
    j = 0;
    for (i = 0; i < 144; i += 2) {
        colour_buf[j++] = key_colour.r[i] << 4 | key_colour.r[i + 1];
    }
    for (i = 0; i < 144; i += 2) {
        colour_buf[j++] = key_colour.g[i] << 4 | key_colour.g[i + 1];
    }
    for (i = 0; i < 144; i += 2) {
        colour_buf[j++] = key_colour.b[i] << 4 | key_colour.b[i + 1];
    }
    // split the colour data across the 4 packets
    memcpy(&colour_pkts[0][4], &colour_buf[0], 60);
    memcpy(&colour_pkts[1][4], &colour_buf[60], 60);
    memcpy(&colour_pkts[2][4], &colour_buf[120], 60);
    memcpy(&colour_pkts[3][4], &colour_buf[180], 36);
}
static struct usb_device *find_device(uint16_t v, uint16_t p)
{
    struct usb_bus *bus;
    struct usb_device *dev;

    usb_init();
    usb_find_busses();
    usb_find_devices();
    bus = usb_get_busses();
    // iterate over the linked lists checking if the ids match
    for (; bus != NULL; bus = bus->next) {
        for (dev = bus->devices; dev != NULL; dev = dev->next) {
            if ((dev->descriptor.idVendor == v) &&
            (dev->descriptor.idProduct == p)) {
                return dev;
            }
        }
    }
    return NULL;
}
void send_control()
{
    int wrote;
    /* packet sent after colour data to cause the keyboard to refresh */
    static int8_t end_pkt[64] = { 0x07, 0x27, 0x00, 0x00, -40/*0xD8*/, 0 };

    /* Perform USB control message to keyboard */
    printf("sending colors\n");
    wrote = usb_control_msg(handle, 0x21, 0x09, 0x0300, BINTERFACENUMBER,
    (char *)colour_pkts[0], 64,	1000);
    if (wrote < 64) {
        printf("Error: bytes written for packet 1: %02d\n", wrote);
    }
    wrote = usb_control_msg(handle, 0x21, 0x09, 0x0300, BINTERFACENUMBER,
    (char *)colour_pkts[1], 64, 1000);
    if (wrote < 64) {
        printf("Error: bytes written for packet 2: %02d\n", wrote);
    }
    wrote = usb_control_msg(handle, 0x21, 0x09, 0x0300, BINTERFACENUMBER,
    (char *)colour_pkts[2], 64, 1000);
    if (wrote < 64) {
        printf("Error: bytes written for packet 3: %02d\n", wrote);
    }
    wrote = usb_control_msg(handle, 0x21, 0x09, 0x0300, BINTERFACENUMBER,
    (char *)colour_pkts[3], 64, 1000);
    if (wrote < 64) {
        printf("Error: bytes written for packet 4: %02d\n", wrote);
    }
    wrote = usb_control_msg(handle, 0x21, 0x09, 0x0300, BINTERFACENUMBER,
    (char *)end_pkt, 64, 1000);
    if (wrote < 64) {
        printf("Error: bytes written for packet 5: %02d\n", wrote);
    }
}

/* ---------------------------- colour ------------------------------------- */

void fill_colour(uint8_t r, uint8_t g, uint8_t b)
{
    if (r > 7) r = 7;
    if (g > 7) g = 7;
    if (b > 7) b = 7;

    memset(key_colour.r, r, 144);
    memset(key_colour.g, g, 144);
    memset(key_colour.b, b, 144);
}
int set_key_colour(char *k, uint8_t r, uint8_t g, uint8_t b)
{
    int i;

    for (i = 0; i < 144; i++) {
        if (strncmp(keys[i], k, 8) == 0) {
            key_colour.r[i] = r;
            key_colour.g[i] = g;
            key_colour.b[i] = b;
            return i;
        }
    }
    return -1;
}

/* ---------------------------- file --------------------------------------- */

// sorry, should probably rewrite this better soon. this one is bad
// TODO: add proper error messaging for misformated config files
// FEATURE: add a wildcard key to allow for setting default colour (*=)
// FEATURE: add comment support maybe (# comment \n)
int parse_file(char *file)
{
    FILE *fp;
    int32_t c = 0;
    int32_t i = 0;
    int32_t j = 0;
    char buf[32] = { 0 };
    struct key_value pairs[144] = {{{ 0 }, 0, 0, 0}};

    fp = fopen(file, "r");
    if (fp == NULL) {
        return 1;
    }
    // for now just limiting to 144 as it is the max number of keys anyway
    for (i = 0; i < 144; i++) {
        while ((c = fgetc(fp)) != EOF && j < 30) {
            if (c == ' ' || c == '\t' || c == '\n') {
                continue;
            }
            if (c == ';') {
                buf[j++] = c;
                buf[j] = '\0';
                j = 0;
                break;
            }
            buf[j++] = c;
        }
        // parse single key
        if (c != EOF) {
            sscanf(buf, "%7[^=]=%hhu,%hhu,%hhu;", pairs[i].key, &pairs[i].r,
                &pairs[i].g, &pairs[i].b);
        } else {
            break;
        }
    }
    fclose(fp);
    // set values
    for (; i >=0; i--) {
        if (set_key_colour(pairs[i].key, pairs[i].r, pairs[i].g,
            pairs[i].b) < 0) {
                printf("Invalid key: %s\n", pairs[i].key);
            }
    }
    return 0;
}

/* ------------------------------------------------------------------------- */

int main(int argc, char **argv)
{
    struct usb_device *dev;
    char *f_path = NULL;
    int32_t usb_stat;
    // getopt
    int opt;
    int list_keys = 0;
    int show_version = 0;
    int use_file = 0;
    int show_help = 0;
    // colours
    uint8_t r = 0, g = 0, b = 0;

    static char const short_options[] = "r:g:b:f:lVhH";

    // getopt
    while (1) {
        opt = getopt(argc, argv, short_options);
        if (opt == -1) {
            break;
        }
        switch (opt) {
            case 'r':
                sscanf(optarg, "%hhu", &r);
                if (r > 7) {
                    r = 7;
                    printf("max value for -r exceeded, r set to 7\n");
                }
                break;
            case 'g':
                sscanf(optarg, "%hhu", &g);
                if (g > 7) {
                    g = 7;
                    printf("max value for -g exceeded, g set to 7\n");
                }
                break;
            case 'b':
                sscanf(optarg, "%hhu", &b);
                if (b > 7) {
                    b = 7;
                    printf("max value for -b exceeded, b set to 7\n");
                }
                break;
            case 'l':
                list_keys = 1;
                break;
            case 'f':
                use_file = 1;
                f_path = optarg;
                break;
            case 'V':
                show_version = 1;
                break;
            case 'h':
            case 'H':
                show_help = 1;
                break;
            default:
                fprintf(stderr, "Default getopt case reached, \
aborting program.\n");
                return 1;
        }
    }
    if (show_version) {
        printf("Version: %s\n", version);
        return 0;
    }
    if (show_help) {
        printf("Sets the backlight colours for corsair K95 keyboards\n");
        printf("Version: %s\n\n", version);
        // TODO: make help page not terrible.....
        printf("-V,     | show the current version and exit.\n");
        printf("-h, -H  | show this help page and exit.\n");
        printf("-r,     | takes a value 0-7 and fills the board with \
that value.\n");
        printf("-g,     | takes a value 0-7 and fills the board with \
that value.\n");
        printf("-b,     | takes a value 0-7 and fills the board with \
that value.\n");
        printf("-f,     | takes path to a config file containing\n\
              | ~individual key color mappings.\n\
              | ~config file is key values pairs formatted key=r,g,b\n\
              | ~whitespace is ignored\n\
              | ~'=' is used to assign a value to a key\n\
              | ~values are in Red, Green, Blue order seperated by ','\n\
              | ~key=value pairs are terminated by ';'\n");
        printf("-l,     | show all the valid keys for the config \
file and exit.\n");
        return 0;
    }
    if (list_keys) {
        for (r = 0; r < 144; r++) {
            if (keys[r][0] != '\0') {
                printf("%s\n", keys[r]);
            }
        }
        return 0;
    }
    #ifdef U_DBG
    usb_set_debug(2);
    #endif
    // get KB and open USB connection
    printf("Searching for keyboard...\n");
    dev = find_device(U_VID, U_PID);
    if (dev == NULL) {
        printf("Unable to find keyboard.\n");
        return 1;
    }
    handle = usb_open(dev);
    if (handle == NULL) {
        printf("unable to open USB device handle\n");
         return 1;
    }
    usb_stat = usb_claim_interface(handle, BINTERFACENUMBER);
    if (usb_stat == 0) {
        printf("USB interface claimed\n");
    } else {
        printf("Failed to claim USB interface, error: %d\n", usb_stat);
        if (usb_stat == -1) {
            printf("permission denied, try again as root (or using sudo)\n");
        }
        return 1;
    }
    // RGB stuff
    fill_colour(r, g, b);
    if (use_file) {
        printf("parsing configuration file..\n");
        if (parse_file(f_path) > 0) {
            printf("error opening config file, exiting\n");
            usb_release_interface(handle, BINTERFACENUMBER);
            usb_close(handle);
            return 1;
        }
    }
    craft_pkts();
    send_control();
    // clean up the USB connection
    usb_release_interface(handle, BINTERFACENUMBER);
    usb_close(handle);
    printf("Done, colours have been set.\n");
    return 0;
}
