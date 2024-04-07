#include <ntddk.h>
#include "WfpReversal.h"
#include "Ioctl.h"



bool WfpFindCalloutAddressById(PCALLOUT_INFO_DATA CalloutData)
{
	PVOID CalloutEntryPtr = nullptr;
	KfdGetRefCallout(CalloutData->CalloutId, &CalloutEntryPtr);
	if (!CalloutEntryPtr)
	{
		DbgPrint("[-] KfdGetRefCallout failed finding callout entry for %d callout id\n", CalloutData->CalloutId);
		return false;
	}

	DWORD64 ClassifyCallback = (DWORD64)(*(PDWORD64)((ULONG_PTR)CalloutEntryPtr + CALLOUT_ENTRY_CLASSIFY_OFFSET));
	DWORD64 NotifyCallback   = (DWORD64)(*(PDWORD64)((ULONG_PTR)CalloutEntryPtr + CALLOUT_ENTRY_NOTIFY_OFFSET));

	DbgPrint("------------------------------------------------------------------------------------\n");
	DbgPrint("[*] Callout Id -> %d\n", CalloutData->CalloutId);
	DbgPrint("[*] Callout Entry Structure -> 0x%p\n", CalloutEntryPtr);
	DbgPrint("[*] Callout CLassify Function -> 0x%p\n", ClassifyCallback);
	DbgPrint("[*] Callout Notify Function -> 0x%p\n", NotifyCallback);
	DbgPrint("------------------------------------------------------------------------------------\n");

	CalloutData->CalloutEntry = (DWORD64)CalloutEntryPtr;
	CalloutData->ClassifyCalloutAddress = ClassifyCallback;
	CalloutData->NotifyCalloutAddress = NotifyCallback;

		
	KfdDeRefCallout(CalloutEntryPtr);

	return true;
}
