/* Name: hiddata.h $$$$$$$$$$$$$$$$$$$$$$$$$
 * Author: Christian Starkjohann
 * Creation Date: 2008-04-11
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 */

#ifndef __HIDUSB_H_INCLUDED__
#define __HIDUSB_H_INCLUDED__

/*
This module implements an abstraction layer for data transfer over HID feature requests,
mainly for use with V-USB based HID custom devices which are not keyboards, mice etc.
The implementation uses native HID API on Windows, Apple OS X
or libusb on Linux etc.
 
Libraries to link are:
  on Windows XP+ : -lhid -lsetupapi
  on Linux or other platforms with libusb 0.1: `libusb-config --libs`
  on Mac OS 10.9+ : IOKit
*/

/* ------------------------------------------------------------------------ */

#define USBHID_OK              0   /* no error */
#define USBHID_ERR_ACCESS      1   /* access or permissions error */
#define USBHID_ERR_IO          2   /* I/O error */
#define USBHID_ERR_NOTFOUND    3   /* device not found */
#define USBHID_ERR_BAD_ARG     20  /* invalid parameter */
#define USBHID_ERR_INTERNAL    23  /* not implemented, not supported... */
#define USBHID_ERR_IO_USB      24  /* I/O error at usb layer  */
#define USBHID_ERR_IO_HID      25  /* I/O error at hid layer */
#define USBHID_ERR_UNKNOWN     -1  /* whatever */


/* ------------------------------------------------------------------------ */

/* USBDEVHANDLE: Opaque handle representing the USB HID device.
 */
struct usbDevice;
typedef struct usbDevice    usbDevice_t;
typedef usbDevice_t         *USBDEVHANDLE;


/* ------------------------------------------------------------------------ */

/* @func usbhidEnumDevices
 * Enumerates USB HID devices, filtered by USB vendorID and productID.
 * Each found device is opened and the callback will be called.
 * The callback can probe the device and close it or keep open.
 * If the callback returns 0, enumeration stops, else it goes on.
 * @param usbhidEnumFunc: callback function
 * @param context: context pointer for the callback
 * @return 0 if some devices were found or error code otherwise.
 */
int usbhidEnumDevices(int vendorID, int productID, void *context,
                      int (*usbhidEnumFunc)(USBDEVHANDLE usbh, void *ctx));

/* 
 * Close USBDEVHANDLE passed by usbhidEnumDevices
 */
void usbhidCloseDevice(USBDEVHANDLE usbh);

/*
 * Read HID vendor and product strings as ASCII null terminated strings
 * Returns 0 on success, error code if error occured or the buffer is too short
 * Any non-ascii characters in the strings will be replaced to '?'
 */
int usbhidGetVendorString(USBDEVHANDLE usbh, char *buffer, int len);
int usbhidGetProductString(USBDEVHANDLE usbh, char *buffer, int len);

/* Send a feature report to the device. The report ID must be
 * in the first byte of buffer and the length 'len' of the report buffer
 * includes this report ID. If the device does not specify report IDs,
 *  buffer[0] must be set to 0.
 * @return: 0 on success, an error code otherwise.
 */
int usbhidSetReport(USBDEVHANDLE usbh, char *buffer, int len);

/* Read a feature report from the device.
 * The requested report ID is passed in 'reportID' parameter and returned
 * in the 1st byte of the data.
 * The caller must pass a buffer of the size of the expected report in 'buffer'
 * including the report id byte, and initialize the variable pointed to by
 * 'len' to the total size of this buffer. Upon successful return, the report
 * (prefixed with the report-ID) is in 'buffer' and the actual length of the
 * report is returned in '*len'.
 * Returns: 0 on success, an error code otherwise.
 */
int usbhidGetReport(USBDEVHANDLE usbh, int reportID, char *buffer, int *len);


/**
 * Return description of error code
 * TODO? return error descr. of underlying libusb or hid layer too?
 */
int usbhidStrerror_r(int err, char *buf, int len);

/* ------------------------------------------------------------------------ */

#endif /* __HIDUSB_H_INCLUDED__ */
