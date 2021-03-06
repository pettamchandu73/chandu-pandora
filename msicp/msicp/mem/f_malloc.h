/* $Id: f_malloc.h,v 1.1.1.1 2006/08/31 22:40:53 hari Exp $
 *
 * simple, very fast, malloc library
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
/*
 * History:
 * --------
 *  2003-05-21  on sparc64 roundto 8 even in debugging mode (so malloc'ed
 *               long longs will be 64 bit aligned) (andrei)
 *  2004-07-19  support for 64 bit (2^64 mem. block) and more info
 *               for the future de-fragmentation support (andrei)
 *  2004-11-10  support for > 4Gb mem., switched to long (andrei)
 */


#if !defined(f_malloc_h) && !defined(VQ_MALLOC) 
#define f_malloc_h



/* defs*/

#ifdef DBG_F_MALLOC
#if defined(__CPU_sparc64) || defined(__CPU_sparc)
/* tricky, on sun in 32 bits mode long long must be 64 bits aligned
 * but long can be 32 bits aligned => malloc should return long long
 * aligned memory */
	#define ROUNDTO		sizeof(long long)
#else
	#define ROUNDTO		sizeof(void*) /* size we round to, must be = 2^n, and
                      sizeof(fm_frag) must be multiple of ROUNDTO !*/
#endif
#else /* DBG_F_MALLOC */
	#define ROUNDTO 8UL
#endif
#define MIN_FRAG_SIZE	ROUNDTO



#define F_MALLOC_OPTIMIZE_FACTOR 14UL /*used below */
#define F_MALLOC_OPTIMIZE  (1UL<<F_MALLOC_OPTIMIZE_FACTOR)
								/* size to optimize for,
									(most allocs <= this size),
									must be 2^k */

#define F_HASH_SIZE (F_MALLOC_OPTIMIZE/ROUNDTO + \
		(sizeof(long)*8-F_MALLOC_OPTIMIZE_FACTOR)+1)

/* hash structure:
 * 0 .... F_MALLOC_OPTIMIZE/ROUNDTO  - small buckets, size increases with
 *                            ROUNDTO from bucket to bucket
 * +1 .... end -  size = 2^k, big buckets */

struct fm_frag{
	unsigned long size;
	union{
		struct fm_frag* nxt_free;
		long reserved;
	}u;
#ifdef DBG_F_MALLOC
	const char* file;
	const char* func;
	unsigned long line;
	unsigned long check;
#endif
};

struct fm_frag_lnk{
	struct fm_frag* first;
	unsigned long no;
};

struct fm_block{
	unsigned long size; /* total size */
#ifdef DBG_F_MALLOC
	unsigned long used; /* alloc'ed size*/
	unsigned long real_used; /* used+malloc overhead*/
	unsigned long max_real_used;
#endif
	
	struct fm_frag* first_frag;
	struct fm_frag* last_frag;
	
	struct fm_frag_lnk free_hash[F_HASH_SIZE];
};



struct fm_block* fm_malloc_init(char* address, unsigned long size);

#ifdef DBG_F_MALLOC
void* fm_malloc(struct fm_block*, unsigned long size,
					const char* file, const char* func, unsigned int line);
#else
void* fm_malloc(struct fm_block*, unsigned long size);
#endif

#ifdef DBG_F_MALLOC
void  fm_free(struct fm_block*, void* p, const char* file, const char* func, 
				unsigned int line);
#else
void  fm_free(struct fm_block*, void* p);
#endif

#ifdef DBG_F_MALLOC
void*  fm_realloc(struct fm_block*, void* p, unsigned long size, 
					const char* file, const char* func, unsigned int line);
#else
void*  fm_realloc(struct fm_block*, void* p, unsigned long size);
#endif

void  fm_status(struct fm_block*);


#endif
