#include "ntddk.h"
#include <ntddkbd.h>
#include "kblayout.h"
#include "stdarg.h"
#include "stdio.h"
#include "ntddkbd.h"

NTSTATUS
DriverEntry(
	IN PDRIVER_OBJECT DriverObject,
	IN PUNICODE_STRING RegistryPath
	)
{
	ULONG i;

	DbgPrint("kblayout.sys: DriverEntry\n");

	// Default to passing through all the entry points
	for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
		DriverObject->MajorFunction[i] = KbLayoutDispatchPassthrough;
	}
	DriverObject->MajorFunction[IRP_MJ_POWER] = KbLayoutDispatchPower; // needs a special passthrough

	// Specify the entry points we want to override
	DriverObject->MajorFunction[IRP_MJ_READ] = KbLayoutDispatchRead; // 
	DriverObject->MajorFunction[IRP_MJ_PNP] = KbLayoutDispatchPNP; // detect when device is removed

	DriverObject->DriverUnload = KbLayoutUnload;
	DriverObject->DriverExtension->AddDevice = KbLayoutAddDevice;

	return STATUS_SUCCESS;
}

NTSTATUS
KbLayoutAddDevice(
	IN PDRIVER_OBJECT Driver,
	IN PDEVICE_OBJECT PDO
	)
{
	PDEVICE_EXTENSION devExt;
	IO_ERROR_LOG_PACKET errorLogEntry;
	PDEVICE_OBJECT device;
	NTSTATUS status = STATUS_SUCCESS;

	DbgPrint("kblayout.sys: add\n");
	// Create the filter driver
	status = IoCreateDevice(Driver,                   // DriverObject
							sizeof(DEVICE_EXTENSION), // DeviceExtensionSize
							NULL,                     // DeviceName
							FILE_DEVICE_KEYBOARD,     // DeviceType
							0,                        // DeviceCharacteristics
							FALSE,                    // Exclusive
							&device                   // (out) DeviceObject
							);

	if (!NT_SUCCESS(status)) {
		return status;
	}

	// attach the driver to the top of the device stack
	RtlZeroMemory(device->DeviceExtension, sizeof(DEVICE_EXTENSION));
	devExt = (PDEVICE_EXTENSION) device->DeviceExtension;

	devExt->TopOfStack = IoAttachDeviceToDeviceStack(device, PDO);

	ASSERT(devExt->TopOfStack);

	device->Flags |= (DO_BUFFERED_IO | DO_POWER_PAGABLE);
	device->Flags &= ~DO_DEVICE_INITIALIZING; // not intialising

	return status;
}


NTSTATUS
KbLayoutDispatchPNP(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
	)
{
	PDEVICE_EXTENSION devExt;
	PIO_STACK_LOCATION irpStack;
	NTSTATUS status = STATUS_SUCCESS;
	KIRQL oldIrql;
	KEVENT ekent;

	DbgPrint("kblayout.sys: pnp\n");
	devExt = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
	irpStack = IoGetCurrentIrpStackLocation(Irp);

	switch(irpStack->MinorFunction){
		case IRP_MN_REMOVE_DEVICE:
			IoSkipCurrentIrpStackLocation(Irp);
			IoCallDriver(devExt->TopOfStack, Irp);
			IoDetachDevice(devExt->TopOfStack);
			IoDeleteDevice(DeviceObject);
			break;

		default:
			IoSkipCurrentIrpStackLocation(Irp);
			status = IoCallDriver(devExt->TopOfStack, Irp);
			break;


	}

	return status;
}

NTSTATUS
KbLayoutDispatchPower(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
	)
{
	PDEVICE_EXTENSION devExt;
	devExt = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

	DbgPrint("kblayout.sys: power\n");
	PoStartNextPowerIrp(Irp);
	IoSkipCurrentIrpStackLocation(Irp);
	return PoCallDriver(devExt->TopOfStack, Irp);
}

VOID
KbLayoutUnload(
	IN PDRIVER_OBJECT Driver
	)
{
	DbgPrint("kblayout.sys: unload\n");
	UNREFERENCED_PARAMETER(Driver); // Suppress compiler warning re Driver not being used
}

NTSTATUS
KbLayoutDispatchRead(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp)
{
	PDEVICE_EXTENSION devExt;
	PIO_STACK_LOCATION currentIrpStack;
	PIO_STACK_LOCATION nextIrpStack;

	DbgPrint("kblayout.sys: read\n");
	devExt=(PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
	currentIrpStack = IoGetCurrentIrpStackLocation(Irp);
	nextIrpStack = IoGetNextIrpStackLocation(Irp);

	*nextIrpStack = *currentIrpStack;

	IoSetCompletionRoutine(Irp,
						   KbLayoutReadComplete,
						   DeviceObject,
						   TRUE,
						   TRUE,
						   TRUE
						   );

	return IoCallDriver(devExt->TopOfStack, Irp);
}

NTSTATUS KbLayoutDispatchPassthrough(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
	)
{
	PDEVICE_EXTENSION devExt;
	devExt=(PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

	DbgPrint("kblayout.sys: passing through\n");
	IoSkipCurrentIrpStackLocation(Irp);
	return IoCallDriver(devExt->TopOfStack, Irp);
}

NTSTATUS KbLayoutReadComplete(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp,
	IN PVOID Context
	)
{
	PIO_STACK_LOCATION irpSp;
	PKEYBOARD_INPUT_DATA keyData;
	int i, numKeys;
	DbgPrint("kblayout.sys: keypressed :)\n");

	irpSp = IoGetCurrentIrpStackLocation(Irp);
	if(NT_SUCCESS(Irp->IoStatus.Status)) {
		keyData = Irp->AssociatedIrp.SystemBuffer;
		numKeys = (int) (Irp->IoStatus.Information / sizeof(KEYBOARD_INPUT_DATA));

		for(i = 0; i < numKeys; i++){
			keyData[i].MakeCode = (USHORT) 0x1E;
		}

	}

	if (Irp->PendingReturned){
		IoMarkIrpPending(Irp);
	}
	return Irp->IoStatus.Status;
}