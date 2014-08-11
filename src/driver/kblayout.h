/**
* Device specific data. Here we can store any state, information, etc.
* that the driver needs to run. Each device has its own device extension
*/
typedef struct _DEVICE_EXTENSION
{
    PDEVICE_OBJECT NextDevice; // next device in the stack. Can be another driver

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


/**
 * @brief Driver entry point
*/
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath 
    );

/**
 * @brief Attaches the driver to the device
*/
DRIVER_ADD_DEVICE KbLayoutAddDevice;
NTSTATUS
KbLayoutAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PDO
    );

/**
 * @brief Dispatch routing for PNP IRP
 *
 * Handles PNP so we detach from the device properly (e.g. when unplugged)
*/
__drv_dispatchType(IRP_MJ_PNP)
DRIVER_DISPATCH KbLayoutDispatchPNP;
NTSTATUS KbLayoutDispatchPNP(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp 
    );

/**
 * @brief Dispatch routing for the power IRP
 *
 * Power IRPs can't be handled with the general dispatch
*/
__drv_dispatchType(IRP_MJ_POWER)
DRIVER_DISPATCH KbLayoutDispatchPower;
NTSTATUS KbLayoutDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp 
    );


/**
 * @brief Clean up when unloading the driver
*/
DRIVER_UNLOAD KbLayoutUnload;
VOID
KbLayoutUnload(
   IN PDRIVER_OBJECT DriverObject
   );

/**
 * @brief Dispatch routine for the read IRP
 *
 * Registers intent that we actuall do want to handle the IRP,
 * and then passes on to the next driver. The actual processing
 * is handle with KbLayoutReadComplete
*/
__drv_dispatchType(IRP_MJ_READ)
DRIVER_DISPATCH KbLayoutDispatchRead;
NTSTATUS KbLayoutDispatchRead( 
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp 
    );

/**
 * @brief General dispatch for any IRP we are not interested in
*/
DRIVER_DISPATCH KbLayoutDispatchPassthrough;
NTSTATUS KbLayoutDispatchGeneral(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp 
    );

/**
 * @brief Process incoming keyboard input
 *
 * Swaps incoming scancodes so they emulate a different keyboard layout
*/
IO_COMPLETION_ROUTINE KbLayoutReadComplete;
NTSTATUS KbLayoutReadComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );
