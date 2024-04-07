#pragma once
#include <ntddk.h>
#include <wdmsec.h>


#define IOCTL_CALLOUT_ADDRESS_BY_ID CTL_CODE(FILE_DEVICE_UNKNOWN, 0x807, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)


typedef struct _CALLOUT_INFO_DATA
{
	int CalloutId;
	DWORD64 CalloutEntry;
	DWORD64 ClassifyCalloutAddress;
	DWORD64 NotifyCalloutAddress;
} CALLOUT_INFO_DATA, * PCALLOUT_INFO_DATA;

NTSTATUS CreateCloseDispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS IoctlHandler(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);