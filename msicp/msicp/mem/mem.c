/*
 * $Id: mem.c,v 1.1.1.1 2006/08/31 22:40:53 hari Exp $
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
 *
 * History:
 * --------
 *  2003-04-08  init_mallocs split into init_{pkg,shm}_malloc (andrei)
 * 
 */


#include <stdio.h>
#include "../config.h"
#include "../dprint.h"
#include "../globals.h"
#include "mem.h"

#ifdef PKG_MALLOC
	#ifdef VQ_MALLOC
		#include "vq_malloc.h"
	#else
		#include "q_malloc.h"
	#endif
#endif

#ifdef SHM_MEM
#include "shm_mem.h"
#endif

#ifdef PKG_MALLOC
	char *mem_pool = 0;
	#ifdef VQ_MALLOC
		struct vqm_block* mem_block;
	#elif defined F_MALLOC
		struct fm_block* mem_block;
	#else
		struct qm_block* mem_block;
	#endif
#endif


int init_pkg_mallocs()
{
#ifdef PKG_MALLOC
	/*init mem*/
	if (pkg_mem_size == 0)
		pkg_mem_size = PKG_MEM_POOL_SIZE;
	mem_pool = malloc(pkg_mem_size);

	#ifdef VQ_MALLOC
		if(mem_pool)
			mem_block=vqm_malloc_init(mem_pool, pkg_mem_size);
	#elif F_MALLOC
		LOG(L_CRIT, "PKG MEMORY SIZE : %ld \n",pkg_mem_size);
		if(mem_pool)
			mem_block=fm_malloc_init(mem_pool, pkg_mem_size);
	#else
		if(mem_pool)
			mem_block=qm_malloc_init(mem_pool, pkg_mem_size);
	#endif
	if (mem_block==0){
		LOG(L_CRIT, "could not initialize memory pool. Too much pkg memory demanded: %ld \n",pkg_mem_size);
		return -1;
	}
#endif
	return 0;
}



int init_shm_mallocs()
{
#ifdef SHM_MEM
	if (shm_mem_init()<0) {
		LOG(L_CRIT, "could not initialize shared memory pool, exiting...\n");
		 fprintf(stderr, "Too much shared memory demanded: %ld\n",
			shm_mem_size );
		return -1;
	}
#endif
	return 0;
}

void destroy_pkg_mallocs(void)
{
#ifdef PKG_MALLOC
	if (mem_pool) {
		free(mem_pool);
		mem_pool = NULL;
	}
#endif /* PKG_MALLOC */
}

