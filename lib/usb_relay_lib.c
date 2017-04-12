/**
* USB HID relays API library
* http://git.io/bGcxrQ
*
* This is reconstruction of the original Windows DLL,
* as supplied by the USB relay vendors.
* It is binary compatible and works with their example programs.
* The original usb_relay_device.h file has been slightly hacked up.
*
* 12-jan-2015 pa01 Win32 version
*/

#define MY_VERSION 0x02

#if defined (WIN32) || defined (_WIN32)

// Windows 32 or 64 bit
#include "targetver.h"
#define WIN32_EXTRALEAN
#include <windows.h>

#ifdef _MSC_VER
/* The original DLL has cdecl calling convention */
#define USBRL_CALL __cdecl
#define USBRL_API __declspec(dllexport) USBRL_CALL

#if _MSC_VER < 1900 /* before VS2015 */
#define snprintf   _snprintf
#endif /* VS2015 */
#endif // _MSC_VER
#endif //WIN32

#include "usb_relay_device.h"

#if USBRELAY_LIB_VER != MY_VERSION
#error "Oops. Wrong version of usb_relay_device.h"
#endif

#include "usb_relay_hw.h"
#include "hiddata.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//#define dbgprintf(fmt, ...) fprintf(stderr, fmt, ## __VA_ARGS__)
//#define dbgprintf(fmt, ...) printf(fmt, ## __VA_ARGS__)
#define dbgprintf(fmt, ...)  //__noop(fmt, __VA_ARGS__)
//#define printerr(fmt, ...) fprintf(stderr, fmt, ## __VA_ARGS__)
//#define printerr(fmt, ...) printf(fmt, ## __VA_ARGS__)
#define printerr(fmt, ...) // __noop(fmt, __VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

struct usbrelay_internal_s {
    struct usb_relay_device_info urdi; //public part
    // Private part:
    USBDEVHANDLE usbh; // handle
    char idstr[8];
};

// struct for enum context
struct enumctx_s {
    struct usbrelay_internal_s *head, *tail;
    int numdevs;
    int status;
};

// Globals

static const char *g_dummyPath = "NOTHING"; // passing dev.path to client not implemented, I return this as path.

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

// Turn relay on/off.
// @param relaynum: positive (1-N): one relay index, negative: all, -num = number of relays
// @returns 0 ok, else error
static int rel_onoff( USBDEVHANDLE dev, int is_on, int relaynum )
{
    unsigned char buffer[10];
    int err = -1;
    unsigned char cmd1, cmd2, mask, maskval;

    if ( relaynum < 0 && (-relaynum) <= 8 ) {
        mask = 0xFF;
        cmd2 = 0;
        if (is_on) {
           cmd1 = 0xFE;
           maskval = (unsigned char)( (1U << (-relaynum)) - 1 );
        } else {
            cmd1 = 0xFC;
            maskval = 0;
        }
    } else {
        if ( relaynum <= 0 || relaynum > 8 ) {
            printerr("Relay number must be 1-8\n");
            return 1;
        }
        mask = (unsigned char)(1U << (relaynum-1));
        cmd2 = (unsigned char)relaynum;
        if (is_on) {
            cmd1 = 0xFF;
            maskval = mask;
        } else {
            cmd1 = 0xFD;
            maskval = 0;
        }
    }

    memset(buffer, 0, sizeof(buffer));
    buffer[0] = 0; /* report # */
    buffer[1] = cmd1;
    buffer[2] = cmd2;
    if((err = usbhidSetReport(dev, (void*)buffer, 9)) != 0) {
        printerr("Error writing data: %s\n", usbErrorMessage(err));
        return 1;
    }

    // Read back & verify
    err = rel_read_status_raw(dev, NULL);
    if ( err < 0 ) {
        printerr("Error read back: %s\n", usbErrorMessage(err));
        return 1;
    }

    err = err & mask;
    if (err != maskval) {
        printerr("Error: failed to set relay %u %s\n", relaynum, is_on ? "ON":"OFF");
        return 1;
    }

    return 0;
}


// Public functions:

/** Initialize the USB Relay Library
@returns: This function returns 0 on success and -1 on error.
*/
int USBRL_API usb_relay_init(void)
{
    return 0;
}

/** Finalize the USB Relay Library.
This function frees all of the static data associated with USB Relay Library.
It should be called at the end of execution to avoid memory leaks.
@returns: This function returns 0 on success and -1 on error.
*/
int USBRL_API usb_relay_exit(void)
{
    return 0;
}

// Enum function for building list of devices
static
int enumfunc(USBDEVHANDLE usbh, void *context)
{
//    static const char vendorName[] = USB_RELAY_VENDOR_NAME;
    static const char productName[] = USB_RELAY_NAME_PREF;
    int err;
    char buffer[128*sizeof(short)]; // max USB string is 128 UTF-16 chars
    int num = 0;
    size_t i;
    struct usbrelay_internal_s *q;
    struct enumctx_s *ectx = (struct enumctx_s *)context;

    //NOTE: Ignore vendor string. This is against ObjDev rules, restore the check if needed!

    err = usbhidGetProductString(usbh, buffer, sizeof(buffer));
    if (err)
    {
        goto next;
    }

    i = strlen(buffer);
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
        dbgprintf("Unknown relay device? num relays=%d\n", num);
        goto next;
    }

     /* Check the unique ID: USB_RELAY_ID_STR_LEN bytes at offset 1 (just after the report id) */
    err = rel_read_status_raw(usbh, buffer);
    if( err < 0 )
    {
        dbgprintf("Error reading report 0: %s\n", usbErrorMessage(err));
        goto next;
    }

    for (i = 1; i <= USB_RELAY_ID_STR_LEN; i++)
    {
        unsigned char x = (unsigned char)buffer[i];
        if (x <= 0x20 || x >= 0x7F)
        {
            dbgprintf("Bad usbrelay ID string!\n");
            goto next;
        }
    }

    if( buffer[USB_RELAY_ID_STR_LEN + 1] != 0 )
    {
        dbgprintf("Bad usbrelay ID string!\n");
        goto next;
    }

    dbgprintf("Device %s%d found: ID=[%5s]\n", productName, num, &buffer[1]);

    // allocate & save info
    q = (struct usbrelay_internal_s *)calloc(1, sizeof(struct usbrelay_internal_s));
    if (!q) {
        dbgprintf("Malloc err\n");
        goto next; //$$$ revise
    }
    /* keep this device, continue */
    q->usbh = usbh;
    memcpy(q->idstr, &buffer[1], USB_RELAY_ID_STR_LEN);
    q->urdi.type = num; // enum = number of relays
    q->urdi.serial_number = &q->idstr[0];
    q->urdi.device_path = (char*)g_dummyPath;

    if (!ectx->head) {
        ectx->head = q;
        ectx->tail =q;
    } else {
        ectx->tail->urdi.next = (pusb_relay_device_info_t)q;
        ectx->tail = q;
    }

    ++ectx->numdevs;
    return 1;

    next:
    /* Continue search */
    usbhidCloseDevice(usbh);
    return 1;
}

// Enum function for open one device by ID
static
int enumOpenfunc(USBDEVHANDLE usbh, void *context)
{
//    static const char vendorName[] = USB_RELAY_VENDOR_NAME;
    static const char productName[] = USB_RELAY_NAME_PREF;
    int err;
    char buffer[128*sizeof(short)]; // max USB string is 128 UTF-16 chars
    int num = 0;
    size_t i;
    struct enumctx_s *ectx = (struct enumctx_s *)context;
    struct usbrelay_internal_s *q = ectx->head;

    //NOTE: Ignore vendor string. This is against ObjDev rules, restore the check if needed!

    err = usbhidGetProductString(usbh, buffer, sizeof(buffer));
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
        dbgprintf("Unknown relay device? num relays=%d\n", num);
        goto next;
    }

     /* Check the unique ID: USB_RELAY_ID_STR_LEN bytes at offset 1 (just after the report id) */
    err = rel_read_status_raw(usbh, buffer);
    if( err < 0 )
    {
        dbgprintf("Error reading report 0: %s\n", usbErrorMessage(err));
        goto next;
    }

    for (i = 1; i <= USB_RELAY_ID_STR_LEN; i++)
    {
        unsigned char x = (unsigned char)buffer[i];
        if (x <= 0x20 || x >= 0x7F)
        {
            dbgprintf("Bad usbrelay ID string!\n");
            goto next;
        }
    }

    if( buffer[USB_RELAY_ID_STR_LEN + 1] != 0 )
    {
        dbgprintf("Bad usbrelay ID string!\n");
        goto next;
    }

    dbgprintf("Device %s%d found: ID=[%5s]\n", productName, num, &buffer[1]);

    if ( 0 == memcmp( q->idstr, &buffer[1], USB_RELAY_ID_STR_LEN) ) {
        q->usbh = usbh;
        q->urdi.type = num; // enum = number of relays
        q->urdi.serial_number = &q->idstr[0];
        q->urdi.device_path = (char*)g_dummyPath;
        ++ectx->numdevs;
        return 0;
    }

    next:
    /* Continue search */
    usbhidCloseDevice(usbh);
    return 1;
}

/** Enumerate the USB Relay Devices.*/
pusb_relay_device_info_t USBRL_API usb_relay_device_enumerate(void)
{
    struct enumctx_s ectx;
    int ret;
    memset(&ectx, 0, sizeof(ectx));
    ret = usbhidEnumDevices(USB_CFG_VENDOR_ID, USB_CFG_DEVICE_ID,
                      (void*)&ectx,
                      enumfunc);

    return (pusb_relay_device_info_t)ectx.head;
}


/** Free an enumeration Linked List*/
void USBRL_API usb_relay_device_free_enumerate(struct usb_relay_device_info *dilist)
{
    struct usbrelay_internal_s *p = (struct usbrelay_internal_s *)dilist;

    while (p) {
        struct usbrelay_internal_s *q = (struct usbrelay_internal_s *)((pusb_relay_device_info_t)p)->next;
        if (p->usbh && ((USBDEVHANDLE)(-1)) != p->usbh) {
            usbhidCloseDevice(p->usbh);
            p->usbh = 0;
        }
        free(p);
        p = q;
    }

    return;
}

/** Open device by serial number
serial_number == NULL is valid and means any one device.
@return: This function returns a valid handle to the device on success or NULL on failure.
Example: usb_relay_device_open_with_serial_number("abcde", 5)  */
intptr_t USBRL_API usb_relay_device_open_with_serial_number(const char *serial_number, unsigned len)
{
    struct enumctx_s ectx;
    int ret;
    struct usbrelay_internal_s *q;
    memset(&ectx, 0, sizeof(ectx));

    if (serial_number && len != USB_RELAY_ID_STR_LEN) {
        printerr("Specified invalid str id length: %u", len);
        return (intptr_t)0;
    }

    q = ectx.head = calloc(1, sizeof(*ectx.head));
    if (!q)
        return (intptr_t)0;

    memcpy(q->idstr, serial_number, len);

    ret = usbhidEnumDevices(USB_CFG_VENDOR_ID, USB_CFG_DEVICE_ID,
                      (void*)&ectx,
                      enumOpenfunc);
    if (ret != 0)
        goto ret_err; // error during enum

    if (ectx.numdevs == 0 || q->usbh == 0) {
        goto ret_err; // not found
    }

    q->urdi.next = (void*)q; // mark this element as standalone
    return (intptr_t)q;

    ret_err:
    free(q);
    return (intptr_t)0;
}

/** Open a USB relay device
@return: This function returns a valid handle to the device on success or NULL on failure.
*/
intptr_t USBRL_API  usb_relay_device_open(struct usb_relay_device_info *device_info)
{
    struct usbrelay_internal_s *p = (struct usbrelay_internal_s *)device_info;
    if (!device_info)
        return 0;
    if ( (uintptr_t)p->usbh == 0 || (uintptr_t)p->usbh == (uintptr_t)-1 )
        return 0;
    //$$$ validate more
    return (uintptr_t)device_info;
}

/** Close a USB relay device*/
void USBRL_API usb_relay_device_close(intptr_t hHandle)
{
    struct usbrelay_internal_s *p = (struct usbrelay_internal_s *)hHandle;
    if ( 0 == hHandle || ((intptr_t)-1) == hHandle )
      return;

    if ( (void*)(p->urdi.next) == (void*)p ) {
        // This was made by usb_relay_device_open_with_serial_number() so free it now:
        if ( p->usbh && ((intptr_t)-1) != (intptr_t)(p->usbh)) {
                usbhidCloseDevice(p->usbh);
                p->usbh = 0;
        }
        p->urdi.next = NULL;
        free( (void*)p );
    }
    // Else this can be in the list, don't do anything.
}

/** Turn ON a relay channel on the USB-Relay-Device
@param index -- which channel your want to open
@param hHandle -- which usb relay device your want to operate
@returns: 0 -- success; 1 -- error; 2 -- index is outnumber the number of the usb relay device
*/
int USBRL_API usb_relay_device_open_one_relay_channel(intptr_t hHandle, int index)
{
    struct usbrelay_internal_s *p = (struct usbrelay_internal_s *)hHandle;
    if (!p)
        return 1;
    if ( index <= 0 || index > (int)p->urdi.type )
        return 2;
    return rel_onoff( p->usbh, 1, index);
}

/** Turn ON all relay channels on the USB-Relay-Device
@param hHandle -- which usb relay device your want to operate
@returns: 0 -- success; 1 -- error
*/
int USBRL_API usb_relay_device_open_all_relay_channel(intptr_t hHandle)
{
    struct usbrelay_internal_s *p = (struct usbrelay_internal_s *)hHandle;
    if (!p)
        return 1;
    return rel_onoff( p->usbh, 1, -(int)p->urdi.type );
}

/** Turn OFF a relay channel on the USB-Relay-Device
@param index -- which channel your want to close
@param hHandle -- which usb relay device your want to operate
@returns: 0 -- success; 1 -- error; 2 -- index is outnumber the number of the usb relay device
*/
int USBRL_API usb_relay_device_close_one_relay_channel(intptr_t hHandle, int index)
{
    struct usbrelay_internal_s *p = (struct usbrelay_internal_s *)hHandle;
    if (!p)
        return 1;
    if ( index <= 0 || index > (int)p->urdi.type )
        return 2;
    return rel_onoff( p->usbh, 0, index);
}

/** Turn OFF all relay channels on the USB-Relay-Device
@param hHandle -- which usb relay device your want to operate
@returns 0 -- success; 1 -- error
*/
int USBRL_API usb_relay_device_close_all_relay_channel(intptr_t hHandle)
{
    struct usbrelay_internal_s *p = (struct usbrelay_internal_s *)hHandle;
    if (!p)
        return 1;
    return rel_onoff( p->usbh, 0, -(int)p->urdi.type );
}

/** Get status of all relays on the device
Status bits:  one bit indicate a relay status.
bit 0/1/2/3/4/5/6/7/8 indicate channel 1/2/3/4/5/6/7/8 status
 1 -- means ON, 0 -- means OFF.
@returns: 0 -- success; 1 -- error
*/
int USBRL_API usb_relay_device_get_status(intptr_t hHandle, unsigned int *status)
{
    struct usbrelay_internal_s *p = (struct usbrelay_internal_s *)hHandle;
    int err;
    if (!p)
        return 1;
    err = rel_read_status_raw(p->usbh, NULL);
    if ( err < 0 ) {
        printerr("Error reading data: %s\n", usbErrorMessage(err));
        return -err;
    }

    *status = (unsigned char)err;
    return 0;
}

/************ Added ****************/

/** Get the library (dll) version
@return Lower 16 bits: the library version. Higher bits: undefined, ignore.
@note The original DLL does not have this function!
*/
int USBRL_API usb_relay_device_lib_version(void)
{
	return (int)(MY_VERSION);
}

/**
The following functions are for non-native callers, to avoid fumbling with C structs.
Native C/C++ callers do not need to use these.
The ptr_usb_relay_device_info arg is pointer to struct usb_relay_device_info, cast to intptr_t, void*, etc.
*/

/* Return next info struct pointer in the list returned by usb_relay_device_enumerate() */
intptr_t USBRL_API usb_relay_device_next_dev(intptr_t ptr_usb_relay_device_info)
{
    if (!ptr_usb_relay_device_info)
        return 0;
    return (intptr_t)(void*)((pusb_relay_device_info_t)ptr_usb_relay_device_info)->next;
}

/* Get number of relay channels on the device */
int USBRL_API usb_relay_device_get_num_relays(intptr_t ptr_usb_relay_device_info)
{
    if (!ptr_usb_relay_device_info)
        return 0;
    return (int)((pusb_relay_device_info_t)ptr_usb_relay_device_info)->type;
}

/* Get the ID string of the device. Returns pointer to const C string (1-byte, 0-terminated) */
intptr_t USBRL_API usb_relay_device_get_id_string(intptr_t ptr_usb_relay_device_info)
{
    if (!ptr_usb_relay_device_info)
        return 0;
    return (intptr_t)(void const *)((pusb_relay_device_info_t)ptr_usb_relay_device_info)->serial_number;
}

/* Get status of all relays on the device.
* @return Bitmask of all relay channels state, if the value > 0. Negative values mean error.
  bit 0/1/2/3/4/5/6/7/8 indicate channel 1/2/3/4/5/6/7/8 status
  Each bit value 1 means ON, 0 means OFF.
* @note This is same as usb_relay_device_get_status, but without dereferencing pointers.
*/
int USBRL_API usb_relay_device_get_status_bitmap(intptr_t hHandle)
{
    unsigned int st;
    int err = usb_relay_device_get_status(hHandle, &st);
    if (0 == err)
        return (int)st;
    return (err > 0) ? (-err) : err;
}

#ifdef __cplusplus
}
#endif
