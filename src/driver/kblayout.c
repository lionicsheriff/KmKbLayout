#include "ntddk.h"
#include <ntddkbd.h>
#include "kblayout.h"
#include "stdarg.h"
#include "stdio.h"
#include "ntddkbd.h"
#include "configuration.h"

USHORT KBLAYOUT_CURRENT_LAYOUT[MAX_SCANCODE] = {0};
LARGE_INTEGER KBLAYOUT_REGISTRY_COOKIE;

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
    ULONG i;

    // Default to passing through all the entry points
    for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
        DriverObject->MajorFunction[i] = KbLayoutDispatchGeneral;
    }
    DriverObject->MajorFunction[IRP_MJ_POWER] = KbLayoutDispatchPower; // needs a special passthrough

    // Specify the entry points we want to override
    DriverObject->MajorFunction[IRP_MJ_READ] = KbLayoutDispatchRead; // sets up handler for incoming read events
    DriverObject->MajorFunction[IRP_MJ_PNP] = KbLayoutDispatchPNP; // detect when device is removed

    DriverObject->DriverUnload = KbLayoutUnload; // clean up routine when unloading
    DriverObject->DriverExtension->AddDevice = KbLayoutAddDevice; // attaches driver to device

    KbLayoutRegLoadConfig(KBLAYOUT_CURRENT_LAYOUT);
    KbLayoutMonitorConfig(DriverObject, KBLAYOUT_CURRENT_LAYOUT, &KBLAYOUT_REGISTRY_COOKIE);

    return STATUS_SUCCESS;
}

NTSTATUS
KbLayoutAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PDO
    )
{
    PDEVICE_EXTENSION devExt;
    PDEVICE_OBJECT device;
    NTSTATUS status = STATUS_SUCCESS;

    // Create the filter driver
    status = IoCreateDevice(DriverObject,             // DriverObject
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

    devExt->NextDevice = IoAttachDeviceToDeviceStack(device, PDO);

    ASSERT(devExt->NextDevice); // if it is null, we were unable to attach and that is a problem

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

    devExt = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    switch(irpStack->MinorFunction){
        case IRP_MN_REMOVE_DEVICE:
            IoSkipCurrentIrpStackLocation(Irp);
            IoCallDriver(devExt->NextDevice, Irp);
            IoDetachDevice(devExt->NextDevice); // the argument is actually the device to detach *from*
            IoDeleteDevice(DeviceObject);
            break;

        default:
            IoSkipCurrentIrpStackLocation(Irp);
            status = IoCallDriver(devExt->NextDevice, Irp);
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

    PoStartNextPowerIrp(Irp); // required by every driver on ever power irp (< Windows Vista)
    IoSkipCurrentIrpStackLocation(Irp);
    return PoCallDriver(devExt->NextDevice, Irp); // also required by < Windows Vista
}

VOID
KbLayoutUnload(
    IN PDRIVER_OBJECT DriverObject
    )
{
    UNREFERENCED_PARAMETER(DriverObject); // Suppress compiler warning re Driver not being used

    KbLayoutMonitorConfigUnload(KBLAYOUT_REGISTRY_COOKIE);
}

NTSTATUS
KbLayoutDispatchRead(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp)
{
    PDEVICE_EXTENSION devExt;
    PIO_STACK_LOCATION currentIrpStack;
    PIO_STACK_LOCATION nextIrpStack;

    devExt=(PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    currentIrpStack = IoGetCurrentIrpStackLocation(Irp);
    nextIrpStack = IoGetNextIrpStackLocation(Irp);

    *nextIrpStack = *currentIrpStack;

    IoSetCompletionRoutine(Irp,                   // Irp
                           KbLayoutReadComplete,  // CompletionRoutine
                           DeviceObject,          // Context
                           TRUE,                  // InvokeOnSuccess
                           TRUE,                  // InvokeOnError
                           TRUE                   // InvokeOnCancel
                           );

    return IoCallDriver(devExt->NextDevice, Irp);
}

NTSTATUS
KbLayoutDispatchGeneral(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PDEVICE_EXTENSION devExt;
    devExt=(PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    IoSkipCurrentIrpStackLocation(Irp);
    return IoCallDriver(devExt->NextDevice, Irp);
}

NTSTATUS
KbLayoutReadComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PIO_STACK_LOCATION irpSp;
    PKEYBOARD_INPUT_DATA keyData;
    int i, numKeys;

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    if(NT_SUCCESS(Irp->IoStatus.Status)) {
        keyData = Irp->AssociatedIrp.SystemBuffer;
        numKeys = (int) (Irp->IoStatus.Information / sizeof(KEYBOARD_INPUT_DATA));

        for(i = 0; i < numKeys; i++){
            USHORT scancode = keyData[i].MakeCode;
            // only handle scancodes we have allocated space for
            if (scancode < MAX_SCANCODE) {
                USHORT mapped = KBLAYOUT_CURRENT_LAYOUT[scancode];
                // swap the scancode if one has been found in the map
                // 0 is not a scancode so is used mark that we will just
                // pass through the scancode untouched
                if(mapped != 0){
                    keyData[i].MakeCode = mapped;
                }
            }
        }

    }

    // mark pending if required (just part of being an IoCompletionRoutine)
    if (Irp->PendingReturned){
        IoMarkIrpPending(Irp);
    }
    return Irp->IoStatus.Status;
}
