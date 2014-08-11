/**
 * @brief Load the current configuration from the registry
 * 
*/
VOID
KbLayoutRegLoadConfig(
    IN USHORT CurrentLayout[]
    );

/**
 * @brief Load a named layout from the registry
 * 
*/
VOID
KbLayoutRegLoadLayout(
    IN PUNICODE_STRING LayoutName,
    IN OUT USHORT CurrentLayout[]
    );

/**
 * @brief Callback routine to load a scancode map from the registry into the current layout
 * 
*/
RTL_QUERY_REGISTRY_ROUTINE KbLayoutRegLoadScanCode;
NTSTATUS
KbLayoutRegLoadScanCode();

/**
 * @brief Convert a unicode string to an integer
 *
 * 
*/
NTSTATUS
KbLayoutConvertUnicodeToInt(
    IN PWSTR StrVal,
    IN OUT int * IntVal
    );
