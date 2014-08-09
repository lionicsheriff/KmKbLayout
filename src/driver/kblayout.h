#define MAX_SCANCODE 200 // arbitrary number, determines the largest scan code supported

typedef struct _DEVICE_EXTENSION
{
    PDEVICE_OBJECT  TopOfStack;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


NTSTATUS DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath 
    );

DRIVER_ADD_DEVICE KbLayoutAddDevice;
NTSTATUS
KbLayoutAddDevice(
    IN PDRIVER_OBJECT   Driver,
    IN PDEVICE_OBJECT   PDO
    );

__drv_dispatchType(IRP_MJ_PNP)
DRIVER_DISPATCH KbLayoutDispatchPNP;
NTSTATUS KbLayoutDispatchPNP(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp 
    );

__drv_dispatchType(IRP_MJ_POWER)
DRIVER_DISPATCH KbLayoutDispatchPower;
NTSTATUS KbLayoutDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp 
    );


DRIVER_UNLOAD KbLayoutUnload;
VOID
KbLayoutUnload(
   IN PDRIVER_OBJECT Driver
   );

__drv_dispatchType(IRP_MJ_READ)
DRIVER_DISPATCH KbLayoutDispatchRead;
NTSTATUS KbLayoutDispatchRead( 
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp 
    );

DRIVER_DISPATCH KbLayoutDispatchPassthrough;
NTSTATUS KbLayoutDispatchPassthrough(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp 
    );

IO_COMPLETION_ROUTINE KbLayoutReadComplete;
NTSTATUS KbLayoutReadComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );
