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


/**
 *@brief Start watching the registry for changes to the config
*/
NTSTATUS
KbLayoutMonitorConfig(
    IN PDRIVER_OBJECT DriverObject,
    IN USHORT CurrentLayout[],
    OUT PLARGE_INTEGER Cookie
    );

/**
 *@brief Update loaded configuration when the registry changes
*/
EX_CALLBACK_FUNCTION KmLayoutRegistryChanged;
NTSTATUS
KmLayoutRegistryChanged(
  IN PVOID CallbackContext,
  IN PVOID Argument1,
  IN PVOID Argument2
);

/**
 *@brief Stop watching the registry for changes to the config
*/
NTSTATUS
KbLayoutMonitorConfigUnload(
    IN LARGE_INTEGER Cookie
    );

/**
 *@brief Check if a registry key is used as configuration
*/
BOOLEAN
KbLayoutIsConfigKey(
    IN PVOID RegistryObject
    );