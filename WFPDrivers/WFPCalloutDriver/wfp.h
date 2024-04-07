
#pragma once
#define NDIS630

#include <initguid.h>
#include <fwpsk.h>
#include <fwpmk.h>



// {265A65E1-F9C3-4F97-82E1-E75300B33E06}
DEFINE_GUID(EXAMPLE_CALLOUT_UDP_GUID,0x265a65e1, 0xf9c3, 0x4f97, 0x82, 0xe1, 0xe7, 0x53, 0x0, 0xb3, 0x3e, 0x6);

// {622B619A-41B5-4854-9B06-6191E95AF336}
DEFINE_GUID(EXAMPLE_FILTERS_SUBLAYER_GUID,0x622b619a, 0x41b5, 0x4854, 0x9b, 0x6, 0x61, 0x91, 0xe9, 0x5a, 0xf3, 0x36);

NTSTATUS RegisterNetworkFilterUDP(PDEVICE_OBJECT DeviceObject);
void UnregisterNetwrokFilterUDP();