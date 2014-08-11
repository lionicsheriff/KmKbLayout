#include "ntddk.h"
#include "configuration.h"
#include "string.h"
#define KBLAYOUT_POOL_TAG ' LBK'
#define KBLAYOUT_REG_BASE L"\\kblayout\\Layouts"

VOID
KbLayoutRegLoadConfig(
    IN USHORT CurrentLayout[]
    )
{
    UNICODE_STRING selectedLayout;
    RTL_QUERY_REGISTRY_TABLE regQueries[2] = {0}; // needs an empty struct to detect the end (remember to zero!)

    // find the selected layout
    regQueries[0].QueryRoutine = NULL; // directly modifying selectedLayout (see EntryContext)
    regQueries[0].Name = L"Selected Layout";
    regQueries[0].Flags = RTL_QUERY_REGISTRY_NOEXPAND
                        | RTL_QUERY_REGISTRY_DIRECT;
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
    RtlZeroMemory(CurrentLayout, sizeof(&CurrentLayout));

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
            DbgPrint("Mapping %i -> %i\n", inputScan, outputScan);
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
