// Chinese USB/HID relay command line tool:
//
// pa02 20-Nov-2014 supports 1,2,4 - relay devices
//
// Build for Windows: using VC++ 2008 and WDK7.1
//~~~~~~~~~~~~~~~~~~~~~~~~

/* Prototype: V-USB example: vusb-20121206/examples/hid-data/commandline/hidtool.c 
 * Author: Christian Starkjohann
 * Creation Date: 2008-04-11
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 */

#define A_VER_STR "r1.2"
#define A_URL "http://vusb.wikidot.com/project:driver-less-usb-relays-hid-interface"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hidusb-tool.h"

#define USB_RELAY_VENDOR_NAME     "www.dcttech.com"
#define USB_RELAY_NAME_PREF       "USBRelay"  // + number


static int g_max_relay_num = 0;

static int rel_read_status_raw(USBDEVHANDLE dev, void *raw_data);

/* ------------------------------------------------------------------------- */

static void usage(char *myName)
{
    char *p = strrchr(myName, '\\'); /* windows */
    if (p) myName = p + 1;
    else p = strrchr(myName, '/'); /* whatever */
    if (p) myName = p + 1;

    fprintf(stderr, "USBHID relay utility, " A_VER_STR " " A_URL "\n\n");
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  %s on <num>  - turn relay <num> ON\n", myName);
    fprintf(stderr, "  %s off <num> - turn relay <num> OFF\n", myName);
    fprintf(stderr, "  %s state     - print state of the relays\n", myName);
    fprintf(stderr, "  %s enum      - print state of all relay devices\n", myName);
    fprintf(stderr, "\nParameter ID=XXXXX selects one device if several are connected.\n");
    fprintf(stderr, "Example: %s ID=ABCDE on 1\n", myName);
}


static const char *usbErrorMessage(int errCode)
{
    static char buffer[80];

    switch (errCode) {
        case USBOPEN_ERR_ACCESS:      return "Access to device denied";
        case USBOPEN_ERR_NOTFOUND:    return "The specified device was not found";
        case USBOPEN_ERR_IO:          return "Communication error with device";
    }

    snprintf(buffer, sizeof(buffer), "Unknown USB error %d", errCode);
    return buffer;
}

// Data for enumeration func:
static struct
{
    USBDEVHANDLE mydev;
    char id[10];
} g_enumCtx;


static int enumFunc(USBDEVHANDLE dev, void *context)
{
    static const char vendorName[] = USB_RELAY_VENDOR_NAME; 
    static const char productName[] = USB_RELAY_NAME_PREF;
    int err;
    char buffer[128*sizeof(short)]; // max USB string is 128 UTF-16 chars
    int num = 0;
    int i;

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
    if ( i != strlen(productName) + 1 )
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
        goto next;
    }

     /* Check the unique ID: 5 bytes at offs 0 */
    err = rel_read_status_raw(dev, buffer);
    if( err < 0 )
    {
        fprintf(stderr, "error reading report 0: %s\n", usbErrorMessage(err));
        goto next;
    }
    
    //hexdump(buffer + 1, sizeof(buffer) - 1);
    for (i=1; i <=5; i++)
    {
        unsigned char x = (unsigned char)buffer[i];
        if (x <= 0x20 || x >= 0x7F)
        {
            fprintf(stderr, "Bad device ID!\n");
            goto next;
        }
    }

    if( buffer[6] != 0 )
    {
        fprintf(stderr, "Bad device ID!\n");
        goto next;
    }

    DEBUG_PRINT(("Device %s%d found: ID=[%5s]\n", productName, num, &buffer[1]));
    g_max_relay_num = num;

    if ( g_enumCtx.id[0] != 0 )
    {
        if ( 0 != memcmp(g_enumCtx.id, &buffer[1], 5) )
            goto next;
    }
#if 0
    if ( g_enumCtx.mydev )
    {
        fprintf(stderr, "ERROR: More than one relay device found. ID must be specified\n");
        usbhidCloseDevice(dev);
        usbhidCloseDevice(g_enumCtx.mydev);
        return 0;
    }
#endif
    g_enumCtx.mydev = dev;
    return 0;

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
        fprintf(stderr, "error finding USB relay: %s\n", usbErrorMessage(err));
        return NULL;
    }

    return g_enumCtx.mydev;
}

/* ------------------------------------------------------------------------- */
#if 0
static void hexdump(char *buffer, int len)
{
int     i;
FILE    *fp = stdout;

    for(i = 0; i < len; i++){
        if(i != 0){
            if(i % 16 == 0){
                fprintf(fp, "\n");
            }else{
                fprintf(fp, " ");
            }
        }
        fprintf(fp, "0x%02x", buffer[i] & 0xff);
    }
    if(i != 0)
        fprintf(fp, "\n");
}
#endif

/* ------------------------------------------------------------------------- */

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
        fprintf(stderr, "error reading status: %s\n", usbErrorMessage(err));
        return -1;
    }

    if ( len != 9 || buffer[0] != reportnum ) {
        fprintf(stderr, "ERROR: wrong HID report returned! %d\n", len);
        return -2;
    }

    if (raw_data) {
        /* copy raw report data */
        memcpy( raw_data, buffer, sizeof(buffer) );
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
        fprintf(stderr, "Invalid relay number. Must be 1-%d or ALL)\n", g_max_relay_num);
        return 1;
    }

    memset(buffer, 0, sizeof(buffer));
    buffer[0] = 0; /* report # */
    buffer[1] = is_on ? 0xFF : 0xFD;
    buffer[2] = (unsigned char)relaynum;
    if((err = usbhidSetReport(dev, (void*)buffer, 9)) != 0) {
        fprintf(stderr, "Error writing data: %s\n", usbErrorMessage(err));
        return 1;
    }
    
    // Read back & verify
    err = rel_read_status_raw(dev, NULL);
    if ( err >= 0 ) {
        err = (err >> (unsigned)(relaynum -1)) & 1;
        err ^= !!is_on;
    }

    if ( err ) {
        fprintf(stderr, "Error: failed set %s relay %u\n", is_on ? "ON":"OFF", relaynum);
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
    if ( err < 0 ){
        fprintf(stderr, "error reading data: %s\n", usbErrorMessage(err));
        err = 1;
    } else {
        //hexdump(buffer + 1, len - 1);
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
            default:
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
        fprintf(stderr, "Error finding USB relay: %s\n", usbErrorMessage(err));
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
        if (strlen(&arg1[3]) != 5) {
            fprintf(stderr, "ERROR: ID must be 5 characters (%s)\n", arg1);
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

/* ------------------------------------------------------------------------- */
