#include "ntifs.h"
#include "ntddk.h"
#include "configuration.h"
#include "string.h"

const PCWSTR KBLAYOUT_REG_ALTITUDE = L"424274.001"; // loads near the top, not actually allocated to this project though

VOID
KbLayoutRegLoadConfig(
    IN USHORT CurrentLayout[]
    )
{
    UNICODE_STRING selectedLayout;
    WCHAR selectedLayoutBuffer[250] = {0}; // arbitrary size
    RTL_QUERY_REGISTRY_TABLE regQueries[2] = {0}; // needs an empty struct to detect the end (remember to zero!)

    // prep the layout unicode_string so it can be loaded via RtlQueryRegistryValues
    // (oddly not needed if we are only doing this in DriverEntry)
    RtlInitEmptyUnicodeString(&selectedLayout, selectedLayoutBuffer, sizeof(selectedLayoutBuffer));

    // find the selected layout
    regQueries[0].QueryRoutine = NULL; // directly modifying selectedLayout (see EntryContext)
    regQueries[0].Name = L"Selected Layout";
    regQueries[0].Flags = RTL_QUERY_REGISTRY_NOEXPAND
                        | RTL_QUERY_REGISTRY_DIRECT
                        | RTL_QUERY_REGISTRY_TYPECHECK;
    regQueries[0].DefaultType = (REG_SZ << RTL_QUERY_REGISTRY_TYPECHECK_SHIFT) | REG_NONE; // give us some protection against usermode modifying the type
    regQueries[0].EntryContext = &selectedLayout;

    RtlQueryRegistryValues(RTL_REGISTRY_SERVICES, KBLAYOUT_REG_BASE, regQueries, NULL, NULL);

    // load the selected layout
    KbLayoutRegLoadLayout(&selectedLayout, CurrentLayout);
}

VOID
KbLayoutRegLoadLayout(
    IN PUNICODE_STRING layoutName,
    IN USHORT CurrentLayout[])
{
    NTSTATUS status;
    RTL_QUERY_REGISTRY_TABLE regQueries[2] = {0}; // needs an empty struct to detect the end (remember to zero!)
    UNICODE_STRING layoutKey;
    WCHAR layoutKeyBuffer[250] = {0}; // arbitrary size

    // set up the full registry path for the scancodes
    RtlInitEmptyUnicodeString(&layoutKey, layoutKeyBuffer, sizeof(layoutKeyBuffer));
    RtlAppendUnicodeToString(&layoutKey, L"\\kblayout\\Layouts\\");
    RtlAppendUnicodeStringToString(&layoutKey, layoutName);
    RtlAppendUnicodeToString(&layoutKey, L"\\Scancodes");

    // wipe the existing layout out
    // NOTE: this means that if the new layout does not exist, it will fallback to
    // passing through all scancodes.
    RtlZeroMemory(CurrentLayout, MAX_SCANCODE * sizeof(USHORT));

    // load the scancode key into the CurrentLayout
    regQueries[0].QueryRoutine = KbLayoutRegLoadScanCode;
    regQueries[0].Flags = RTL_QUERY_REGISTRY_NOEXPAND; // expanding is unsafe and shouldn't be used in kernel mode
    regQueries[0].Name = NULL; // all values
    RtlQueryRegistryValues(RTL_REGISTRY_SERVICES, layoutKeyBuffer, regQueries, CurrentLayout, NULL);
}

NTSTATUS
KbLayoutRegLoadScanCode(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    USHORT * layout = (USHORT*) Context;
    int inputScan, outputScan;
    PWSTR valueBuffer;
    UNREFERENCED_PARAMETER(EntryContext);

    // only use values that are a valid scancode map (hex->hex)
    // we are ignoring 0 as it is the default value and indicates that 
    // the string to number conversion failed. It also does not seem
    // to be a valid scandcode for any keyboard set.
    if (ValueType == REG_DWORD){
        if (NT_SUCCESS(KbLayoutConvertUnicodeToInt(ValueName,&inputScan))
            && inputScan != 0){
            outputScan = ((PULONG) ValueData)[0];
            layout[inputScan] = (USHORT)outputScan;
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS
KbLayoutConvertUnicodeToInt(
    IN PWSTR StrVal,
    IN OUT int * IntVal
    )
{
    UNICODE_STRING unicodeStr;
    RtlInitUnicodeString(&unicodeStr, StrVal);
    return RtlUnicodeStringToInteger(&unicodeStr, 0, IntVal);
}

NTSTATUS
KbLayoutMonitorConfig(
    IN PDRIVER_OBJECT DriverObject,
    IN USHORT CurrentLayout[],
    OUT PLARGE_INTEGER Cookie
    )
{
    NTSTATUS status;
    UNICODE_STRING altitude;

    RtlInitUnicodeString(&altitude, KBLAYOUT_REG_ALTITUDE);

    status = CmRegisterCallbackEx(&KmLayoutRegistryChanged, // callback
                                  &altitude, // Altitude
                                  DriverObject, // Driver
                                  CurrentLayout, // Context
                                  Cookie, // Cookie
                                  NULL// Reserved
        );

    return status;
}

NTSTATUS
KmLayoutRegistryChanged(
  IN PVOID CallbackContext,
  IN PVOID Argument1,
  IN PVOID Argument2
)
{
    NTSTATUS status;
    BOOLEAN reloadConfig; 
    REG_NOTIFY_CLASS type;
    PREG_POST_OPERATION_INFORMATION postOperationInformation;
    PREG_SET_VALUE_KEY_INFORMATION regSetValueInformation;

    status = STATUS_SUCCESS;
    type = (REG_NOTIFY_CLASS) Argument1;
    reloadConfig = FALSE;



    switch(type){
        case RegNtPostSetValueKey:
            // get the operation information
            postOperationInformation = (PREG_POST_OPERATION_INFORMATION) Argument2;
            regSetValueInformation = (PREG_SET_VALUE_KEY_INFORMATION) postOperationInformation->PreInformation;

            // trigger a reload if the kblayout config has been changed
            reloadConfig = KbLayoutIsConfigKey(regSetValueInformation->Object);
            break;
    }

    if (reloadConfig){
        // probably want some sort of debounce here so when a .reg is merged the
        // config is loaded once at the end
        KbLayoutRegLoadConfig((USHORT*) CallbackContext);
    }

    return status;
}

NTSTATUS
KbLayoutMonitorConfigUnload(
    IN LARGE_INTEGER Cookie
    )
{
    return CmUnRegisterCallback(Cookie);
}

BOOLEAN
KbLayoutIsConfigKey(
    IN PVOID RegistryObject
    )
{
    NTSTATUS status;
    ULONG length;
    PUNICODE_STRING objectName;

    objectName = NULL;

    // get the length of the registry key name
    status = ObQueryNameString(RegistryObject, (POBJECT_NAME_INFORMATION)objectName, 0, &length);

    if (status == STATUS_INFO_LENGTH_MISMATCH){

        // load the registry key
        objectName = ExAllocatePoolWithTag(NonPagedPool, length, KBLAYOUT_POOL_TAG);
        status = ObQueryNameString(RegistryObject, objectName, length, &length);

        // Assume the registry key is part of the kblayout config if it has
        // \\kblayout\Layouts in the path. This is probably unique enough, and extra layout
        // loading shouldn't hurt
        // A proper test would actually work out the current control set, and use the full path
        // e.g. \REGISTRY\MACHINE\System\ControlSet001\services\kblayout
        if(NT_SUCCESS(status)
            && wcsstr(objectName->Buffer,KBLAYOUT_REG_BASE)!= NULL){
            ExFreePool(objectName);
            return TRUE;
        }
        ExFreePool(objectName);

    }

    return FALSE;

}