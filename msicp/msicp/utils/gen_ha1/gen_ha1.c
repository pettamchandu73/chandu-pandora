/*
 * $Id: gen_ha1.c,v 1.1.1.1 2006/08/31 22:40:54 hari Exp $
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
 */


#include <stdio.h>
#include <stdlib.h>
#include "../../md5global.h"
#include "../../md5.h"
#include "calc.h"



int main(int argc, char** argv)
{
	HASHHEX ha1;

	if (argc != 4) {
		printf("Usage: gen_ha1 <username> <realm> <password> \n");
		/* return EXIT_SUCCESS; -jku */
		return 1;
	}

	DigestCalcHA1("md5", argv[1], argv[2], argv[3], "", "", ha1);
	printf("%s\n", ha1);
	
	return 0; /* jku */
}
