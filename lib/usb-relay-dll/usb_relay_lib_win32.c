// Windows version

#include "stdafx.h"

#define USBRL_API __declspec(dllexport)
#include "../usb_relay_device.h"
#include "../usb_relay_hw.h"

#include <stdio.h>

#ifdef _MSC_VER
#pragma comment(lib, "setupapi")
#pragma comment(lib, "hid")
#endif /*_MSC_VER*/

#undef ERROR
#define ERROR (-1)
#define OK (0)

#define dbgprintf(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)
//#define dbgprintf(fmt, ...) printf(fmt, __VA_ARGS__)
//#define dbgprintf(fmt, ...) __noop(fmt, __VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

// Globals


// Public functions:

/** Initialize the USB Relay Library
@returns: This function returns 0 on success and -1 on error.
*/
int USBRL_API usb_relay_init(void)
{
	dbgprintf("Init lib\n");
    return ERROR;
}

/** Finalize the USB Relay Library.
This function frees all of the static data associated with USB Relay Library. 
It should be called at the end of execution to avoid memory leaks.
@returns: This function returns 0 on success and -1 on error.
*/
int USBRL_API usb_relay_exit(void)
{
    return OK;
}

/** Enumerate the USB Relay Devices.*/
struct usb_relay_device_info USBRL_API * usb_relay_device_enumerate(void)
{
    return NULL;
}


/** Free an enumeration Linked List*/
void USBRL_API usb_relay_device_free_enumerate(struct usb_relay_device_info *di)
{
    return;
}

/** Open device by serial number
@return: This function returns a valid handle to the device on success or NULL on failure.
Example: usb_relay_device_open_with_serial_number("abcde", 5)  */
intptr_t USBRL_API usb_relay_device_open_with_serial_number(const char *serial_number, unsigned len)
{
    return ERROR;
}

/* Open a USB relay device
@return: This function returns a valid handle to the device on success or NULL on failure.
*/
intptr_t USBRL_API  usb_relay_device_open(struct usb_relay_device_info *device_info)
{
    return ERROR;
}

/* Close a USB relay device*/
void USBRL_API usb_relay_device_close(intptr_t hHandle)
{
    return;
}

/** Turn ON a relay channel on the USB-Relay-Device
@param index -- which channel your want to open
@param hHandle -- which usb relay device your want to operate
@returns: 0 -- success; 1 -- error; 2 -- index is outnumber the number of the usb relay device
*/
int USBRL_API usb_relay_device_open_one_relay_channel(intptr_t hHandle, int index)
{
    return ERROR;
}

/** Turn ON all relay channels on the USB-Relay-Device
@param hHandle -- which usb relay device your want to operate
@returns: 0 -- success; 1 -- error
*/
int USBRL_API usb_relay_device_open_all_relay_channel(intptr_t hHandle)
{
    return ERROR;
}

/** Turn OFF a relay channel on the USB-Relay-Device
@param index -- which channel your want to close
@paramhHandle -- which usb relay device your want to operate
@returns: 0 -- success; 1 -- error; 2 -- index is outnumber the number of the usb relay device
*/
int USBRL_API usb_relay_device_close_one_relay_channel(intptr_t hHandle, int index)
{
    return ERROR;
}

/** Turn OFF all relay channels on the USB-Relay-Device
@paramter: hHandle -- which usb relay device your want to operate
@returns: 0 -- success; 1 -- error
*/
int USBRL_API usb_relay_device_close_all_relay_channel(intptr_t hHandle)
{
    return ERROR;
}

/*
Status bits:  one bit indicate a relay status.
bit 0/1/2/3/4/5/6/7/8 indicate channel 1/2/3/4/5/6/7/8 status
 1 -- means ON, 0 -- means OFF.
@returns: 0 -- success; 1 -- error
*/
int USBRL_API usb_relay_device_get_status(intptr_t hHandle, unsigned int *status)
{
    return ERROR;
}

#ifdef __cplusplus
}
#endif 

