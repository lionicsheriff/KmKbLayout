#include "ntddk.h"
#include "configuration.h"

VOID
KbLayoutRegLoad(USHORT currentLayout[])
{
	NTSTATUS status;
	RTL_QUERY_REGISTRY_TABLE regQueries[2]; // terminates on a zeroed entry (hence one larger than needed)

	regQueries[0].QueryRoutine = KbLayoutRegLoadScanCode;
	regQueries[0].Flags = RTL_QUERY_REGISTRY_NOEXPAND; // expanding is unsafe and shouldn't be used in kernel mode

	RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
		(PCWSTR)L"\\kblayout\\Layouts\\Dvorak\\Scancodes",
		regQueries,
		currentLayout,
		NULL);
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

	KbLayoutConvertUnicodeToInt(ValueName,&inputScan);

	if (ValueType == REG_DWORD){
		outputScan = ((PULONG) ValueData)[0];
	}

	layout[inputScan] = (USHORT)outputScan;
	return STATUS_SUCCESS;
}

NTSTATUS
KbLayoutConvertUnicodeToInt(PWSTR StrVal, int * IntVal){
	UNICODE_STRING unicodeStr;
	RtlInitUnicodeString(&unicodeStr, StrVal);
	return RtlUnicodeStringToInteger(&unicodeStr, 0, IntVal);
}