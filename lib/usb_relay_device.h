#ifndef USB_RELAY_DEVICE_H__
#define USB_RELAY_DEVICE_H__

#define USBRELAY_LIB_VER 0x02

#ifdef WIN32
#ifdef _MSC_VER /* Microsoft compiler: */
#
#ifndef USBRL_CALL
#  define USBRL_CALL __cdecl
#endif
#ifndef USBRL_API
#  define USBRL_API __declspec(dllimport) USBRL_CALL
#  pragma comment(lib, "usb_relay_device")
#endif
#
#else /* Windows & Not Microsoft's compiler */
#  define USBRL_CALL
#  define USBRL_API USBRL_CALL
#endif /* Windows & Not Microsoft's compiler */
#else /* non-Windows */
  /* do not include this always for compat with VC++ < 2010 */
#include <stdint.h>
#endif /* WIN32 */

#ifndef USBRL_CALL
#  define USBRL_CALL
#endif
#ifndef USBRL_API
#  define USBRL_API USBRL_CALL
#endif


#include <stddef.h>

enum usb_relay_device_type
{
    USB_RELAY_DEVICE_ONE_CHANNEL   = 1,
    USB_RELAY_DEVICE_TWO_CHANNEL   = 2,
    USB_RELAY_DEVICE_FOUR_CHANNEL  = 4,
    USB_RELAY_DEVICE_EIGHT_CHANNEL = 8
};


/** USB relay board info structure */
struct usb_relay_device_info
{
    char *serial_number;
    char *device_path;
    intptr_t /*enum usb_relay_device_type*/ type;
    struct usb_relay_device_info* next;
};

typedef struct usb_relay_device_info *pusb_relay_device_info_t;

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize the USB Relay Library
@returns: This function returns 0 on success and -1 on error.
*/
int USBRL_API usb_relay_init(void);

/** Finalize the USB Relay Library.
This function frees all of the static data associated with USB Relay Library.
It should be called at the end of execution to avoid memory leaks.
@returns:This function returns 0 on success and -1 on error.
*/
int USBRL_API usb_relay_exit(void);

/** Enumerate the USB Relay Devices.
@return Pointer to list of usb_relay_device_info
        Caller should free it with usb_relay_device_free_enumerate
*/
pusb_relay_device_info_t USBRL_API usb_relay_device_enumerate(void);

/** Free an enumeration Linked List */
void USBRL_API usb_relay_device_free_enumerate(struct usb_relay_device_info*);

/** Open device that serial number is serial_number
@return: This function returns a valid handle to the device on success or NULL on failure.
e.g: usb_relay_device_open_with_serial_number("abcde", 5") */
intptr_t USBRL_API usb_relay_device_open_with_serial_number(const char *serial_number, unsigned len);

/* Open a USB relay device
@return: This function returns a valid handle to the device on success or NULL on failure.
*/
intptr_t USBRL_API usb_relay_device_open(struct usb_relay_device_info *device_info);

/* Close a USB relay device */
void USBRL_API usb_relay_device_close(intptr_t hHandle);

/** Turn ON a relay channel on the USB-Relay-Device
@param hHandle -- which usb relay device your want to operate
@param index -- the channel number: 1...max
@returns: 0 -- success; 1 -- error; 2 -- index is invalid
*/
int USBRL_API usb_relay_device_open_one_relay_channel(intptr_t hHandle, int index);

/** Turn ON all relay channels on the USB-Relay-Device
@param hHandle -- which usb relay device your want to operate
@returns: 0 -- success; 1 -- error
*/
int USBRL_API usb_relay_device_open_all_relay_channel(intptr_t hHandle);

/** Turn OFF a relay channel on the USB-Relay-Device
@param index -- which channel your want to close
@param hHandle -- which usb relay device your want to operate
@returns: 0 -- success; 1 -- error
*/
int USBRL_API usb_relay_device_close_one_relay_channel(intptr_t hHandle, int index);

/** Turn OFF all relay channels on the USB-Relay-Device
@param hHandle -- which usb relay device your want to operate
@returns: 0 -- success; 1 -- error; 2 -- index is invalid
*/
int USBRL_API usb_relay_device_close_all_relay_channel(intptr_t hHandle);

/** Get state of all channels of the  USB-Relay-Device
Status bits:  one bit indicate a relay status.
bit 0/1/2/3/4/5/6/7/8 indicate channel 1/2/3/4/5/6/7/8 status
Bit value 1 means ON, 0 means OFF.
@returns: 0 -- success; 1 -- error
*/
int USBRL_API usb_relay_device_get_status(intptr_t hHandle, unsigned int *status);

#if 1 /* added */

/** Get the library (dll) version
@return Lower 16 bits: the library version. Higher bits: undefined, ignore.
@note The original DLL does not have this function!
*/
int USBRL_API usb_relay_device_lib_version(void);


/**
 The following functions are for non-native callers, to avoid fumbling with C structs and pointers.
 Native C/C++ callers do not need to use these.
 The ptr_usb_relay_device_info arg is pointer to struct usb_relay_device_info, cast to intptr_t, void*, etc.
*/

/* Return next info struct pointer in the list returned by usb_relay_device_enumerate() */
intptr_t USBRL_API usb_relay_device_next_dev(intptr_t ptr_usb_relay_device_info);

/* Get number of relay channels on the device */
int      USBRL_API usb_relay_device_get_num_relays(intptr_t ptr_usb_relay_device_info);

/* Get the ID string of the device. Returns pointer to const C string (1-byte, 0-terminated) */
intptr_t USBRL_API usb_relay_device_get_id_string(intptr_t ptr_usb_relay_device_info);

/* Get status of all relays on the device.
* @param hHandle The relay device handle
* @return Bitmask of all relay channels state, if the value > 0. Negative values mean error.
*  bit 0/1/2/3/4/5/6/7/8 indicate channel 1/2/3/4/5/6/7/8 status
*  Each bit value 1 means ON, 0 means OFF.
* @note This is same as usb_relay_device_get_status, but without dereferencing pointers.
*/
int USBRL_API usb_relay_device_get_status_bitmap(intptr_t hHandle);

#endif /* added */

#ifdef __cplusplus
}
#endif

#endif /* USB_RELAY_DEVICE_H__ */
