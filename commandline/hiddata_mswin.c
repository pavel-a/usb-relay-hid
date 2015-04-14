/* hiddata_mswin.c
 * Win32 variant
 */

/* Inspired by hiddata.c|h by Christian Starkjohann
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 */

#if !defined(_WIN32)
#error
#endif

#include "hiddata.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

#if 0 //ifdef DEBUG
#define DEBUG_PRINT(arg)    printf arg
#else
#define DEBUG_PRINT(arg)
#endif

#include "targetver.h"
#include <windows.h>
#include <setupapi.h>
#include <hidsdi.h>
#include <hidpi.h>

#ifdef _MSC_VER
#pragma comment(lib, "setupapi")
#pragma comment(lib, "hid")
#if _MSC_VER < 1900 /* before VS2015 */
#define snprintf   _snprintf
#endif /* VS2015 */
#else /* GCC, Mingw... */
#endif /*_MSC_VER*/

/*
 * Convert UTF-16 null term. string to single byte (ASCII or ISO Latin)
 * change all weird characters to "?"
 */
static void usbstring_to_ascii(unsigned short *wp, char *cp, int size)
{
    unsigned short *wpend = wp + (size/sizeof(unsigned short));
    for ( ; wp < wpend; )
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
    int                                 i;
    int openFlags = 0;
    int                                 errorCode = USBHID_ERR_NOTFOUND;
    HANDLE                              handle = INVALID_HANDLE_VALUE;
    HIDD_ATTRIBUTES                     deviceAttributes;
    BOOL b;

    HidD_GetHidGuid(&hidGuid);
    deviceInfoList = SetupDiGetClassDevsW(&hidGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
    if (!deviceInfoList || deviceInfoList == INVALID_HANDLE_VALUE)
    {
        return USBHID_ERR_NOTFOUND;
    }

    deviceInfo.cbSize = sizeof(deviceInfo);
    for (i=0; ; i++) {
        if (handle != INVALID_HANDLE_VALUE) {
            CloseHandle(handle);
            handle = INVALID_HANDLE_VALUE;
        }
        if ( !SetupDiEnumDeviceInterfaces(deviceInfoList, 0, &hidGuid, i, &deviceInfo) )
            break;  /* no more entries */
        /* First do a dummy call just to determine the actual size required */
        size = 0;
        SetupDiGetDeviceInterfaceDetailW(deviceInfoList, &deviceInfo, NULL, 0, &size, NULL);
        if ( size == 0 )
            continue;
        if (deviceDetails != NULL)
            free(deviceDetails);
        deviceDetails = malloc(size);
        if ( !deviceDetails ) {
            DEBUG_PRINT(("ALLOC ERROR!\n"));
            continue;
        }
        deviceDetails->cbSize = sizeof(*deviceDetails);
        /* 2nd call */
        b = SetupDiGetDeviceInterfaceDetailW(deviceInfoList, &deviceInfo, deviceDetails, size, &size, NULL);
        if ( !b )
            continue;
        DEBUG_PRINT(("checking HID path \"%s\"\n", deviceDetails->DevicePath));

        handle = CreateFileW(deviceDetails->DevicePath, 
            GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, openFlags, NULL);
        if (handle == INVALID_HANDLE_VALUE){
            DEBUG_PRINT(("open USB device failed: gle=%d\n", (int)GetLastError()));
            /* Opening devices owned by OS or other apps will fail ; just ignore these. */
            continue;
        }
        deviceAttributes.Size = sizeof(deviceAttributes);
        HidD_GetAttributes(handle, &deviceAttributes);
        DEBUG_PRINT(("device attributes: vid=%d pid=%d ver=%4.4X\n", deviceAttributes.VendorID, deviceAttributes.ProductID, deviceAttributes.VersionNumber));
        if (deviceAttributes.VendorID != vendor || deviceAttributes.ProductID != product)
            continue;   /* skip this device */

        errorCode = 0;
        if ( 0 == usbhidEnumFunc((USBDEVHANDLE)handle, context) )
        {
            break; /* stop enumeration */
        }

        /* Now the handle is owned by the callback It may close it before return or later. */
        handle = INVALID_HANDLE_VALUE;
    }

    SetupDiDestroyDeviceInfoList(deviceInfoList);
    if (deviceDetails != NULL)
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
    buffer[0] = (char)reportNumber;
    rval = HidD_GetFeature((HANDLE)usbh, buffer, *len);
    return rval == 0 ? USBHID_ERR_IO_HID : 0;
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
