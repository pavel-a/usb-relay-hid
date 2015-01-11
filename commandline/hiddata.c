/* Name: hiddata.c
 * Author: Christian Starkjohann
 * Creation Date: 2008-04-11
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 */

#include "hiddata.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

#if 0 //ifdef DEBUG
#define DEBUG_PRINT(arg)    printf arg
#else
#define DEBUG_PRINT(arg)
#endif

#define A_MAX_USB_STRING_LEN 126


/* ######################################################################## */
#if defined(_WIN32)
/* ######################################################################## */

#include "targetver.h"
#include <windows.h>
#include <setupapi.h>
#include <hidsdi.h>
#include <hidpi.h>

#ifdef _MSC_VER
#pragma comment(lib, "setupapi")
#pragma comment(lib, "hid")

#define snprintf   _snprintf
#endif /*_MSC_VER*/

/*
 * Convert UTF-16 null term. string to single byte (ASCII or ISO Latin)
 * change all weird characters to "?"
 */
static void usbstring_to_ascii(unsigned short *wp, char *cp, int size)
{
    unsigned short *wpend = wp + (size/sizeof(unsigned short));
    for( ; wp < wpend; )
    {
        unsigned short h = *wp++;
        *cp++ = (h < 0xFF) ? (char)h : '?';
        if (h == 0)
            break;
    }
}

/*
 * Read HID string for vendor and device, return as ASCII (or ISO Latin...)
 */
int usbhidGetVendorString(USBDEVHANDLE usbh, char *buffer, int len)
{
    /* HidD_GetManufacturerString returns zero terminated UTF-16 string */
    /* Error if buffer is too short */
    if ( !HidD_GetManufacturerString((HANDLE)usbh, (void*)buffer, len ) ) {
        DEBUG_PRINT(("error obtaining vendor name\n"));
        return USBHID_ERR_IO_HID;
    }
    usbstring_to_ascii((UINT16*)buffer, buffer, len);
    return 0;
}

int usbhidGetProductString(USBDEVHANDLE usbh, char *buffer, int len)
{
    /* HidD_GetProductString returns zero terminated UTF-16 string */
    /* Error if buffer is too short */
    if (!HidD_GetProductString((HANDLE)usbh, (void*)buffer, len ) ) {
        DEBUG_PRINT(("error obtaining product name\n"));
        return USBHID_ERR_IO_HID;
    }
    usbstring_to_ascii((UINT16*)buffer, buffer, len);
    return 0;
}

/*
 * Enumerate HID USB devices.
 * In Windows this will find also non-USB devices, but assume that
 * filtering by PID & VID is enough.
 * Some HID devices (mice, kbd) are locked by Windows and cannot be opened.
 * If we cannot open a device for R&W, we skip it without error.
 * Assume our devices are not of types reserved by Windows.
 */
int usbhidEnumDevices(int vendor, int product,
                      void *context,
                      int (*usbhidEnumFunc)(USBDEVHANDLE usbh, void *ctx))
{
    GUID                                hidGuid;        /* GUID for HID class */
    HDEVINFO                            deviceInfoList;
    SP_DEVICE_INTERFACE_DATA            deviceInfo;
    SP_DEVICE_INTERFACE_DETAIL_DATA_W   *deviceDetails = NULL;
    DWORD                               size;
    int                                 i, openFlag = 0;  /* may be FILE_FLAG_OVERLAPPED */
    int                                 errorCode = USBHID_ERR_NOTFOUND;
    HANDLE                              handle = INVALID_HANDLE_VALUE;
    HIDD_ATTRIBUTES                     deviceAttributes;
                
    HidD_GetHidGuid(&hidGuid);
    deviceInfoList = SetupDiGetClassDevsW(&hidGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
    if (!deviceInfoList || deviceInfoList == INVALID_HANDLE_VALUE)
    {
        return USBHID_ERR_NOTFOUND;
    }

    deviceInfo.cbSize = sizeof(deviceInfo);
    for (i=0; ; i++) {
        if(handle != INVALID_HANDLE_VALUE){
            CloseHandle(handle);
            handle = INVALID_HANDLE_VALUE;
        }
        if( !SetupDiEnumDeviceInterfaces(deviceInfoList, 0, &hidGuid, i, &deviceInfo) )
            break;  /* no more entries */
        /* first do a dummy call just to determine the actual size required */
        SetupDiGetDeviceInterfaceDetailW(deviceInfoList, &deviceInfo, NULL, 0, &size, NULL);
        if(deviceDetails != NULL)
            free(deviceDetails);
        deviceDetails = malloc(size);
        deviceDetails->cbSize = sizeof(*deviceDetails);
        /* this call is for real: */
        SetupDiGetDeviceInterfaceDetailW(deviceInfoList, &deviceInfo, deviceDetails, size, &size, NULL);
        DEBUG_PRINT(("checking HID path \"%s\"\n", deviceDetails->DevicePath));

        handle = CreateFileW(deviceDetails->DevicePath, 
            GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, openFlag, NULL);
        if(handle == INVALID_HANDLE_VALUE){
            DEBUG_PRINT(("open USB device failed: gle=%d\n", (int)GetLastError()));
            /* errorCode = USBOPEN_ERR_ACCESS; opening will always fail for mouse -- ignore */
            continue;
        }
        deviceAttributes.Size = sizeof(deviceAttributes);
        HidD_GetAttributes(handle, &deviceAttributes);
        DEBUG_PRINT(("device attributes: vid=%d pid=%d ver=%4.4X\n", deviceAttributes.VendorID, deviceAttributes.ProductID, deviceAttributes.VersionNumber));
        if(deviceAttributes.VendorID != vendor || deviceAttributes.ProductID != product)
            continue;   /* skip this device */

        errorCode = 0;
        if ( 0 == usbhidEnumFunc((USBDEVHANDLE)handle, context) )
        {
            break; /* stop enumeration */
        }

        /* Now the handle is owned by the callback */
        handle = INVALID_HANDLE_VALUE;
    }

    SetupDiDestroyDeviceInfoList(deviceInfoList);
    if(deviceDetails != NULL)
        free(deviceDetails);

    return errorCode;
}


void usbhidCloseDevice(USBDEVHANDLE usbh)
{
    CloseHandle((HANDLE)usbh);
}


int usbhidSetReport(USBDEVHANDLE usbh, char *buffer, int len)
{
    BOOLEAN rval;
    rval = HidD_SetFeature((HANDLE)usbh, buffer, len);
    return rval == 0 ? USBHID_ERR_IO_HID : 0;
}


int usbhidGetReport(USBDEVHANDLE usbh, int reportNumber, char *buffer, int *len)
{
    BOOLEAN rval = 0;
    buffer[0] = reportNumber;
    rval = HidD_GetFeature((HANDLE)usbh, buffer, *len);
    return rval == 0 ? USBHID_ERR_IO_HID : 0;
}



/* ######################################################################## */
#else /* defined WIN32 #################################################### */
/* ######################################################################## */

// Using the old simple version of libusb (before 1.0)
#include <usb.h>

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
    int                 errorCode = USBOPEN_ERR_NOTFOUND;
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
                    errorCode = USBOPEN_ERR_ACCESS;
                    fprintf(stderr, "Warning: cannot open USB device: %s\n", usb_strerror());
                    continue;
                }

                errorCode = 0;

//  printf("Probing: [%s] nc=%u %p\n", dev->filename, dev->num_children, dev->children);
                // Assume our devices are leaf, so we can use dev->children to store the handle
                if ( dev->children ) {
                    fprintf(stderr, "ERROR: assertion failed for usb dev %p\n", dev);
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
      fprintf(stderr, "usbhid: Access denied to USB device. Run as root or adjust device permissions.\n");
    else
      fprintf(stderr, "usbhid: %s error %s\n", __FUNCTION__, usb_strerror());
    return -1;
}

int usbhidGetVendorString(USBDEVHANDLE usbh, char *buffer, int len)
{
    int len2 = usbhidGetStringAscii(usbDevHandle(usbh), usbDevStruct(usbh)->descriptor.iManufacturer, buffer, len);
    if (len2 < 0) {
        fprintf(stderr, "Warning: cannot query vendor for device\n");
        return USBOPEN_ERR_IO;
    }
    return 0;
}

int usbhidGetProductString(USBDEVHANDLE usbh, char *buffer, int len)
{
    int len2 = usbhidGetStringAscii(usbDevHandle(usbh), usbDevStruct(usbh)->descriptor.iProduct, buffer, len);
    if (len2 < 0) {
        fprintf(stderr, "Warning: cannot query product for device\n");
        return USBOPEN_ERR_IO;
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
            fprintf(stderr, "Error sending message: %s\n", usb_strerror());
        return USBOPEN_ERR_IO;
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
        fprintf(stderr, "Error sending message: %s\n", usb_strerror());
        return USBOPEN_ERR_IO;
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
}

/* ######################################################################## */
#endif /* WIN32 */
/* ######################################################################## */

int usbhidStrerror_r( int err, char *buf, int len)
{
    const char *s;
    switch (err) {
        case USBHID_ERR_ACCESS:      s = "Access to device denied";
        case USBHID_ERR_NOTFOUND:    s = "The specified device was not found";
        case USBHID_ERR_IO:          s = "Communication error with device";
        case USBHID_ERR_IO_HID:      s = "HID I/O error with device";
        default:
            s = "";
    }
  
    return snprintf(buf, len, "%s", s);
}
