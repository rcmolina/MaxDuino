/*********************************************************************
 USBStorage derived from Adafruit's TinyUSB "msc_sd" example
 modified to use latest sdfat and integrated with TZXDuino

 Adafruit header follows below:
 ********************************************************************
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 Copyright (c) 2019 Ha Thach for Adafruit Industries
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

#include "configs.h"

#ifdef USB_STORAGE_ENABLED

#include "Adafruit_TinyUSB.h"
#include "file_utils.h"

Adafruit_USBD_MSC usb_msc;

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and
// return number of copied bytes (must be multiple of block size)
int32_t msc_read_cb (uint32_t lba, void* buffer, uint32_t bufsize)
{
  (void) bufsize;
  return sd.card()->readSector(lba, (uint8_t*) buffer) ? 512 : -1;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and 
// return number of written bytes (must be multiple of block size)
int32_t msc_write_cb (uint32_t lba, uint8_t* buffer, uint32_t bufsize)
{
  (void) bufsize;
  return sd.card()->writeSector(lba, buffer) ? 512 : -1;
}

// Callback invoked when WRITE10 command is completed (status received and accepted by host).
// used to flush any pending cache.
void msc_flush_cb (void)
{
  // nothing to do
}

void usb_detach()
{
  //usb_msc.detach();
  USBDevice.detach();
}

void usb_retach()
{
  //usb_msc.retach();
  USBDevice.attach();
}

void setup_usb_storage()
{
  // Set disk vendor id, product id and revision with string up to 8, 16, 4 characters respectively
  usb_msc.setID("MaxDuino", "SD Card", "1.0");

  // Set read write callback
  usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);

  // Still initialize MSC but tell usb stack that MSC is not ready to read/write
  // If we don't initialize, board will be enumerated as CDC only
  usb_msc.setUnitReady(false);
  usb_msc.begin();

  uint32_t sector_count = sd.card()->sectorCount();

  // Set disk size, SD block size is always 512
  usb_msc.setCapacity(sector_count, 512);

  // MSC is ready for read/write
  usb_msc.setUnitReady(true);
}

#endif
