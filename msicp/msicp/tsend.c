/*
 * $Id: tsend.c,v 1.1.1.1 2006/08/31 22:40:44 hari Exp $
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
 * send with timeout for stream and datagram sockets
 * 
 * History:
 * --------
 *  2004-02-26  created by andrei
 *  2003-03-03  switched to heavy macro use, added tsend_dgram_ev (andrei) 
 */

#include <string.h>
#include <errno.h>
#include <sys/poll.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>

#include "dprint.h"

/* the functions below are very similar => some generic macros */
#define TSEND_INIT \
	int n; \
	struct pollfd pf; \
	pf.fd=fd; \
	pf.events=POLLOUT

#define TSEND_POLL(f_name) \
poll_loop: \
	while(1){ \
		n=poll(&pf, 1, timeout); \
		if (n<0){ \
			if (errno==EINTR) continue; /* signal, ignore */ \
			LOG(L_ERR, "ERROR: " f_name ": poll failed: %s [%d]\n", \
					strerror(errno), errno); \
			goto error; \
		}else if (n==0){ \
			/* timeout */ \
			LOG(L_ERR, "ERROR: " f_name ": send timeout (%d)\n", timeout); \
			goto error; \
		} \
		if (pf.revents&POLLOUT){ \
			/* we can write again */ \
			goto again; \
		}else if (pf.revents&(POLLERR|POLLHUP|POLLNVAL)){ \
			LOG(L_ERR, "ERROR: " f_name ": bad poll flags %x\n", \
					pf.revents); \
			goto error; \
		} \
		/* if POLLIN or POLLPRI or other non-harmful events happened,    \
		 * continue ( although poll should never signal them since we're  \
		 * not interested in them => we should never reach this point) */ \
	} 


#define TSEND_ERR_CHECK(f_name)\
	if (n<0){ \
		if (errno==EINTR) goto again; \
		else if (errno!=EAGAIN && errno!=EWOULDBLOCK){ \
			LOG(L_ERR, "ERROR: " f_name ": failed to send: (%d) %s\n", \
					errno, strerror(errno)); \
			goto error; \
		}else goto poll_loop; \
	}
	


/* sends on fd (which must be O_NONBLOCK); if it cannot send any data
 * in timeout milliseconds it will return ERROR
 * returns: -1 on error, or number of bytes written
 *  (if less than len => couldn't send all)
 *  bugs: signals will reset the timer
 */
int tsend_stream(int fd, char* buf, unsigned int len, int timeout)
{
	int written;
	TSEND_INIT;
	
	written=0;
again:
	n=send(fd, buf, len,
#ifdef HAVE_MSG_NOSIGNAL
			MSG_NOSIGNAL
#else
			0
#endif
		);
	TSEND_ERR_CHECK("tsend_stream");
	written+=n; 
	if (n<len){ 
		/* partial write */ 
		buf+=n; 
		len-=n; 
	}else{ 
		/* successful full write */ 
		return written;
	}
	TSEND_POLL("tsend_stream");
error:
	return -1;
}



/* sends on dgram fd (which must be O_NONBLOCK); if it cannot send any data
 * in timeout milliseconds it will return ERROR
 * returns: -1 on error, or number of bytes written
 *  (if less than len => couldn't send all)
 *  bugs: signals will reset the timer
 */
int tsend_dgram(int fd, char* buf, unsigned int len, 
				const struct sockaddr* to, socklen_t tolen, int timeout)
{
	TSEND_INIT;
again:
	n=sendto(fd, buf, len, 0, to, tolen);
	TSEND_ERR_CHECK("tsend_dgram");
	/* we don't care about partial writes: they shouldn't happen on 
	 * a datagram socket */
	return n;
	TSEND_POLL("tsend_datagram");
error:
	return -1;
}

	
/* sends on connected datagram fd (which must be O_NONBLOCK); 
 * if it cannot send any data in timeout milliseconds it will return ERROR
 * returns: -1 on error, or number of bytes written
 *  (if less than len => couldn't send all)
 *  bugs: signals will reset the timer
 */

int tsend_dgram_ev(int fd, const struct iovec* v, int count, int timeout)
{
	TSEND_INIT;
again:
	n=writev(fd, v, count);
	TSEND_ERR_CHECK("tsend_datagram_ev");
	return n;
	TSEND_POLL("tsend_datagram_ev");
error:
	return -1;
}

