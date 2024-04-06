#include "utils.h"
#include <ntddk.h>
#include "WfpReversal.h"
#include "Ioctl.h"

PDEVICE_OBJECT DeviceObject;
UNICODE_STRING deviceName, symLinkName;

KfdDeRefCalloutPtr KfdDeRefCallout;
KfdGetRefCalloutPtr KfdGetRefCallout;

void DriverUnload(PDRIVER_OBJECT DriverObject)
{
	IoDeleteDevice(DriverObject->DeviceObject);
	IoDeleteSymbolicLink(&symLinkName);
	DbgPrint("[*] driver has been unloaded\n");

}





EXTERN_C NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING ssdl;


	DriverObject->DriverUnload = DriverUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = CreateCloseDispatch;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = CreateCloseDispatch;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoctlHandler;


	RtlInitUnicodeString(&deviceName, L"\\Device\\WfpEnumDrv");
	RtlInitUnicodeString(&symLinkName, L"\\??\\WfpEnumDrv");

	RtlInitUnicodeString(&ssdl, L"D:P(A;;GA;;;SY)(A;;GA;;;BA)");
	status = IoCreateDeviceSecure(DriverObject, 0, &deviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &ssdl, NULL, &DeviceObject);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("[*] Failed to create device  (status = 0x%lx)\n", status);
		return status;
	}

	status = IoCreateSymbolicLink(&symLinkName, &deviceName);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("[*] Failed to create symlink  (status = %lx0x)\n", status);
		return status;
	}
	

	PVOID NetioBase = util::GetModuleBase("NETIO.SYS");
	if (!NetioBase)
	{
		DbgPrint("[-] failed to find netio.sys base address\n");
		return status;
	}
	DbgPrint("[*] netio.sys -> 0x%p\n", NetioBase);

	KfdDeRefCallout = (KfdDeRefCalloutPtr)util::FindExport(NetioBase, reinterpret_cast < const unsigned char*>("KfdDeRefCallout"));
	if (!KfdDeRefCallout)
	{
		DbgPrint("[-] failed to find netio!KfdDeRefCallout\n");
		return status;
	}
	DbgPrint("[*] netio!KfdDeRefCallout -> 0x%p\n", KfdDeRefCallout);

	KfdGetRefCallout = (KfdGetRefCalloutPtr)util::FindExport(NetioBase, reinterpret_cast < const unsigned char*>("KfdGetRefCallout"));
	if (!KfdGetRefCallout)
	{
		DbgPrint("[-] failed to find netio!KfdDeRefCallout\n");
		return status;
	}
	DbgPrint("[*] netio!KfdGetRefCallout -> 0x%p\n", KfdGetRefCallout);


	DbgPrint("[*] initailized successfully\n");
	return status;
}