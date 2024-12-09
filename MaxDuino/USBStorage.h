#include "configs.h"
#ifndef USBSTORAGE_H_INCLUDED
#define USBSTORAGE_H_INCLUDED

#ifdef USB_STORAGE_ENABLED
void setup_usb_storage();
void usb_detach();
void usb_retach();
#endif

#endif // USBSTORAGE_H_INCLUDED
