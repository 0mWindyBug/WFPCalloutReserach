#include <Windows.h>
#include <iostream>
#include "wfp.h"
#include "Ioctl.h"



bool EnumerateCallouts()
{
    HANDLE EngHandle = nullptr;
    DWORD Status = FwpmEngineOpen0(nullptr, RPC_C_AUTHN_WINNT, nullptr, nullptr, &EngHandle);
    if (Status != ERROR_SUCCESS) {
        std::cerr << "[*] FwpmEngineOpen0  error ->  " << std::hex << Status << std::endl;
        return false;
    }
    HANDLE EnumHandle = nullptr;
    Status = FwpmCalloutCreateEnumHandle0(EngHandle, nullptr, &EnumHandle);
    if (Status != ERROR_SUCCESS) {
        FwpmEngineClose0(EngHandle);
        std::cerr << "[*] FwpmCalloutCreateEnumHandle0 error -> " << std::hex << Status << std::endl;
        return false;
    }
    UINT32 NumOfEntries = 0xFFFFFFFF; 
    FWPM_CALLOUT0** CalloutEntries = nullptr;
    UINT32 numEntriesReturned = 0;
    Status = FwpmCalloutEnum0(EngHandle, EnumHandle, NumOfEntries, &CalloutEntries, &numEntriesReturned);
    if (Status != ERROR_SUCCESS) {
        FwpmCalloutDestroyEnumHandle0(EngHandle, EnumHandle);
        FwpmEngineClose0(EngHandle);
        std::cerr << "[*] FwpmCalloutEnum0 error -> " << std::hex << Status << std::endl;
        return false;
    }
    WCHAR LayerGuidStr[100]; 
    for (UINT32 i = 0; i < numEntriesReturned; ++i) 
    {

        std::wstring CalloutName = CalloutEntries[i]->displayData.name;
        int CalloutId = CalloutEntries[i]->calloutId;
        GUID LayerGuid = CalloutEntries[i]->applicableLayer;
        if (!StringFromGUID2(LayerGuid, LayerGuidStr, sizeof(LayerGuidStr) / sizeof(WCHAR)))
        {
            FwpmFreeMemory0((void**)&CalloutEntries);
            FwpmCalloutDestroyEnumHandle0(EngHandle, EnumHandle);
            FwpmEngineClose0(EngHandle);
            std::cerr << "[*] FwpmCalloutEnum0 error -> " << std::hex << Status << std::endl;
            return false;
        }
        CALLOUT_INFO_DATA CalloutData = {};
        CalloutData.CalloutId = CalloutId;
        FindCalloutAddressByid(&CalloutData);
        std::wcout << "-----------------------------------------------------------------------------------------------------" << std::endl;
        std::wcout << " CalloutId = " << CalloutId << " Callout Name = " << CalloutName << " Layer = " << LayerGuidStr << std::endl;
        std::cout << " gWfpGloabl Callout Entry = 0x" << std::hex << CalloutData.CalloutEntry << " Classify Callback = 0x" << std::hex << CalloutData.ClassifyCalloutAddress << " Notify Callback = 0x" << std::hex << CalloutData.NotifyCalloutAddress << std::endl;
        std::wcout << "-----------------------------------------------------------------------------------------------------\n" << std::endl;

    }

    FwpmFreeMemory0((void**)&CalloutEntries);
    FwpmCalloutDestroyEnumHandle0(EngHandle, EnumHandle);
    FwpmEngineClose(EngHandle);
    return true;
}