/*
 * $Id: tsend.h,v 1.1.1.1 2006/08/31 22:40:44 hari Exp $
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
 *  History:
 * --------
 *  2004-02-26  created by andrei
 */

#ifndef __tsend_h
#define __tsend_h


int tsend_stream(int fd, char* buf, unsigned int len, int timeout);
int tsend_dgram(int fd, char* buf, unsigned int len, 
				const struct sockaddr* to, socklen_t tolen, int timeout);
int tsend_dgram_ev(int fd, const struct iovec* v, int count, int timeout);



#endif


