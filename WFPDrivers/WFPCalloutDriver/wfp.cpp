#include <ntddk.h>
#include "wfp.h"

HANDLE EngHandle = nullptr;
UINT32 CalloutId = 0;
UINT32 SystemCalloutId = 0; 
UINT64 FilterId = 0;


VOID ClassifyCallback(
	const FWPS_INCOMING_VALUES* inFixedValues,
	const FWPS_INCOMING_METADATA_VALUES* inMetaValues,
	void* layerData,
	const void* classifyContext,
	const FWPS_FILTER* filter,
	UINT64 flowContext,
	FWPS_CLASSIFY_OUT* classifyOut
) {
	UNREFERENCED_PARAMETER(inFixedValues);
	UNREFERENCED_PARAMETER(inMetaValues);
	UNREFERENCED_PARAMETER(layerData);
	UNREFERENCED_PARAMETER(classifyContext);
	UNREFERENCED_PARAMETER(filter);
	UNREFERENCED_PARAMETER(flowContext);
	UNREFERENCED_PARAMETER(classifyOut);

	DbgPrint("[*] inside callout driver udp classify logic...\n");
	
}

NTSTATUS NotifyCallback(
	FWPS_CALLOUT_NOTIFY_TYPE  notifyType,
	const GUID* filterKey,
	FWPS_FILTER* filter
) {
	UNREFERENCED_PARAMETER(notifyType);
	UNREFERENCED_PARAMETER(filterKey);
	UNREFERENCED_PARAMETER(filter);

	DbgPrint("[*] inside callout driver udp notify logic...\n");

	return STATUS_SUCCESS;
}


NTSTATUS RegisterNetworkFilterUDP(PDEVICE_OBJECT DeviceObject)
{
	
	NTSTATUS status = STATUS_SUCCESS;

	// open filter engine session 
	status = FwpmEngineOpen(NULL, RPC_C_AUTHN_WINNT, NULL, NULL, &EngHandle);
	if (!NT_SUCCESS(status)) {
		DbgPrint("[*] failed to open filter engine\n");
		return status;
	}

	// register callout in filter engine 
	FWPS_CALLOUT callout = {};
	callout.calloutKey = EXAMPLE_CALLOUT_UDP_GUID;
	callout.flags = 0;
	callout.classifyFn = ClassifyCallback;
	callout.notifyFn = NotifyCallback;
	callout.flowDeleteFn = nullptr;
	
	status =  FwpsCalloutRegister(DeviceObject, &callout, &CalloutId);
	if (!NT_SUCCESS(status)) {
		DbgPrint("[*] failed to register callout in filter engine\n");
		return status;
	}

	// add callout to the system 
	FWPM_CALLOUT calloutm = { };
	calloutm.flags = 0;							 
	calloutm.displayData.name = L"example callout udp";
	calloutm.displayData.description = L"example PoC callout for udp ";
	calloutm.calloutKey = EXAMPLE_CALLOUT_UDP_GUID;
	calloutm.applicableLayer = FWPM_LAYER_DATAGRAM_DATA_V4;
	

	status =  FwpmCalloutAdd(EngHandle, &calloutm, NULL, &SystemCalloutId);
	if (!NT_SUCCESS(status)) {
		DbgPrint("[*] failed to add callout to the system \n");
		return status;
	}

	// create a sublayer to group filters (not actually required 
	FWPM_SUBLAYER sublayer = {};
	sublayer.displayData.name = L"PoC sublayer example filters";
	sublayer.displayData.name = L"PoC sublayer examle filters";
	sublayer.subLayerKey = EXAMPLE_FILTERS_SUBLAYER_GUID;
	sublayer.weight = 65535;


	status =  FwpmSubLayerAdd(EngHandle, &sublayer, NULL);
	if (!NT_SUCCESS(status)) {
		DbgPrint("[*] failed to create a sublayer\n");
		return status;
	}

	// add a filter that references our callout with no conditions 
	UINT64 weightValue = 0xFFFFFFFFFFFFFFFF;								
	FWP_VALUE weight = {};
	weight.type = FWP_UINT64;
	weight.uint64 = &weightValue;

	// process every packet , no conditions 
	FWPM_FILTER_CONDITION conditions[1] = { 0 };										\

		FWPM_FILTER filter = {};
	filter.displayData.name = L"example filter callout udp";
	filter.displayData.name = L"example filter calout udp";
	filter.layerKey = FWPM_LAYER_DATAGRAM_DATA_V4;
	filter.subLayerKey = EXAMPLE_FILTERS_SUBLAYER_GUID;
	filter.weight = weight;
	filter.numFilterConditions = 0;
	filter.filterCondition = conditions;
	filter.action.type = FWP_ACTION_CALLOUT_INSPECTION;
	filter.action.calloutKey = EXAMPLE_CALLOUT_UDP_GUID;
	

	return FwpmFilterAdd(EngHandle, &filter, NULL, &FilterId);
}




void UnregisterNetwrokFilterUDP()
{
	if (EngHandle)
	{
		if (FilterId) 
		{
			FwpmFilterDeleteById(EngHandle, FilterId);
			FwpmSubLayerDeleteByKey(EngHandle, &EXAMPLE_FILTERS_SUBLAYER_GUID);
		}

		// Remove the callout from the FWPM_LAYER_INBOUND_TRANSPORT_V4 layer
		if (SystemCalloutId) {
			FwpmCalloutDeleteById(EngHandle, SystemCalloutId);
		}

		// Unregister the callout
		if (CalloutId) {
			FwpsCalloutUnregisterById(CalloutId);
		}
		FwpmEngineClose(EngHandle);
	}
}