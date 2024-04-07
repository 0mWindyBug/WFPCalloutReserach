#include <ntddk.h>
#include "Ioctl.h"
#include "WfpReversal.h"


NTSTATUS CreateCloseDispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS IoctlHandler(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	NTSTATUS status;
	PIO_STACK_LOCATION irpStack;
	irpStack = IoGetCurrentIrpStackLocation(Irp);

	if (irpStack->Parameters.DeviceIoControl.IoControlCode == IOCTL_CALLOUT_ADDRESS_BY_ID)
	{
		if (irpStack->Parameters.DeviceIoControl.OutputBufferLength == sizeof(CALLOUT_INFO_DATA))
		{
			PCALLOUT_INFO_DATA CalloutData = (PCALLOUT_INFO_DATA)Irp->AssociatedIrp.SystemBuffer;
			WfpFindCalloutAddressById(CalloutData);
			Irp->IoStatus.Information = sizeof(CALLOUT_INFO_DATA);
			Irp->IoStatus.Status = STATUS_SUCCESS;
			IoCompleteRequest(Irp, IO_NO_INCREMENT);
			return STATUS_SUCCESS;
		}
		else
			status = STATUS_INVALID_PARAMETER;
	}
	else
	{
		status = STATUS_INVALID_DEVICE_REQUEST;
	}

	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}