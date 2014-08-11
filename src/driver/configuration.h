VOID KbLayoutRegLoadConfig(USHORT CurrentLayout[]);
VOID KbLayoutRegLoadLayout(PUNICODE_STRING LayoutName, USHORT CurrentLayout[]);
RTL_QUERY_REGISTRY_ROUTINE KbLayoutRegLoadScanCode;
NTSTATUS KbLayoutRegLoadScanCode();
NTSTATUS KbLayoutConvertUnicodeToInt(PWSTR StrVal, int * IntVal);
