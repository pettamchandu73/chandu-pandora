/*
 * $Id: forward.c,v 1.1.1.1 2006/08/31 22:40:44 hari Exp $
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
 * -------
 *  2001-??-??  created by andrei
 *  ????-??-??  lots of changes by a lot of people
 *  2003-01-23  support for determination of outbound interface added :
 *               get_out_socket (jiri)
 *  2003-01-24  reply to rport support added, contributed by
 *               Maxim Sobolev <sobomax@FreeBSD.org> and modified by andrei
 *  2003-02-11  removed calls to upd_send & tcp_send & replaced them with
 *               calls to msg_send (andrei)
 *  2003-03-19  replaced all mallocs/frees w/ pkg_malloc/pkg_free (andrei)
 *  2003-04-02  fixed get_send_socket for tcp fwd to udp (andrei)
 *  2003-04-03  added su_setport (andrei)
 *  2003-04-04  update_sock_struct_from_via now differentiates between
 *               local replies  & "normal" replies (andrei)
 *  2003-04-12  update_sock_struct_from via uses also FL_FORCE_RPORT for
 *               local replies (andrei)
 *  2003-08-21  check_self properly handles ipv6 addresses & refs   (andrei)
 *  2003-10-21  check_self updated to handle proto (andrei)
 *  2003-10-24  converted to the new socket_info lists (andrei)
 *  2004-10-10  modified check_self to use grep_sock_info (andrei)
 *  2004-11-08  added force_send_socket support in get_send_socket (andrei)
 */



#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "forward.h"
#include "hash_func.h"
#include "config.h"
#include "parser/msg_parser.h"
#include "route.h"
#include "dprint.h"
#include "globals.h"
#include "data_lump.h"
#include "ut.h"
#include "mem/mem.h"
#include "msg_translator.h"
#include "sr_module.h"
#include "ip_addr.h"
#include "resolve.h"
#include "name_alias.h"
#include "socket_info.h"

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif



/* return a socket_info_pointer to the sending socket; as opposed to
 * get_send_socket, which returns process's default socket, get_out_socket
 * attempts to determine the outbound interface which will be used;
 * it creates a temporary connected socket to determine it; it will
 * be very likely noticeably slower, but it can deal better with
 * multihomed hosts
 */
struct socket_info* get_out_socket(union sockaddr_union* to, int proto)
{
	int temp_sock;
	socklen_t len;
	union sockaddr_union from; 
	struct socket_info* si;
	struct ip_addr ip;

	if (proto!=PROTO_UDP) {
		LOG(L_CRIT, "BUG: get_out_socket can only be called for UDP\n");
		return 0;
	}
	
	temp_sock=socket(to->s.sa_family, SOCK_DGRAM, 0 );
	if (temp_sock==-1) {
		LOG(L_ERR, "ERROR: get_out_socket: socket() failed: %s\n",
				strerror(errno));
		return 0;
	}
	if (connect(temp_sock, &to->s, sockaddru_len(*to))==-1) {
		LOG(L_ERR, "ERROR: get_out_socket: connect failed: %s\n",
				strerror(errno));
		goto error;
	}
	len=sizeof(from);
	if (getsockname(temp_sock, &from.s, &len)==-1) {
		LOG(L_ERR, "ERROR: get_out_socket: getsockname failed: %s\n",
				strerror(errno));
		goto error;
	}
	su2ip_addr(&ip, &from);
	si=find_si(&ip, 0, proto);
	if (si==0) goto error;
	close(temp_sock);
	DBG("DEBUG: get_out_socket: socket determined: %p\n", si );
	return si;
error:
	LOG(L_ERR, "ERROR: get_out_socket: no socket found\n");
	close(temp_sock);
	return 0;
}



/* returns a socket_info pointer to the sending socket or 0 on error
 * params: sip msg (can be null), destination socket_union pointer, protocol
 * if msg!=null and msg->force_send_socket, the force_send_socket will be
 * used
 */
struct socket_info* get_send_socket(struct sip_msg *msg, 
										union sockaddr_union* to, int proto)
{
	struct socket_info* send_sock;
	
	/* check if send interface is not forced */
	if (msg && msg->force_send_socket){
		if (msg->force_send_socket->proto!=proto){
			DBG("get_send_socket: force_send_socket of different proto"
					" (%d)!\n", proto);
			msg->force_send_socket=find_si(&(msg->force_send_socket->address),
											msg->force_send_socket->port_no,
											proto);
		}
		if (msg->force_send_socket && (msg->force_send_socket->socket!=-1)) 
			return msg->force_send_socket;
		else{
			if (msg->force_send_socket->socket==-1)
				LOG(L_WARN, "WARNING: get_send_socket: not listening"
						 " on the requested socket, no fork mode?\n");
			else
				LOG(L_WARN, "WARNING: get_send_socket: "
						"protocol/port mismatch\n");
		}
	};

	if (mhomed && proto==PROTO_UDP){
		send_sock=get_out_socket(to, proto);
		if ((send_sock==0) || (send_sock->socket!=-1))
			return send_sock; /* found or error*/
		else if (send_sock->socket==-1){
			LOG(L_WARN, "WARNING: get_send_socket: not listening on the"
					" requested socket, no fork mode?\n");
			/* continue: try to use some socket */
		}
	}

	send_sock=0;
	/* check if we need to change the socket (different address families -
	 * eg: ipv4 -> ipv6 or ipv6 -> ipv4) */
	switch(proto){
#ifdef USE_TCP
		case PROTO_TCP:
		/* on tcp just use the "main address", we don't really now the
		 * sending address (we can find it out, but we'll need also to see
		 * if we listen on it, and if yes on which port -> too complicated*/
			switch(to->s.sa_family){
				/* FIXME */
				case AF_INET:	send_sock=sendipv4_tcp;
								break;
#ifdef USE_IPV6
				case AF_INET6:	send_sock=sendipv6_tcp;
								break;
#endif
				default:	LOG(L_ERR, "get_send_socket: BUG: don't know how"
									" to forward to af %d\n", to->s.sa_family);
			}
			break;
#endif
#ifdef USE_TLS
		case PROTO_TLS:
			switch(to->s.sa_family){
				/* FIXME */
				case AF_INET:	send_sock=sendipv4_tls;
								break;
#ifdef USE_IPV6
				case AF_INET6:	send_sock=sendipv6_tls;
								break;
#endif
				default:	LOG(L_ERR, "get_send_socket: BUG: don't know how"
									" to forward to af %d\n", to->s.sa_family);
			}
			break;
#endif /* USE_TLS */
		case PROTO_UDP:
			if ((bind_address==0)||(to->s.sa_family!=bind_address->address.af)||
				  (bind_address->proto!=PROTO_UDP)){
				switch(to->s.sa_family){
					case AF_INET:	send_sock=sendipv4;
									break;
#ifdef USE_IPV6
					case AF_INET6:	send_sock=sendipv6;
									break;
#endif
					default:	LOG(L_ERR, "get_send_socket: BUG: don't know"
										" how to forward to af %d\n",
										to->s.sa_family);
				}
			}else send_sock=bind_address;
			break;
		default:
			LOG(L_CRIT, "BUG: get_send_socket: unknown proto %d\n", proto);
	}
	return send_sock;
}



/* checks if the proto: host:port is one of the address we listen on;
 * if port==0, the  port number is ignored
 * if proto==0 (PROTO_NONE) the protocol is ignored
 * returns 1 if true, 0 if false, -1 on error
 * WARNING: uses str2ip6 so it will overwrite any previous
 *  unsaved result of this function (static buffer)
 */
int check_self(str* host, unsigned short port, unsigned short proto)
{
	if (grep_sock_info(host, port, proto)) goto found;
	/* try to look into the aliases*/
	if (grep_aliases(host->s, host->len, port, proto)==0){
		DBG("check_self: host != me\n");
		return 0;
	}
found:
	return 1;
}



int forward_request( struct sip_msg* msg, struct proxy_l * p, int proto)
{
	unsigned int len;
	char* buf;
	union sockaddr_union* to;
	struct socket_info* send_sock;
	char md5[MD5_LEN];
	int id; /* used as branch for tcp! */
	
	to=0;
	buf=0;
	id=0;
	
	to=(union sockaddr_union*)pkg_malloc(sizeof(union sockaddr_union));
	if (to==0){
		ser_error=E_OUT_OF_MEM;
		LOG(L_ERR, "ERROR: forward_request: out of memory\n");
		goto error;
	}
	
	
	/* if error try next ip address if possible */
	if (p->ok==0){
		if (p->host.h_addr_list[p->addr_idx+1])
			p->addr_idx++;
		else p->addr_idx=0;
		p->ok=1;
	}
	
	hostent2su(to, &p->host, p->addr_idx, 
				(p->port)?p->port:SIP_PORT);
	p->tx++;
	p->tx_bytes+=len;
	

	send_sock=get_send_socket(msg, to, proto);
	if (send_sock==0){
		LOG(L_ERR, "forward_req: ERROR: cannot forward to af %d, proto %d "
				"no corresponding listening socket\n", to->s.sa_family, proto);
		ser_error=E_NO_SOCKET;
		goto error1;
	}

	/* calculate branch for outbound request;  if syn_branch is turned off,
	   calculate is from transaction key, i.e., as an md5 of From/To/CallID/
	   CSeq exactly the same way as TM does; good for reboot -- than messages
	   belonging to transaction lost due to reboot will still be forwarded
	   with the same branch parameter and will be match-able downstream

       if it is turned on, we don't care about reboot; we simply put a simple
	   value in there; better for performance
	*/
	if (syn_branch ) {
		*msg->add_to_branch_s='0';
		msg->add_to_branch_len=1;
	} else {
		if (!char_msg_val( msg, md5 )) 	{ /* parses transaction key */
			LOG(L_ERR, "ERROR: forward_request: char_msg_val failed\n");
			goto error1;
		}
		msg->hash_index=hash( msg->callid->body, get_cseq(msg)->number);
		if (!branch_builder( msg->hash_index, 0, md5, id /* 0-th branch */,
					msg->add_to_branch_s, &msg->add_to_branch_len )) {
			LOG(L_ERR, "ERROR: forward_request: branch_builder failed\n");
			goto error1;
		}
	}

	buf = build_req_buf_from_sip_req( msg, &len, send_sock,  proto);
	if (!buf){
		LOG(L_ERR, "ERROR: forward_request: building failed\n");
		goto error1;
	}
	 /* send it! */
	DBG("Sending:\n%.*s.\n", (int)len, buf);
	DBG("orig. len=%d, new_len=%d, proto=%d\n", msg->len, len, proto );
	
	if (msg_send(send_sock, proto, to, 0, buf, len)<0){
		ser_error=E_SEND;
		p->errors++;
		p->ok=0;
		STATS_TX_DROPS;
		goto error1;
	}
	
	/* sent requests stats */
	STATS_TX_REQUEST(  msg->first_line.u.request.method_value );
	
	pkg_free(buf);
	pkg_free(to);
	/* received_buf & line_buf will be freed in receive_msg by free_lump_list*/
	return 0;

error1:
	pkg_free(to);
error:
	if (buf) pkg_free(buf);
	return -1;
}



int update_sock_struct_from_via( union sockaddr_union* to,
								 struct sip_msg* msg,
								 struct via_body* via )
{
	struct hostent* he;
	str* name;
	int err;
	unsigned short port;

	port=0;
	if(via==msg->via1){ 
		/* _local_ reply, we ignore any rport or received value
		 * (but we will send back to the original port if rport is
		 *  present) */
		if ((msg->msg_flags&FL_FORCE_RPORT)||(via->rport))
			port=msg->rcv.src_port;
		else port=via->port;
		name=&(via->host); /* received=ip in 1st via is ignored (it's
							  not added by us so it's bad) */
	}else{
		/* "normal" reply, we use rport's & received value if present */
		if (via->rport && via->rport->value.s){
			DBG("update_sock_struct_from_via: using 'rport'\n");
			port=str2s(via->rport->value.s, via->rport->value.len, &err);
			if (err){
				LOG(L_NOTICE, "ERROR: forward_reply: bad rport value(%.*s)\n",
						via->rport->value.len, via->rport->value.s);
				port=0;
			}
		}
		if (via->received){
			DBG("update_sock_struct_from_via: using 'received'\n");
			name=&(via->received->value);
			/* making sure that we won't do SRV lookup on "received"
			 * (possible if no DNS_IP_HACK is used)*/
			if (port==0) port=via->port?via->port:SIP_PORT; 
		}else{
			DBG("update_sock_struct_from_via: using via host\n");
			name=&(via->host);
			if (port==0) port=via->port;
		}
	}
	/* we do now a malloc/memcpy because gethostbyname loves \0-terminated 
	   strings; -jiri 
	   but only if host is not null terminated
	   (host.s[len] will always be ok for a via)
	    BTW: when is via->host.s non null terminated? tm copy? - andrei 
	    Yes -- it happened on generating a 408 by TM; -jiri
	    sip_resolvehost now accepts str -janakj
	*/
	DBG("update_sock_struct_from_via: trying SRV lookup\n");
	he=sip_resolvehost(name, &port, via->proto);
	
	if (he==0){
		LOG(L_NOTICE, "ERROR:forward_reply:resolve_host(%.*s) failure\n",
				name->len, name->s);
		return -1;
	}
		
	hostent2su(to, he, 0, port);
	return 1;
}



/* removes first via & sends msg to the second */
int forward_reply(struct sip_msg* msg)
{
	char* new_buf;
	union sockaddr_union* to;
	unsigned int new_len;
	struct sr_module *mod;
	int proto;
	unsigned int id; /* used only by tcp*/
#ifdef USE_TCP
	char* s;
	int len;
#endif
	
	to=0;
	id=0;
	new_buf=0;
	/*check if first via host = us */
	if (check_via){
		if (check_self(&msg->via1->host,
					msg->via1->port?msg->via1->port:SIP_PORT,
					msg->via1->proto)!=1){
			LOG(L_NOTICE, "ERROR: forward_reply: host in first via!=me :"
					" %.*s:%d\n", msg->via1->host.len, msg->via1->host.s,
									msg->via1->port);
			/* send error msg back? */
			goto error;
		}
	}
	/* quick hack, slower for multiple modules*/
	for (mod=modules;mod;mod=mod->next){
		if ((mod->exports) && (mod->exports->response_f)){
			DBG("forward_reply: found module %s, passing reply to it\n",
					mod->exports->name);
			if (mod->exports->response_f(msg)==0) goto skip;
		}
	}

	/* we have to forward the reply stateless, so we need second via -bogdan*/
	if (parse_headers( msg, HDR_VIA2, 0 )==-1 
		|| (msg->via2==0) || (msg->via2->error!=PARSE_OK))
	{
		/* no second via => error */
		LOG(L_ERR, "ERROR: forward_reply: no 2nd via found in reply\n");
		goto error;
	}

	to=(union sockaddr_union*)pkg_malloc(sizeof(union sockaddr_union));
	if (to==0){
		LOG(L_ERR, "ERROR: forward_reply: out of memory\n");
		goto error;
	}

	new_buf = build_res_buf_from_sip_res( msg, &new_len);
	if (!new_buf){
		LOG(L_ERR, "ERROR: forward_reply: building failed\n");
		goto error;
	}

	proto=msg->via2->proto;
	if (update_sock_struct_from_via( to, msg, msg->via2 )==-1) goto error;


#ifdef USE_TCP
	if (proto==PROTO_TCP
#ifdef USE_TLS
			|| proto==PROTO_TLS
#endif
			){
		/* find id in i param if it exists */
		if (msg->via1->i&&msg->via1->i->value.s){
			s=msg->via1->i->value.s;
			len=msg->via1->i->value.len;
			DBG("forward_reply: i=%.*s\n",len, ZSW(s));
			if (reverse_hex2int(s, len, &id)<0){
				LOG(L_ERR, "ERROR: forward_reply: bad via i param \"%.*s\"\n",
						len, ZSW(s));
					id=0;
			}
			DBG("forward_reply: id= %x\n", id);
		}		
				
	} 
#endif
	if (msg_send(0, proto, to, (int)id, new_buf, new_len)<0) goto error;
#ifdef STATS
	STATS_TX_RESPONSE(  (msg->first_line.u.reply.statuscode/100) );
#endif

	DBG(" reply forwarded to %.*s:%d\n", 
			msg->via2->host.len, msg->via2->host.s,
			(unsigned short) msg->via2->port);

	pkg_free(new_buf);
	pkg_free(to);
skip:
	return 0;
error:
	if (new_buf) pkg_free(new_buf);
	if (to) pkg_free(to);
	return -1;
}
