#include <ntddk.h>
#include "wfp.h"


PDEVICE_OBJECT DeviceObject;

void DriverUnload(PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);
	
	UnregisterNetwrokFilterUDP();

	IoDeleteDevice(DeviceObject);

	DbgPrint("[*] callout driver unloaded\n");
}



EXTERN_C NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	NTSTATUS status = STATUS_SUCCESS;

	status = IoCreateDevice(DriverObject, 0, NULL, FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject);
	if (!NT_SUCCESS(status)) {
		DbgPrint("[*] failed to crate device object for callout driver\n");
		return status;
	}
	
	status = RegisterNetworkFilterUDP(DeviceObject);
	if (!NT_SUCCESS(status)) {
		DbgPrint("[*] failed to register callout filter driver\n");
		return status;
	}

	DriverObject->DriverUnload = DriverUnload;
	DbgPrint("[*] callout filter driver has been registered and loaded\n");
	return status;
}