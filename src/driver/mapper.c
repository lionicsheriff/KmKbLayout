#include "ntddk.h"
#include "mapper.h"

VOID
KbLayoutMapFromQwerty(
	USHORT layout[],
	char map[][2],
	int mapLength
	)
{
	int i;
	for (i = 0; i < mapLength; i++) {
		char oldVal = map[i][0]
		   , newVal = map[i][1];
		USHORT scancode = KbLayoutGetQwertyScancode(newVal);
		USHORT position = KbLayoutGetQwertyScancode(oldVal);

		layout[position] = scancode;
		DbgPrint("mapping: %i -> %i\n", position, scancode);
	}

	for (i = 0; i < 200; i++){
		DbgPrint("mapped: %i -> %i\n", i, layout[i]);
	}
}

USHORT
KbLayoutGetQwertyScancode(
	char character
	)
{
	USHORT map[256]; // any ascii character
	map['`'] = 0x29;
	map['1'] = 0x02;
	map['2'] = 0x03;
	map['3'] = 0x04;
	map['4'] = 0x05;
	map['5'] = 0x06;
	map['6'] = 0x07;
	map['7'] = 0x08;
	map['8'] = 0x09;
	map['9'] = 0x0A;
	map['0'] = 0x0B;
	map['-'] = 0x0C;
	map['='] = 0x0D;

	map['q'] = 0x10;
	map['w'] = 0x11;
	map['e'] = 0x12;
	map['r'] = 0x13;
	map['t'] = 0x14;
	map['y'] = 0x15;
	map['u'] = 0x16;
	map['i'] = 0x17;
	map['o'] = 0x18;
	map['p'] = 0x19;
	map['['] = 0x1A;
	map[']'] = 0x1B;
	map['\\'] = 0x2b;

	map['a'] = 0x1E;
	map['s'] = 0x1F;
	map['d'] = 0x20;
	map['f'] = 0x21;
	map['g'] = 0x22;
	map['h'] = 0x23;
	map['j'] = 0x24;
	map['k'] = 0x25;
	map['l'] = 0x26;
	map[';'] = 0x27;
	map['\''] = 0x28;


	map['z'] = 0x2C;
	map['x'] = 0x2D;
	map['c'] = 0x2E;
	map['v'] = 0x2F;
	map['b'] = 0x30;
	map['n'] = 0x31;
	map['m'] = 0x32;
	map[','] = 0x33;
	map['.'] = 0x34;
	map['/'] = 0x35;

	return map[character];
}