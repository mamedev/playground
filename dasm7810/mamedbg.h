/*****************************************************************************
 *  d7810 - an 7810 disassembler for the commandline
 *
 *  Copyright (C) 2002 Juergen Buchmueller <pullmoll@stop1984.com>
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
#ifndef _MAMEDBG_H
#define _MAMEDBG_H

#include "driver.h"

enum {
	EA_DST,
	EA_SRC
};

/* Size of the data element accessed (or the immediate value) */
enum {
	EA_DEFAULT,
	EA_INT8,
	EA_UINT8,
	EA_INT16,
	EA_UINT16,
	EA_INT32,
	EA_UINT32,
	EA_SIZE
};

/* Access modes for effective addresses to debug_ea_info */
enum {
	EA_NONE,        /* no EA mode */
	EA_VALUE,       /* immediate value */
	EA_ABS_PC,      /* change PC absolute (JMP or CALL type opcodes) */
	EA_REL_PC,      /* change PC relative (BRA or JR type opcodes) */
	EA_ZPG_RD,	/* read zero page memory */
	EA_ZPG_WR,	/* write zero page memory */
	EA_ZPG_RDWR,	/* read then write zero page memory */
	EA_MEM_RD,      /* read memory */
	EA_MEM_WR,      /* write memory */
	EA_MEM_RDWR,    /* read then write memory */
	EA_PORT_RD,     /* read i/o port */
	EA_PORT_WR,     /* write i/o port */
	EA_COUNT
};

/***************************************************************************
 * Convenience macro for the CPU cores, this is defined to empty
 * if MAME_DEBUG is not specified, so a CPU core can simply add
 * CALL_MAME_DEBUG; before executing an instruction
 ***************************************************************************/
#define CALL_MAME_DEBUG if( mame_debug ) MAME_Debug()

#ifndef DECL_SPEC
#define DECL_SPEC
#endif

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#ifndef INVALID
#define INVALID 0xffffffff
#endif

extern const char *set_ea_info(int what, unsigned value, int size, int access);

#endif
