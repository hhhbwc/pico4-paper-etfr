#pragma once
#include <sys/ioctl.h>
struct paper_usbdevfs_bulktransfer { unsigned int ep; unsigned int len; unsigned int timeout; void* data; };
struct paper_usbdevfs_ctrltransfer { unsigned char bRequestType; unsigned char bRequest; unsigned short wValue; unsigned short wIndex; unsigned short wLength; unsigned int timeout; void* data; };
#ifndef USBDEVFS_BULK
#define USBDEVFS_BULK _IOWR('U', 2, struct paper_usbdevfs_bulktransfer)
#endif
#ifndef USBDEVFS_CLAIMINTERFACE
#define USBDEVFS_CLAIMINTERFACE _IOR('U', 15, unsigned int)
#endif
#ifndef USBDEVFS_RELEASEINTERFACE
#define USBDEVFS_RELEASEINTERFACE _IOR('U', 16, unsigned int)
#endif
#ifndef USBDEVFS_CONTROL
#define USBDEVFS_CONTROL _IOWR('U', 0, struct paper_usbdevfs_ctrltransfer)
#endif
