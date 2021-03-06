/* 
 * $Id: case_unsu.h,v 1.1.1.1 2006/08/31 22:40:56 hari Exp $ 
 *
 * Unsupported Header Field Name Parsing Macros
 *
 * Copyright (C) 2001-2003 FhG Fokus
 *
 * This file is part of msicp, a free SIP server.
 *
 * msicp is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version
 *
 * For a license to use the msicp software under conditions
 * other than those described here, or to purchase support for this
 * software, please contact iptel.org by e-mail at the following addresses:
 *    info@iptel.org
 *
 * msicp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * History:
 * --------
 * 2003-02-28 scratchpad compatibility abandoned (jiri)
 * 2003-01-27 next baby-step to removing ZT - PRESERVE_ZT (jiri)
 */


#ifndef CASE_UNSU_H
#define CASE_UNSU_H

#include "../comp_defs.h"

#define TED_CASE                             \
        switch(LOWER_DWORD(val)) {           \
        case _ted1_:                         \
                hdr->type = HDR_UNSUPPORTED; \
                hdr->name.len = 11;          \
	        return (p + 4);              \
                                             \
        case _ted2_:                         \
                hdr->type = HDR_UNSUPPORTED; \
                p += 4;                      \
	        goto dc_end;                 \
        }


#define PPOR_CASE                  \
        switch(LOWER_DWORD(val)) { \
        case _ppor_:               \
                p += 4;            \
                val = READ(p);     \
                TED_CASE;          \
                goto other;        \
        }


#define unsu_CASE         \
        p += 4;           \
        val = READ(p);    \
        PPOR_CASE;        \
        goto other;       \


#endif /* CASE_UNSU_H */
