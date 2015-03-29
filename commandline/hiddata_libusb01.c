/* hiddata_libusb01.c
 * Variant for libusb v 0.1 (old simple version, found in many PC Linux distros)
 */

/* Inspired by hiddata.c|h by Christian Starkjohann
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 */


#include "hiddata.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <usb.h>

#if 0 //ifdef DEBUG
#define DEBUG_PRINT(fmt, ...)    do {} while(0)
#else
#define DEBUG_PRINT(fmt, ...) fprintf(stderr, fmt, ## __VA_ARGS__)
#endif


// USBDEVHANDLE is same as struct usb_device *
// Open handles are stored in struct usb_device (HACK! revise for new libusb)

static inline struct usb_device * usbDevStruct(USBDEVHANDLE usbh) {
    return (struct usb_device *)usbh;
}

static inline usb_dev_handle * usbDevHandle(USBDEVHANDLE usbh) {
    return (usb_dev_handle *)(usbDevStruct(usbh)->children);
}

// TO DO: for devices that prepend report ID, implement this somehow
static inline int usesReportIDs(USBDEVHANDLE usbh) {
  /* return 1 if 1st byte of get/set report buffer is report ID */
   return 0;
}

/* ------------------------------------------------------------------------- */

#define USBRQ_HID_GET_REPORT    0x01
#define USBRQ_HID_SET_REPORT    0x09
#define USB_HID_REPORT_TYPE_FEATURE 3

#define A_REPORT_REQUEST_TIMEOUT 5000 /* in milliseconds */

/* ------------------------------------------------------------------------- */

int usbhidEnumDevices(int vendor, int product,
                      void *context,
                      int (*usbhidEnumFunc)(USBDEVHANDLE usbh, void *ctx))

{
    struct usb_bus      *bus;
    struct usb_device   *dev;
    usb_dev_handle      *handle = NULL;
    int                 errorCode = USBHID_ERR_NOTFOUND;
    static int          didUsbInit = 0;

    if(!didUsbInit){
        usb_init();
        didUsbInit = 1;
    }
    usb_find_busses();
    usb_find_devices();
    for (bus=usb_get_busses(); bus; bus=bus->next) {
        for (dev=bus->devices; dev; dev=dev->next) {
            if (dev->descriptor.idVendor == vendor && dev->descriptor.idProduct == product) {
                handle = usb_open(dev); /* we need to open the device in order to query strings */
                if ( !handle ) {
                    errorCode = USBHID_ERR_ACCESS;
                    DEBUG_PRINT("Warning: cannot open USB device: %s\n", usb_strerror());
                    continue;
                }

                errorCode = 0;

//  printf("Probing: [%s] nc=%u %p\n", dev->filename, dev->num_children, dev->children);
                // Assume our devices are leaf, so we can use dev->children to store the handle
                if ( dev->num_children != 0 ) {
                    DEBUG_PRINT("ERROR: assertion failed for usb dev %p\n", dev);
                    usb_close(handle);
                    handle = NULL;
                    continue;
                }

                dev->children = (void*)handle;
                if ( 0 == usbhidEnumFunc((void*)dev, context) )
                {
                    break; /* stop enumeration */
                }

                /* Now the handle is owned by the callback */
                handle = 0;
            }
        }
    }

    return errorCode;
}

/* ------------------------------------------------------------------------- */

void usbhidCloseDevice(USBDEVHANDLE usbh)
{
    if (usbh != NULL) {
        usb_close(usbDevHandle(usbh));
        usbDevStruct(usbh)->children = NULL;
    }
}

/* ------------------------------------------------------------------------- */

static int usbhidGetStringAscii(struct usb_dev_handle *dev, int index, char *buf, int buflen)
{
    int     rval;
    if((rval = usb_get_string_simple(dev, index, buf, buflen)) >= 0) /* use libusb version if it works */
        return rval;
    if (errno == EPERM)
      DEBUG_PRINT("usbhid: Access denied to USB device. Run as root or adjust device permissions.\n");
    else
      DEBUG_PRINT("usbhid: %s error %s\n", __FUNCTION__, usb_strerror());
    return -1;
}

int usbhidGetVendorString(USBDEVHANDLE usbh, char *buffer, int len)
{
    int len2 = usbhidGetStringAscii(usbDevHandle(usbh), usbDevStruct(usbh)->descriptor.iManufacturer, buffer, len);
    if (len2 < 0) {
        DEBUG_PRINT("Warning: cannot query vendor for device\n");
        return USBHID_ERR_IO;
    }
    return 0;
}

int usbhidGetProductString(USBDEVHANDLE usbh, char *buffer, int len)
{
    int len2 = usbhidGetStringAscii(usbDevHandle(usbh), usbDevStruct(usbh)->descriptor.iProduct, buffer, len);
    if (len2 < 0) {
        DEBUG_PRINT("Warning: cannot query product for device\n");
        return USBHID_ERR_IO;
    }
    return 0;
}




int usbhidSetReport(USBDEVHANDLE usbh, char *buffer, int len)
{
    int bytesSent, reportId = buffer[0];

    if ( !usesReportIDs(usbh) ) {
        buffer++;   /* skip dummy report ID */
        len--;
    }
    bytesSent = usb_control_msg(usbDevHandle(usbh),
            USB_TYPE_CLASS | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, USBRQ_HID_SET_REPORT, USB_HID_REPORT_TYPE_FEATURE << 8 | (reportId & 0xff),
            0, buffer, len, A_REPORT_REQUEST_TIMEOUT);
    if (bytesSent != len) {
        if (bytesSent < 0)
            DEBUG_PRINT("Error sending message: %s\n", usb_strerror());
        return USBHID_ERR_IO;
    }
    return 0;
}

/* ------------------------------------------------------------------------- */

int usbhidGetReport(USBDEVHANDLE usbh, int reportNumber, char *buffer, int *len)
{
int bytesReceived, maxLen = *len;

    if (!usesReportIDs(usbh)) {
        buffer++;   /* make room for dummy report ID */
        maxLen--;
    }
    bytesReceived = usb_control_msg(usbDevHandle(usbh),
            USB_TYPE_CLASS | USB_RECIP_DEVICE | USB_ENDPOINT_IN, USBRQ_HID_GET_REPORT, USB_HID_REPORT_TYPE_FEATURE << 8 | reportNumber,
            0, buffer, maxLen, A_REPORT_REQUEST_TIMEOUT);
    if(bytesReceived < 0){
        DEBUG_PRINT("Error sending message: %s\n", usb_strerror());
        return USBHID_ERR_IO;
    }
    *len = bytesReceived;
    if (!usesReportIDs(usbh)) {
        buffer[-1] = reportNumber;  /* add dummy report ID */
        (*len)++;
    }
    return 0;
}


void usbhidSetUsesReportId(USBDEVHANDLE usbh)
{
    //TODO Implement if some devices prepend report IDs
	//This differs among several versions of Linux HID libraries...
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
