/*
 * $Id: hash_func.c,v 1.1.1.1 2006/08/31 22:40:48 hari Exp $
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



#ifndef _CRC_H_
#define _CRC_H_

extern unsigned long int crc_32_tab[];
extern unsigned short int ccitt_tab[];
extern unsigned short int crc_16_tab[];

#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hash_func.h"
#include "dprint.h"
#include "crc.h"
#include "ut.h"


int new_hash( str call_id, str cseq_nr )
{
	int hash_code = 0;
	int i,j, k, third;
	int ci_len, cs_len;
	char *ci, *cs;

	/* trim EoLs */
/*
	ci_len = call_id.len;
	while (ci_len && ((c=call_id.s[ci_len-1])==0 || c=='\r' || c=='\n'))
		ci_len--;
	cs_len = cseq_nr.len;
	while (cs_len && ((c=cseq_nr.s[cs_len-1])==0 || c=='\r' || c=='\n'))
		cs_len--;
*/
	trim_len( ci_len, ci, call_id );
	trim_len( cs_len, cs, cseq_nr );

	/* run the cycle from the end ... we are interested in the
	   most-right digits ... and just take the %10 value of it
	*/
	third=(ci_len-1)/3;
	for ( i=ci_len-1, j=2*third, k=third;
		k>0 ; i--, j--, k-- ) {
		hash_code+=crc_16_tab[(unsigned char)(*(ci+i)) /*+7*/ ]+
			ccitt_tab[*(ci+k)+63]+
			ccitt_tab[*(ci+j)+13];
	}
	for( i=0 ; i<cs_len ; i++ )
		//hash_code+=crc_32_tab[(cseq_nr.s[i]+hash_code)%243];
		hash_code+=ccitt_tab[*(cs+i)+123];

	/* hash_code conditioning */
#ifdef _BUG
	/* not flat ... % 111b has shorter period than
       & 111b by one and results in different distribution;
	   ( 7 % 7 == 0, 7 %7 == 1 )
 	   % is used as a part of the hash function too, not only
	   for rounding; & is not flat; whoever comes up with
	   a nicer flat hash function which does not take
	   costly division is welcome; feel free to verify
	   distribution using hashtest()
    */
	hash_code &= (TABLE_ENTRIES-1); /* TABLE_ENTRIES = 2^k */
#endif
	hash_code=hash_code%(TABLE_ENTRIES-1)+1;
   	return hash_code;
}



int new_hash2( str call_id, str cseq_nr )
{
#define h_inc h+=v^(v>>3)
	char* p;
	register unsigned v;
	register unsigned h;
	
	h=0;
	
	
	for (p=call_id.s; p<=(call_id.s+call_id.len-4); p+=4){
		v=(*p<<24)+(p[1]<<16)+(p[2]<<8)+p[3];
		h_inc;
	}
	v=0;
	for (;p<(call_id.s+call_id.len); p++){ v<<=8; v+=*p;}
	h_inc;
	
	for (p=cseq_nr.s; p<=(cseq_nr.s+cseq_nr.len-4); p+=4){
		v=(*p<<24)+(p[1]<<16)+(p[2]<<8)+p[3];
		h_inc;
	}
	v=0;
	for (;p<(cseq_nr.s+cseq_nr.len); p++){ v<<=8; v+=*p;}
	h_inc;
	
	h=((h)+(h>>11))+((h>>13)+(h>>23));
	return (h)&(TABLE_ENTRIES-1);
}



void hashtest_cycle( int hits[TABLE_ENTRIES+5], char *ip )
{
	long int i,j,k, l;
	int  hashv;
	static char buf1[1024];
	static char buf2[1024];
	str call_id; 
	str cseq;

	call_id.s=buf1;
	cseq.s=buf2;

	for (i=987654328;i<987654328+10;i++)
		for (j=85296341;j<85296341+10;j++)
			for (k=987654;k<=987654+10;k++)
				for (l=101;l<201;l++) {
					call_id.len=sprintf( buf1, "%d-%d-%d@%s",(int)i,(int)j,
						(int)k, ip );
					cseq.len=sprintf( buf2, "%d", (int)l );
					/* printf("%s\t%s\n", buf1, buf2 ); */
					hashv=hash( call_id, cseq );
					hits[ hashv ]++;
				}
}

void hashtest()
{
	int hits[TABLE_ENTRIES+5];
	int i;

	memset( hits, 0, sizeof hits );
	hashtest_cycle( hits, "192.168.99.100" );
	hashtest_cycle( hits, "172.168.99.100" );
	hashtest_cycle( hits, "142.168.99.100" );
	for (i=0; i<TABLE_ENTRIES+5; i++)
		printf("[%d. %d]\n", i, hits[i] );
	exit(0);
}

