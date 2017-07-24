/**
 * File: hiddata-OSX.c
 * Version for OS/X 10.9+
 * 05-jan-2014 pa
 * License: ?
 */

#include "hiddata.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

// OS X: Using IOKit HID functionality
#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDDevice.h>
#include <IOKit/hid/IOHIDManager.h>


#if 0 //ifdef DEBUG
#define DEBUG_PRINT(arg)    printf arg
#else
#define DEBUG_PRINT(arg)
#endif

static IOHIDManagerRef g_hidmgr = NULL;
static int g_hidmgr_open_count = 0;

// Device handle is IOHIDDeviceRef
static inline IOHIDDeviceRef usbh_to_IOHIDDeviceRef(USBDEVHANDLE usbh)
{
    return (IOHIDDeviceRef)(void*)usbh;
}


static void _hidmgrDeref(void)
{
    if ( g_hidmgr && (--g_hidmgr_open_count <= 0) ) {
        IOHIDManagerClose(g_hidmgr, kIOHIDOptionsTypeNone);
        CFRelease(g_hidmgr);
        g_hidmgr = NULL;
    }
}


static int32_t get_int_property(IOHIDDeviceRef device, CFStringRef key)
{
    int32_t value = -1;
    
    CFTypeRef ref = IOHIDDeviceGetProperty(device, key);
    if (ref) {
        if (CFGetTypeID(ref) == CFNumberGetTypeID()) {
            value = 0;
            CFNumberGetValue((CFNumberRef) ref, kCFNumberSInt32Type, &value);
        }
    }
    return value;
}

static int get_string_property_utf8(IOHIDDeviceRef device, CFStringRef prop, char *buf, ssize_t len)
{
    *buf = 0;
    if (len < 2)
        return 0;
    CFStringRef str = IOHIDDeviceGetProperty(device, prop);
    if (!str)
        return USBHID_ERR_BAD_ARG;
    
    CFIndex str_len = CFStringGetLength(str);
    CFIndex chars_copied = 0;
    if (str_len) {
        CFIndex used_buf_len;
        len--;
        chars_copied = CFStringGetBytes(str,
                                        CFRangeMake(0, CFStringGetLength(str)),
                                        kCFStringEncodingUTF8,
                                        '?',
                                        FALSE,
                                        (void*)buf,
                                        len,
                                        &used_buf_len);
        buf[used_buf_len] = 0;
    }
    return (int)chars_copied;
}


// Enumerate devices
int usbhidEnumDevices(int vendor, int product,
                      void *context,
                      int (*usbhidEnumFunc)(USBDEVHANDLE usbh, void *ctx))

{
    /* Enumerate thru HID devices */
    IOHIDManagerRef mgr;
    mgr = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    if (!mgr)
        return USBHID_ERR_IO_HID;
    // *** TODO: match by vendor/product right here ***
    IOHIDManagerSetDeviceMatching(mgr, NULL);
    IOReturn ir = IOHIDManagerOpen(mgr, kIOHIDOptionsTypeNone);
    if (ir != kIOReturnSuccess)
        return USBHID_ERR_IO_HID;
    
    CFSetRef device_set = IOHIDManagerCopyDevices(mgr);
    CFIndex num_devices = CFSetGetCount(device_set);
    if (0 == num_devices)
        return USBHID_ERR_NOTFOUND;
    
    g_hidmgr = mgr;
    g_hidmgr_open_count = 1;
    
    IOHIDDeviceRef *device_array = calloc(num_devices, sizeof(IOHIDDeviceRef));
    CFSetGetValues(device_set, (const void **) device_array);
    
    int i;
    for (i = 0; i < num_devices; i++) {
        IOHIDDeviceRef dev = device_array[i];
        char buf[32];
        uint16_t nvid, npid;
        
        if (!dev)
            continue; /* device gone during enumeration? */
        
        /* Skip non-USB devices */
        get_string_property_utf8(dev, CFSTR(kIOHIDTransportKey), buf, sizeof(buf));
        if (0 != strcasecmp(buf, "USB"))
            continue;
        
        nvid = (uint16_t)get_int_property(dev, CFSTR(kIOHIDVendorIDKey));
        npid = (uint16_t)get_int_property(dev, CFSTR(kIOHIDProductIDKey));

        if (nvid != (uint16_t)vendor || npid != (uint16_t)product) {
            continue;
        }
        //printf("Device: %p\n", dev);

        /* the device handle is owned by the callback, it shoud call usbhidCloseDevice() on it */
        /* No need to open the device only for control pipe io (get/set HID feature) */
        ++g_hidmgr_open_count;
        if ( 0 == usbhidEnumFunc((USBDEVHANDLE)dev, context) )
        {
            break; /* stop enumeration */
        }

    }

    free(device_array);
    _hidmgrDeref();
    return 0;
}

/* ------------------------------------------------------------------------- */

void usbhidCloseDevice(USBDEVHANDLE usbh)
{
    IOHIDDeviceRef dev = usbh_to_IOHIDDeviceRef(usbh);
    //printf("close %p\n", dev);
    if (dev) {
        _hidmgrDeref();
    }
}

/* ------------------------------------------------------------------------- */

int usbhidGetVendorString(USBDEVHANDLE usbh, char *buffer, int len)
{
    IOHIDDeviceRef dev = usbh_to_IOHIDDeviceRef(usbh);
    int len2 = get_string_property_utf8(dev, CFSTR(kIOHIDManufacturerKey), buffer, len);
    if (len2 < 0) {
        fprintf(stderr, "Warning: cannot query vendor for device\n");
        return USBHID_ERR_IO_HID;
    }
    return 0;
}

int usbhidGetProductString(USBDEVHANDLE usbh, char *buffer, int len)
{
    IOHIDDeviceRef dev = usbh_to_IOHIDDeviceRef(usbh);
    int len2 = get_string_property_utf8(dev, CFSTR(kIOHIDProductKey), buffer, len);
    if (len2 < 0) {
        fprintf(stderr, "Warning: cannot query product for device\n");
        return USBHID_ERR_IO_HID;
    }
    return 0;
}


static inline bool usesReportIDs(USBDEVHANDLE usbh) {
    /* true => the device (or underlying HID layer) returns the report ID
     in the 1st byte of Get report buffer, and expects the id in Set request.
     The v-usb relay devices do not do this.
     */
    return false;
}


int usbhidSetReport(USBDEVHANDLE usbh, char *buffer, int len)
{
    IOHIDDeviceRef dev = usbh_to_IOHIDDeviceRef(usbh);
    char reportNumber = buffer[0];

    if ( !usesReportIDs(usbh) ) {
        buffer++;   /* skip report ID */
        len--;
    }
    
    IOReturn
    res = IOHIDDeviceSetReport(dev, kIOHIDReportTypeFeature, (CFIndex)reportNumber, (void*)buffer, (CFIndex)len);
     if (res != kIOReturnSuccess) {
        fprintf(stderr, "Error sending message: %d\n", (int)res);
        return USBHID_ERR_IO_HID;
    }
    return 0;
}

/* ------------------------------------------------------------------------- */

int usbhidGetReport(USBDEVHANDLE usbh, int reportNumber, char *buffer, int *len)
{
    IOHIDDeviceRef dev = usbh_to_IOHIDDeviceRef(usbh);
    int maxLen = *len;
    if (maxLen < 2)
        return USBHID_ERR_BAD_ARG;

    if (!usesReportIDs(usbh)) {
        buffer++;   /* make room for dummy report ID */
        maxLen--;
    }
    
    CFIndex rb = maxLen;
    IOReturn
    res = IOHIDDeviceGetReport(dev, kIOHIDReportTypeFeature, (CFIndex)reportNumber, (void*)buffer, &rb);

    if (res != kIOReturnSuccess || ((unsigned)rb <= 0)) {
        fprintf(stderr, "Error get report: %d\n", res);
        return USBHID_ERR_IO_HID;
    }
    
    *len = (int)rb;

    if (!usesReportIDs(usbh)) {
        buffer[-1] = (char)reportNumber;  /* add dummy report ID */
        (*len)++;
    }
    return 0;
}


void usbhidSetUsesReportId(USBDEVHANDLE usbh)
{
    //TODO Implement if some devices prepend report IDs
}


int usbhidStrerror_r( int err, char *buf, int len)
{
    const char *s;
    switch (err) {
        case USBHID_ERR_ACCESS:      s = "Access to device denied";
            break;
        case USBHID_ERR_NOTFOUND:    s = "The specified device was not found";
            break;
        case USBHID_ERR_IO:          s = "Communication error with device";
            break;
        case USBHID_ERR_IO_HID:      s = "HID I/O error with device";
            break;
        default:
            s = "";
    }

    return snprintf(buf, len, "%s", s);
}
