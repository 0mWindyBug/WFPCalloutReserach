#include <Windows.h>
#include <iostream>
#include "Ioctl.h"
#include "AutoHandle.h"


bool FindCalloutAddressByid(PCALLOUT_INFO_DATA OutCalloutInfo)
{


    DWORD BytesRet;
    HANDLE device = CreateFile(TARGET_DEVICE, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    AutoHandle AutHan(device);
    if (device == INVALID_HANDLE_VALUE) {
        std::cout << "[*] Failed to obtain  device handle" << std::endl;
        return false;
    }

    BOOL success = DeviceIoControl(device, IOCTL_CALLOUT_ADDRESS_BY_ID, OutCalloutInfo, sizeof(CALLOUT_INFO_DATA), OutCalloutInfo, sizeof(CALLOUT_INFO_DATA), &BytesRet, nullptr);

    if (!success) {
        std::cout << "[*] Failed in DeviceIoControl: " << GetLastError() << std::endl;
        return false;
    }

    return true;
    

}