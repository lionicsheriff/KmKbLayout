/* DEPRECATED
 * mapper due for a rewrite so it can load keyboards from a file
*/
VOID
KbLayoutMapFromQwerty(
	USHORT layout[],
	char map[][2],
	int mapLength
	);

USHORT
KbLayoutGetQwertyScancode(
	char character
	);