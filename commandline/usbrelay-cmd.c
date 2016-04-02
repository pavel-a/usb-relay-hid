// Command line tool for low-cost USB/HID relays
//
// pa02 20-Nov-2014 supports 1,2,4,8 - relay devices
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/* Prototype: V-USB example: vusb-20121206/examples/hid-data/commandline/hidtool.c 
 * Author: Christian Starkjohann
 * Creation Date: 2008-04-11
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 */

#define A_VER_STR "r1.4"
#define A_URL "http://vusb.wikidot.com/project:driver-less-usb-relays-hid-interface"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hidusb-tool.h"

#define USB_RELAY_VENDOR_NAME     "www.dcttech.com"
#define USB_RELAY_NAME_PREF       "USBRelay"  // + number

#define USB_RELAY_ID_STR_LEN      5 /* length of "unique serial number" in the devices */

static int rel_read_status_raw(USBDEVHANDLE dev, void *raw_data);

#define printerr(fmt, ...) fprintf(stderr, fmt, ## __VA_ARGS__);

/* ------------------------------------------------------------------------- */

static void usage(char *myName)
{
    char *p = strrchr(myName, '\\'); /* windows */
    if (p) myName = p + 1;
    else p = strrchr(myName, '/'); /* whatever */
    if (p) myName = p + 1;

    printf("HID USB relay utility, " A_VER_STR " "
                    "For info: " A_URL "\n"
                    "Usage:\n");
    printf("  %s on <num>  - turn relay <num> ON\n", myName);
    printf("  %s off <num> - turn relay <num> OFF\n", myName);
    printf("  %s state     - print states of the relays\n", myName);
    printf("  %s enum      - print state of all found relay devices\n", myName);
    printf("\nParameter ID=XXXXX selects one device if several are connected.\n");
    printf("Example: %s ID=ABCDE ON 2\n\n", myName);
}


static const char *usbErrorMessage(int errCode)
{
    static char buffer[80];
    buffer[0] = 0;
    if ( errCode != USBHID_ERR_UNKNOWN ) {
        usbhidStrerror_r(errCode, buffer, sizeof(buffer));
    }
    if ( 0 == buffer[0] ) {
        snprintf(buffer, sizeof(buffer), "Unknown error (%d)", errCode);
    }
    return buffer;
}

// Data for enumeration func:
static struct
{
    USBDEVHANDLE mydev;
    char id[USB_RELAY_ID_STR_LEN * 2];
} g_enumCtx;


static int g_max_relay_num = 0; // number of relays in the active device


static int enumFunc(USBDEVHANDLE dev, void *context)
{
    static const char vendorName[] = USB_RELAY_VENDOR_NAME; 
    static const char productName[] = USB_RELAY_NAME_PREF;
    int err;
    char buffer[128*sizeof(short)]; // max USB string is 128 UTF-16 chars
    int num = 0;
    int i;

	(context); // ~ unreferenced warning

    err = usbhidGetVendorString(dev, buffer, sizeof(buffer));
    if ( err || 0 != strcmp( buffer, vendorName) )
    {
        goto next;
    }

    err = usbhidGetProductString(dev, buffer, sizeof(buffer));
    if (err)
    {
        goto next;
    }

    i = (int)strlen(buffer);
    if ( i != (int)strlen(productName) + 1 )
    {
        goto next;
    }

    /* the last char of ProductString is number of relays */
    num = (int)(buffer[i - 1]) - (int)'0';
    buffer[i - 1] = 0;

    if ( 0 != strcmp( buffer, productName) )
    {
        goto next;
    }

    if ( num <= 0 || num > 8 )
    {
        printerr("Unknown relay device? num relays=%d\n", num);
        goto next;
    }

     /* Check the unique ID: USB_RELAY_ID_STR_LEN bytes at offset 1 (just after the report id) */
    err = rel_read_status_raw(dev, buffer);
    if( err < 0 )
    {
        printerr("Error reading report 0: %s\n", usbErrorMessage(err));
        goto next;
    }
    
    for (i = 1; i <= USB_RELAY_ID_STR_LEN; i++)
    {
        unsigned char x = (unsigned char)buffer[i];
        if (x <= 0x20 || x >= 0x7F)
        {
            fprintf(stderr, "Bad device ID!\n");
            goto next;
        }
    }

    if( buffer[USB_RELAY_ID_STR_LEN + 1] != 0 )
    {
        printerr("Bad device ID!\n");
        goto next;
    }

    DEBUG_PRINT(("Device %s%d found: ID=[%5s]\n", productName, num, &buffer[1]));
    g_max_relay_num = num;

    if ( g_enumCtx.id[0] != 0 )
    {
        if ( 0 != memcmp(g_enumCtx.id, &buffer[1], USB_RELAY_ID_STR_LEN) )
            goto next;
    }
#if 0
    if ( g_enumCtx.mydev )
    {
        printerr("ERROR: More than one relay device found. ID must be specified\n");
        usbhidCloseDevice(dev);
        usbhidCloseDevice(g_enumCtx.mydev);
        return 0;
    }
#endif
    g_enumCtx.mydev = dev;
    return 0; /* stop */

    next:
    /* Continue search */
    usbhidCloseDevice(dev);
    return 1;
}

static USBDEVHANDLE openDevice(void)
{
    int err;
    err = usbhidEnumDevices(USB_CFG_VENDOR_ID, USB_CFG_DEVICE_ID, &g_enumCtx, enumFunc);

    if ( err || !g_enumCtx.mydev )
    {
        printerr("error finding USB relay: %s\n", usbErrorMessage(err));
        return NULL;
    }

    return g_enumCtx.mydev;
}


// Read state of all relays
// @return bit mask of all relays (R1->bit 0, R2->bit 1 ...) or -1 on error
static int rel_read_status_raw(USBDEVHANDLE dev, void *raw_data)
{
    char buffer[10];
    int err;
    int reportnum = 0;
    int len = 8 + 1; /* report id 1 byte + 8 bytes data */
    memset(buffer, 0, sizeof(buffer));

    err = usbhidGetReport(dev, reportnum, buffer, &len);
    if ( err ) {
        printerr("error reading status: %s\n", usbErrorMessage(err));
        return -1;
    }

    if ( len != 9 || buffer[0] != reportnum ) {
        printerr("ERROR: wrong HID report returned! %d\n", len);
        return -2;
    }

    if (raw_data) {
        /* copy raw report data */
        memcpy( raw_data, buffer, len );
    }

    return (unsigned char)buffer[8]; /* byte of relay states */
}


static int rel_onoff( USBDEVHANDLE dev, int is_on, char const *numstr )
{
    unsigned char buffer[10];
    int err = -1;
    int relaynum = numstr ? atoi(numstr) : 0;
    
    if ( numstr && (0 == strcasecmp(numstr,"all")) ) {
        char x[2] = {'1', 0};
        int i;
        for (i = 1; i <= g_max_relay_num; i++) {
            x[0] = (char)('0' + i);
            err = rel_onoff(dev, is_on, x);
            if (err) break;
        }
        return err;
    }

    if ( relaynum <= 0 || relaynum > g_max_relay_num ) {
        printerr("Invalid relay number. Must be 1-%d or ALL)\n", g_max_relay_num);
        return 1;
    }

    memset(buffer, 0, sizeof(buffer));
    buffer[0] = 0; /* report # */
    buffer[1] = is_on ? 0xFF : 0xFD;
      // ALL ON=0xFE, ALL OFF=0xFC
    buffer[2] = (unsigned char)relaynum;
    if((err = usbhidSetReport(dev, (void*)buffer, 9)) != 0) {
        printerr("Error writing data: %s\n", usbErrorMessage(err));
        return 1;
    }
    
    // Read back & verify
    err = rel_read_status_raw(dev, NULL);
    if ( err >= 0 ) {
        err = (err >> (unsigned)(relaynum -1)) & 1;
        err ^= !!is_on;
    }

    if ( err ) {
        printerr("Error: failed to set relay %u %s\n", relaynum, is_on ? "ON":"OFF");
        return 1;
    }

    return 0;
}


static int show_status(USBDEVHANDLE dev)
{
    int err;
    char buffer[10];
    static const char* on_off[] = {"OFF","ON"};

#define onoff(n) on_off[!!(err & (1U << n))]

    err = rel_read_status_raw(dev, buffer);
    if ( err < 0 ) {
        printerr("Error reading data: %s\n", usbErrorMessage(err));
        err = 1;
    } else {
        switch (g_max_relay_num) {
            case 1:
                printf("Board ID=[%5.5s] State: R1=%s\n", &buffer[1], onoff(0) );
                break;
            case 2:
                printf("Board ID=[%5.5s] State: R1=%s R2=%s\n",
                    &buffer[1],	onoff(0), onoff(1) );
                break;
            case 4:
                printf("Board ID=[%5.5s] State: R1=%s R3=%s R1=%s R4=%s\n",
                    &buffer[1],	onoff(0), onoff(1), onoff(2), onoff(3) );
                break;
            default: /* print as bit mask */
                printf("Board ID=[%5.5s] State: %2.2X (hex)\n", &buffer[1], (unsigned char)err );
                break;
        }
        err = 0;
    }
    return err;
#undef onoff
}

// Enumerate available relay devices

static int showFunc(USBDEVHANDLE dev, void *context)
{
    int err = enumFunc( dev, context );
    if (err != 0 || g_enumCtx.mydev == 0) // not my device, continue
        return err;

    show_status(g_enumCtx.mydev);
    usbhidCloseDevice(g_enumCtx.mydev);
    g_enumCtx.mydev = 0;

    return 1; // continue
}

static int show_relays(void)
{
    int err;
    g_enumCtx.mydev = 0;

    err = usbhidEnumDevices(USB_CFG_VENDOR_ID, USB_CFG_DEVICE_ID, &g_enumCtx, showFunc);
    if ( err )
    {
        printerr("Error finding USB relays: %s\n", usbErrorMessage(err));
        return 1;
    }

    return 0;
}


int main(int argc, char **argv)
{
    USBDEVHANDLE dev = 0;
    int         err;
    char const *arg1 = (argc >= 2) ? argv[1] : NULL;
    char const *arg2 = (argc >= 3) ? argv[2] : NULL;

    if ( !arg1 ) {
        usage(argv[0]);
        return 1;
    }

    if ( strcasecmp(arg1, "enum") == 0 ) {
        err = show_relays();
        return err;
    }

    if ( strncasecmp(arg1, "id=", 3) == 0 ) {
        /* Set the ID for following commands. else use 1st found device.*/
        if (strlen(&arg1[3]) != USB_RELAY_ID_STR_LEN) {
            printerr("ERROR: ID must be %d characters (%s)\n", USB_RELAY_ID_STR_LEN, arg1);
            return 1;
        }
        
        strcpy(g_enumCtx.id, &arg1[3]);

        // shift following params
        arg1 = arg2;
        arg2 = (argc >= 4) ? argv[3] : NULL;
    }

    dev = openDevice();
    if ( !dev )
        return 1;

    if ( strncasecmp(arg1, "stat", 4) == 0 ) { // stat|state|status
        err = show_status(dev);
    }else if( strcasecmp(arg1, "on" ) == 0) {
        err = rel_onoff(dev, 1, arg2);
    }else if( strcasecmp(arg1, "off" ) == 0) {
        err = rel_onoff(dev, 0, arg2);
    }else {
        usage(argv[0]);
        err = 2;
    }

    if ( dev ) {
        usbhidCloseDevice(dev);
    }

    return err;
}

