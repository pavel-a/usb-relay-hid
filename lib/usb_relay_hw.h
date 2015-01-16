#ifndef USB_RELAY_DEVICE_HW__
#define USB_RELAY_DEVICE_HW__

#define USB_CFG_VENDOR_ID       0x16c0 /* 5824 = voti.nl */
#define USB_CFG_DEVICE_ID       0x05DF /* obdev's shared PID for HIDs */
#define USB_RELAY_VENDOR_NAME   "www.dcttech.com"
#define USB_RELAY_NAME_PREF     "USBRelay"  /* can be relay1... relay8 */

#define USB_RELAY_ID_STR_LEN    5 /* length of "unique serial number" in the devices */

#endif /*USB_RELAY_DEVICE_HW__*/
