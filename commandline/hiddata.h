/* Name: hiddata.h $$$$$$$$$$$$$$$$$$$$$$$$$
 * Author: Christian Starkjohann
 * Creation Date: 2008-04-11
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 */

#ifndef __HIDDATA_H_INCLUDED__
#define __HIDDATA_H_INCLUDED__

/*
General Description:
This module implements an abstraction layer for data transfer over HID feature
requests. The implementation uses native Windows functions on Windows so that
no driver installation is required and libusb on Unix. You must link the
appropriate libraries in either case: "-lhid -lusb -lsetupapi" on Windows and
`libusb-config --libs` on Unix.
*/

/* ------------------------------------------------------------------------ */

#define USBOPEN_SUCCESS         0   /* no error */
#define USBOPEN_ERR_ACCESS      1   /* not enough permissions to open device */
#define USBOPEN_ERR_IO          2   /* I/O error */
#define USBOPEN_ERR_NOTFOUND    3   /* device not found */

/* ------------------------------------------------------------------------ */

struct usbDevice;
typedef struct usbDevice    usbDevice_t;
typedef usbDevice_t        *USBDEVHANDLE;

/* Opaque data type representing the USB device. This can be a Windows handle
 * or a libusb handle, depending on the backend implementation.
 */

/* ------------------------------------------------------------------------ */

/* @func usbhidEnumDevices
 * Enumerates USB HID devices, filtered by USB vendorID and productID.
 * Each found device is opened and the callback will be called.
 * The callback can probe the device and close it or keep open.
 * If the callback returns 0, enumeration stops, else it goes on.
 * @returns 0 if some devices were found or error code otherwise.
 */
int usbhidEnumDevices(int vendorID, int productID, void *context,
					  int (*usbhidEnumFunc)(USBDEVHANDLE usbh, void *ctx));

/* 
 * Close USB device handle opened with usbhidEnumDevices
 */
void usbhidCloseDevice(USBDEVHANDLE usbh);

/*
 * Read HID vendor and product strings as ASCII null terminated strings
 * Returns 0 on success, error code if error occured or the buffer is too short
 * Any non-ascii characters in the strings will be replaced to '?'
 */
int usbhidGetVendorString(USBDEVHANDLE usbh, char *buffer, int len);
int usbhidGetProductString(USBDEVHANDLE usbh, char *buffer, int len);

int usbhidSetReport(usbDevice_t *device, char *buffer, int len);
/* This function sends a feature report to the device. The report ID must be
 * in the first byte of buffer and the length 'len' of the report is specified
 * including this report ID. If no report IDs are used, buffer[0] must be set
 * to 0 (dummy report ID).
 * Returns: 0 on success, an error code otherwise.
 */

int usbhidGetReport(usbDevice_t *device, int reportID, char *buffer, int *len);
/* This function obtains a feature report from the device. The requested
 * report-ID is passed in 'reportID'. The caller must pass a buffer of the size
 * of the expected report in 'buffer' and initialize the variable pointed to by
 * 'len' to the total size of this buffer. Upon successful return, the report
 * (prefixed with the report-ID) is in 'buffer' and the actual length of the
 * report is returned in '*len'.
 * Returns: 0 on success, an error code otherwise.
 */

/* ------------------------------------------------------------------------ */

#endif /* __HIDDATA_H_INCLUDED__ */
