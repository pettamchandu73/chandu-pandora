/*
 * $Id: t_funcs.c,v 1.1.1.1 2006/08/31 22:40:48 hari Exp $
 *
 * transaction maintenance functions
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
 * -------
 *  2003-03-31  200 for INVITE/UAS resent even for UDP (jiri)
 *               info only if compiling w/ -DEXTRA_DEBUG (andrei)
 *  2003-03-19  replaced all mallocs/frees w/ pkg_malloc/pkg_free (andrei)
 *  2003-03-13  send_pr_buffer is called w/ file/function/line debugging
 *  2003-03-01  start_retr changed to retransmit only for UDP
 *  2003-02-13  modified send_pr_buffer to use msg_send & rb->dst (andrei)
 *  2003-04-14  use protocol from uri (jiri)
 *  2003-04-25  do it (^) really everywhere (jiri)
 *  2003-04-26  do it (^) really really really everywhere (jiri)
 *  2003-07-07  added get_proto calls when proxy!=0 (andrei)
 *  2004-02-13  t->is_invite and t->local replaced with flags (bogdan)
 *  2005-02-16  fr_*_timer acceps full AVP specifications; empty AVP
 *              desable variable timer feature (bogdan)
 */

#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include "../../dprint.h"
#include "../../hash_func.h"
#include "../../dset.h"
#include "../../mem/mem.h"
#include "defs.h"
#include "t_funcs.h"
#include "t_fwd.h"
#include "t_lookup.h"
#include "config.h"
#include "t_stats.h"

/* fr_timer AVP specs */
static int     fr_timer_avp_type = 0;
static int_str fr_timer_avp = /*(int_str)*/{ 0 };
static str     fr_timer_str;
static int     fr_inv_timer_avp_type = 0;
static int_str fr_inv_timer_avp = /*(int_str)*/{ 0 };
static str     fr_inv_timer_str;


/* ----------------------------------------------------- */
int send_pr_buffer(	struct retr_buf *rb, void *buf, int len
#ifdef EXTRA_DEBUG
						, char* file, char *function, int line
#endif
					)
{
	if (buf && len && rb )
		return msg_send( rb->dst.send_sock, rb->dst.proto, &rb->dst.to,
				         rb->dst.proto_reserved1, buf, len);
	else {
#ifdef EXTRA_DEBUG
		LOG(L_CRIT, "ERROR: send_pr_buffer: sending an empty buffer"
				"from %s: %s (%d)\n", file, function, line );
#else
		LOG(L_CRIT, "ERROR: send_pr_buffer: attempt to send an "
				"empty buffer\n");
#endif
		return -1;
	}
}

#ifdef _OBSOLETED
void start_retr( struct retr_buf *rb )
{
	if (rb->dst.proto==PROTO_UDP) {
		rb->retr_list=RT_T1_TO_1;
		set_timer( &rb->retr_timer, RT_T1_TO_1 );
	}
	set_timer( &rb->fr_timer, FR_TIMER_LIST );
}
#endif





void tm_shutdown()
{

	DBG("DEBUG: tm_shutdown : start\n");
	unlink_timer_lists();

	/* destroy the hash table */
	DBG("DEBUG: tm_shutdown : emptying hash table\n");
	free_hash_table( );
	DBG("DEBUG: tm_shutdown : releasing timers\n");
	free_timer_table();
	DBG("DEBUG: tm_shutdown : removing semaphores\n");
	lock_cleanup();
	DBG("DEBUG: tm_shutdown : destroying tmcb lists\n");
	destroy_tmcb_lists();
	free_tm_stats();
	DBG("DEBUG: tm_shutdown : done\n");
}


/*   returns 1 if everything was OK or -1 for error
*/
int t_release_transaction( struct cell *trans )
{
	set_kr(REQ_RLSD);

	reset_timer( & trans->uas.response.fr_timer );
	reset_timer( & trans->uas.response.retr_timer );

	cleanup_uac_timers( trans );
	
	put_on_wait( trans );
	return 1;
}


/* -----------------------HELPER FUNCTIONS----------------------- */


/*
  */
void put_on_wait(  struct cell  *Trans  )
{

#ifdef EXTRA_DEBUG
	DBG("DEBUG: put on WAIT \n");
#endif


	/* we put the transaction on wait timer; we do it only once
	   in transaction's timelife because putting it multiple-times
	   might result in a second instance of a wait timer to be
	   set after the first one fired; on expiration of the second
	   instance, the transaction would be re-deleted

			PROCESS1		PROCESS2		TIMER PROCESS
		0. 200/INVITE rx;
		   put_on_wait
		1.					200/INVITE rx;
		2.									WAIT fires; transaction
											about to be deleted
		3.					avoid putting
							on WAIT again
		4.									WAIT timer executed,
											transaction deleted
	*/
	set_1timer( &Trans->wait_tl, WT_TIMER_LIST, 0 );
}



static int kill_transaction( struct cell *trans )
{
	char err_buffer[128];
	int sip_err;
	int reply_ret;
	int ret;

	/*  we reply statefully and enter WAIT state since error might
		have occurred in middle of forking and we do not
		want to put the forking burden on upstream client;
		however, it may fail too due to lack of memory */

	ret=err2reason_phrase( ser_error, &sip_err,
		err_buffer, sizeof(err_buffer), "TM" );
	if (ret>0) {
		reply_ret=t_reply( trans, trans->uas.request, 
			sip_err, err_buffer);
		/* t_release_transaction( T ); */
		return reply_ret;
	} else {
		LOG(L_ERR, "ERROR: kill_transaction: err2reason failed\n");
		return -1;
	}
}



int t_relay_to( struct sip_msg  *p_msg , struct proxy_l *proxy, int proto,
				int replicate)
{
	int ret;
	int new_tran;
	str *uri;
	int reply_ret;
	/* struct hdr_field *hdr; */
	struct cell *t;
#ifdef ACK_FORKING_HACK
	str ack_uri;
	str backup_uri;
#endif

	ret=0;

	new_tran = t_newtran( p_msg );
	

	/* parsing error, memory alloc, whatever ... if via is bad
	   and we are forced to reply there, return with 0 (->break),
	   pass error status otherwise
	*/
	if (new_tran<0) {
		ret = (ser_error==E_BAD_VIA && reply_to_via) ? 0 : new_tran;
		goto done;
	}
	/* if that was a retransmission, return we are happily done */
	if (new_tran==0) {
		ret = 1;
		goto done;
	}

	/* new transaction */

	/* ACKs do not establish a transaction and are fwd-ed statelessly */
	if ( p_msg->REQ_METHOD==METHOD_ACK) {
		DBG( "SER: forwarding ACK  statelessly \n");
		if (proxy==0) {
			uri = GET_RURI(p_msg);
			proxy=uri2proxy(GET_NEXT_HOP(p_msg), proto);
			if (proxy==0) {
					ret=E_BAD_ADDRESS;
					goto done;
			}
			proto=proxy->proto; /* uri2proxy set it correctly */
			ret=forward_request( p_msg , proxy, proto) ;
			free_proxy( proxy );	
			pkg_free( proxy );
#ifdef ACK_FORKING_HACK
			backup_uri=p_msg->new_uri;
			init_branch_iterator();
			while((ack_uri.s=next_branch(&ack_uri.len))) {
				p_msg->new_uri=ack_uri;
				proxy=uri2proxy(GET_NEXT_HOP(p_msg), proto);
				if (proxy==0) continue;
				forward_request(p_msg, proxy, proxy->proto); /* ok, uri2proxy*/
				free_proxy( proxy );	
				pkg_free( proxy );
			}
			p_msg->new_uri=backup_uri;
#endif
		} else {
			proto=get_proto(proto, proxy->proto);
			ret=forward_request( p_msg , proxy, proto ) ;
#ifdef ACK_FORKING_HACK
			backup_uri=p_msg->new_uri;
			init_branch_iterator();
			while((ack_uri.s=next_branch(&ack_uri.len))) {
				p_msg->new_uri=ack_uri;
				forward_request(p_msg, proxy, proto);
			}
			p_msg->new_uri=backup_uri;
#endif
		}
		goto done;
	}

	/* if replication flag is set, mark the transaction as local
	   so that replies will not be relaied */
	t=get_t();
	if (replicate) t->flags|=T_IS_LOCAL_FLAG;

	/* INVITE processing might take long, particularly because of DNS
	   look-ups -- let upstream know we're working on it */
	if (p_msg->REQ_METHOD==METHOD_INVITE )
	{
		DBG( "SER: new INVITE\n");
		if (!t_reply( t, p_msg , 100 ,
			"trying -- your call is important to us"))
				DBG("SER: ERROR: t_reply (100)\n");
	} 

	/* now go ahead and forward ... */
	ret=t_forward_nonack(t, p_msg, proxy, proto);
	if (ret<=0) {
		DBG( "ERROR:tm:t_relay_to:  t_forward_nonack returned error \n");
		/* we don't want to pass upstream any reply regarding replicating
		 * a request; replicated branch must stop at us*/
		if (!replicate) {
			reply_ret=kill_transaction( t );
			if (reply_ret>0) {
				/* we have taken care of all -- do nothing in
			  	script */
				DBG("ERROR: generation of a stateful reply "
					"on error succeeded\n");
				ret=0;
			}  else {
				DBG("ERROR: generation of a stateful reply "
					"on error failed\n");
			}
		}
	} else {
		DBG( "SER: new transaction fwd'ed\n");
	}

done:
	return ret;
}



/*
 * Initialize parameters containing the ID of
 * AVPs with variable timers
 */
int init_avp_params(char *fr_timer_param, char *fr_inv_timer_param)
{
	if (fr_timer_param && *fr_timer_param) {
		fr_timer_str.s = fr_timer_param;
		fr_timer_str.len = strlen(fr_timer_str.s);
		if (parse_avp_spec( &fr_timer_str, &fr_timer_avp_type,
		&fr_timer_avp)<0) {
			LOG(L_CRIT,"ERROR:tm:init_avp_params: invalid fr_timer "
				"AVP specs \"%s\"\n", fr_timer_param);
			return -1;
		}
	}

	if (fr_inv_timer_param && *fr_inv_timer_param) {
		fr_inv_timer_str.s = fr_inv_timer_param;
		fr_inv_timer_str.len = strlen(fr_inv_timer_str.s);
		if (parse_avp_spec( &fr_inv_timer_str, &fr_inv_timer_avp_type, 
		&fr_inv_timer_avp)<0) {
			LOG(L_CRIT,"ERROR:tm:init_avp_params: invalid fr_inv_timer "
				"AVP specs \"%s\"\n", fr_inv_timer_param);
			return -1;
		}
	}
	return 0;
}


/*
 * Get the FR_{INV}_TIMER from corresponding AVP
 */
static inline int avp2timer(unsigned int* timer, int type, int_str name)
{
	struct usr_avp *avp;
	int_str val_istr;
	int err;

	avp = search_first_avp( type, name, &val_istr);
	if (!avp) {
		/*
		 DBG("avp2timer: AVP '%.*s' not found\n", param.s->len, ZSW(param.s->s));
		 */
		return 1;
	}
	
	if (avp->flags & AVP_VAL_STR) {
		*timer = str2s(val_istr.s->s, val_istr.s->len, &err);
		if (err) {
			LOG(L_ERR, "avp2timer: Error while converting string to integer\n");
			return -1;
		}
	} else {
		*timer = val_istr.n;
	}

	return 0;
}


int fr_avp2timer(unsigned int* timer)
{
	if (fr_timer_avp.n!=0)
		return avp2timer( timer, fr_timer_avp_type, fr_timer_avp);
	else
		return 1;
}


int fr_inv_avp2timer(unsigned int* timer)
{
	if (fr_inv_timer_avp.n!=0)
		return avp2timer( timer, fr_inv_timer_avp_type, fr_inv_timer_avp);
	else
		return 1;
}


