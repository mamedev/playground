/*****************************************************************************
 *  d7810 - an 7810 disassembler for the commandline
 *
 *  Copyright (C) 2002 Juergen Buchmueller <pullmoll@t-online.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "mamedbg.h"
#include "upd7810.h"

#ifndef	MAXPATHLEN
#define	MAXPATHLEN	256
#endif

#ifndef	ABITS
#define	ABITS	16
#endif
#ifndef	AMASK
#define	AMASK	((1<<ABITS)-1)
#endif

static int dasm_relative_jumps = 0;
static unsigned dasm_ea[2];
static unsigned upd7810_pc;

UINT8 *rom = NULL;

UINT8 cpu_readop(UINT32 offs)
{
	return rom[offs];
}

UINT8 cpu_readop_arg(UINT32 offs)
{
	return rom[offs];
}

unsigned upd7810_get_reg(int which)
{
	switch (which) {
	case UPD7810_PC:
		return upd7810_pc;
	}
	return 0;	/* dummy: return zero */
}

const char *set_ea_info( int what, unsigned value, int size, int access )
{
	static char buffer[8][63+1];
	static int which = 0;
	const char *sign = "";
	unsigned width, result;

	which = (which+1) % 8;

	dasm_ea[what] = value;

	if (access == EA_REL_PC)
		/* PC relative calls set_ea_info with value = PC
		 * and size = offset
		 */
		result = value + size;
	else
		result = value;

	switch (access) {
	case EA_VALUE:	/* Immediate value */
		switch( size )
		{
		case EA_INT8:
			width = 2;
			result &= 0xff;
			if (result & 0x80) {
				sign = "-";
				result = (unsigned)-result;
			}
			break;
		case EA_INT16:
			width = 4;
			result &= 0xffff;
			if (result & 0x8000) {
				sign = "-";
				result = (unsigned)-result;
			}
			break;
		case EA_INT32:
			width = 8;
			if (result & 0x80000000) {
				sign = "-";
				result = (unsigned)-result;
			}
			break;
		case EA_UINT8:
			width = 2;
			result &= 0xff;
			break;
		case EA_UINT16:
			width = 4;
			result &= 0xffff;
			break;
		case EA_UINT32:
			width = 8;
			break;
		default:
			return "set_ea_info: invalid <size>!";
		}
		break;

	case EA_ZPG_RD:
	case EA_ZPG_WR:
	case EA_ZPG_RDWR:
		result &= 0xff;
		width = 2;
		break;

	case EA_ABS_PC: /* Absolute program counter change */
		result &= AMASK;
		if (size == EA_INT8 || size == EA_UINT8)
			width = 2;
		else
		if (size == EA_INT16 || size == EA_UINT16)
			width = 4;
		else
		if (size == EA_INT32 || size == EA_UINT32)
			width = 8;
		else
			width = (ABITS + 3) / 4;
		break;

	case EA_REL_PC: /* Relative program counter change */
		if (dasm_relative_jumps) {
			if (size == 0)
				return "$";
			if (size < 0) {
				sign = "-";
				result = (unsigned) -size;
			} else {
				sign = "+";
				result = (unsigned) size;
			}
			sprintf(buffer[which], "$%s%u", sign, result);
			return buffer[which];
		}
		/* fall through */
	default:
		result &= AMASK;
		width = (ABITS + 3) / 4;
	}
	sprintf(buffer[which], "%s$%0*X", sign, width, result);
	return buffer[which];
}

void usage(char **argv)
{
	fprintf(stderr, "usage: %s [options] filename[.ext]\n",
		argv[0]);
	fprintf(stderr, "options may be one or more of:\n");
	fprintf(stderr, "-b xxxx\tbase address for the disassembly\n");
	fprintf(stderr, "-r\tdisplay relative jump addresses (not absolute)\n");
}

int main(int argc, char **argv)
{
	char filename[MAXPATHLEN] = "";
	char dasm[256];
	FILE *fp;
	unsigned size, addr, offs = 0;
	int i;

	if (argc < 2) {
		usage(argv);
		return 1;
	}

	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
			case 'b':
				if (i + 1 >= argc) {
					usage(argv);
					return 1;
				}
				i++;
				offs = strtoul(argv[i], NULL, 0);
				break;
			case 'r':
				dasm_relative_jumps = 1;
				break;
			default:
				usage(argv);
				return 1;
			}
		} else if (filename[0] != '\0') {
			usage(argv);
			return 1;
		} else {
			strcpy(filename, argv[i]);
		}
	}

	fp = fopen(filename, "rb");
	if (NULL == fp) {
		fprintf(stderr, "Cannot open '%s' (%s)\n",
			filename, strerror(errno));
		return 1;
	}
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (0 == size) {
		fprintf(stderr, "File '%s' is emtpy!?\n",
			filename);
		return 1;
	}
	rom = malloc(size + offs);
	if (NULL == rom) {
		fprintf(stderr, "malloc(%d) call failed (%s)\n",
			size + offs, strerror(errno));
		return 1;
	}
	if (size != fread(&rom[offs], 1, size, fp)) {
		fprintf(stderr, "fread() call failed (%s)\n",
			strerror(errno));
		return 1;
	}
	fclose(fp);

	printf("Disassembly of %s, %d bytes [0x%x to 0x%x]\n",
		filename, size, offs, size + offs - 1);
	addr = offs;
	while (addr < size + offs) {
		unsigned op, length;

		upd7810_pc = addr;
		length = Dasm7810(dasm, addr);
		printf("%04x:", addr);
		for (op = 0; op < 4; op++) {
			if (op < length) {
				printf(" %02x", cpu_readop(addr+op));
			} else {
				printf("   ");
			}
		}
		printf(" %s\n", dasm);
		addr += length;
	}

	return 0;
}
