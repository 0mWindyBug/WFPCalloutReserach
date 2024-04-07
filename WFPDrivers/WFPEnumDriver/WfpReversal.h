#pragma once
#include <ntddk.h>
#include "Ioctl.h"

#define CALLOUT_ENTRY_CLASSIFY_OFFSET 0x10 
#define CALLOUT_ENTRY_NOTIFY_OFFSET   0x18 


// return callout entry pointer from callout id (exported bt netio.sys)
typedef PVOID(__fastcall *KfdGetRefCalloutPtr)(_In_ INT64 CalloutId, _Out_ PVOID* CalloutEntry);

// dereference callout entry (exported by netio.sys)
typedef void(__fastcall *KfdDeRefCalloutPtr)(_In_ PVOID CalloutEntry);

// return pointer to g_WfpGlobal (exported by netio.sys) 
typedef PVOID(__fastcall* FeGetWfpGlobalPtrPtr)();  


extern KfdDeRefCalloutPtr KfdDeRefCallout;
extern KfdGetRefCalloutPtr KfdGetRefCallout;

bool WfpFindCalloutAddressById(PCALLOUT_INFO_DATA CalloutData);