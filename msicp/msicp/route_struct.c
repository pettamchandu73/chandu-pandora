/*
 * $Id: route_struct.c,v 1.1.1.1 2006/08/31 22:40:44 hari Exp $
 *
 * route structures helping functions
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
/* History:
 * --------
 *  2003-01-29  src_port introduced (jiri)
 *  2003-03-19  replaced all mallocs/frees w/ pkg_malloc/pkg_free (andrei)
 *  2003-04-12  FORCE_RPORT_T added (andrei)
 *  2003-10-02  added SET_ADV_ADDRESS & SET_ADV_PORT (andrei)
 *  2004-02-24  added LOAD_AVP_T and AVP_TO_URI_T (bogdan)
 */



#include  "route_struct.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dprint.h"
#include "ip_addr.h"
#include "mem/mem.h"
#include "ut.h" /* ZSW() */


struct expr* mk_exp(int op, struct expr* left, struct expr* right)
{
	struct expr * e;
	e=(struct expr*)pkg_malloc(sizeof (struct expr));
	if (e==0) goto error;
	e->type=EXP_T;
	e->op=op;
	e->l.expr=left;
	e->r.expr=right;
	return e;
error:
	LOG(L_CRIT, "ERROR: mk_exp: memory allocation failure\n");
	return 0;
}


struct expr* mk_elem(int op, int subtype, int operand, void* param)
{
	struct expr * e;
	e=(struct expr*)pkg_malloc(sizeof (struct expr));
	if (e==0) goto error;
	e->type=ELEM_T;
	e->op=op;
	e->subtype=subtype;
	e->l.operand=operand;
	e->r.param=param;
	return e;
error:
	LOG(L_CRIT, "ERROR: mk_elem: memory allocation failure\n");
	return 0;
}



struct action* mk_action(int type, int p1_type, int p2_type,
											void* p1, void* p2)
{
	struct action* a;
	a=(struct action*)pkg_malloc(sizeof(struct action));
	if (a==0) goto  error;
	memset(a,0,sizeof(struct action));
	a->type=type;
	a->p1_type=p1_type;
	a->p2_type=p2_type;
	a->p1.string=(char*) p1;
	a->p2.string=(char*) p2;
	a->next=0;
	return a;
	
error:
	LOG(L_CRIT, "ERROR: mk_action: memory allocation failure\n");
	return 0;

}


struct action* mk_action3(int type, int p1_type, int p2_type, int p3_type,
							void* p1, void* p2, void* p3)
{
	struct action* a;

	a=mk_action(type, p1_type, p2_type, p1, p2);
	if (a){
			a->p3_type=p3_type;
			a->p3.data=p3;
	}
	return a;
}



struct action* append_action(struct action* a, struct action* b)
{
	struct action *t;
	if (b==0) return a;
	if (a==0) return b;
	
	for(t=a;t->next;t=t->next);
	t->next=b;
	return a;
}



void print_expr(struct expr* exp)
{
	if (exp==0){
		LOG(L_CRIT, "ERROR: print_expr: null expression!\n");
		return;
	}
	if (exp->type==ELEM_T){
		switch(exp->l.operand){
			case METHOD_O:
				DBG("method");
				break;
			case URI_O:
				DBG("uri");
				break;
			case FROM_URI_O:
				DBG("from_uri");
				break;
			case TO_URI_O:
				DBG("to_uri");
				break;
			case SRCIP_O:
				DBG("srcip");
				break;
			case SRCPORT_O:
				DBG("srcport");
				break;
			case DSTIP_O:
				DBG("dstip");
				break;
			case DSTPORT_O:
				DBG("dstport");
				break;
			case NUMBER_O:
				break;
			case ACTION_O:
				break;
			default:
				DBG("UNKNOWN");
		}
		switch(exp->op){
			case EQUAL_OP:
				DBG("==");
				break;
			case MATCH_OP:
				DBG("=~");
				break;
			case NO_OP:
				break;
			case GT_OP:
				DBG(">");
				break;
			case GTE_OP:
				DBG(">=");
				break;
			case LT_OP:
				DBG("<");
				break;
			case LTE_OP:
				DBG("<=");
				break;
			case DIFF_OP:
				DBG("!=");
				break;
			default:
				DBG("<UNKNOWN>");
		}
		switch(exp->subtype){
			case NOSUBTYPE: 
					DBG("N/A");
					break;
			case STRING_ST:
					DBG("\"%s\"", ZSW((char*)exp->r.param));
					break;
			case NET_ST:
					print_net((struct net*)exp->r.param);
					break;
			case IP_ST:
					print_ip("", (struct ip_addr*)exp->r.param, "");
					break;
			case ACTIONS_ST:
					print_action((struct action*)exp->r.param);
					break;
			case NUMBER_ST:
					DBG("%d",exp->r.intval);
					break;
			case MYSELF_ST:
					DBG("_myself_");
					break;
			default:
					DBG("type<%d>", exp->subtype);
		}
	}else if (exp->type==EXP_T){
		switch(exp->op){
			case AND_OP:
					DBG("AND( ");
					print_expr(exp->l.expr);
					DBG(", ");
					print_expr(exp->r.expr);
					DBG(" )");
					break;
			case OR_OP:
					DBG("OR( ");
					print_expr(exp->l.expr);
					DBG(", ");
					print_expr(exp->r.expr);
					DBG(" )");
					break;
			case NOT_OP:	
					DBG("NOT( ");
					print_expr(exp->l.expr);
					DBG(" )");
					break;
			default:
					DBG("UNKNOWN_EXP ");
		}
					
	}else{
		DBG("ERROR:print_expr: unknown type\n");
	}
}
					

					

void print_action(struct action* a)
{
	struct action* t;
	for(t=a; t!=0;t=t->next){
		switch(t->type){
			case FORWARD_T:
					DBG("forward(");
					break;
			case FORWARD_TCP_T:
					DBG("forward_tcp(");
					break;
			case FORWARD_UDP_T:
					DBG("forward_udp(");
					break;
			case SEND_T:
					DBG("send(");
					break;
			case SEND_TCP_T:
					DBG("send_tcp(");
					break;
			case DROP_T:
					DBG("drop(");
					break;
			case LOG_T:
					DBG("log(");
					break;
			case ERROR_T:
					DBG("error(");
					break;
			case ROUTE_T:
					DBG("route(");
					break;
			case EXEC_T:
					DBG("exec(");
					break;
			case REVERT_URI_T:
					DBG("revert_uri(");
					break;
			case STRIP_T:
					DBG("strip(");
					break;
			case APPEND_BRANCH_T:
					DBG("append_branch(");
					break;
			case PREFIX_T:
					DBG("prefix(");
					break;
			case LEN_GT_T:
					DBG("len_gt(");
					break;
			case SETFLAG_T:
					DBG("setflag(");
					break;
			case RESETFLAG_T:
					DBG("resetflag(");
					break;
			case ISFLAGSET_T:
					DBG("isflagset(");
					break;
			case SET_HOST_T:
					DBG("sethost(");
					break;
			case SET_HOSTPORT_T:
					DBG("sethostport(");
					break;
			case SET_USER_T:
					DBG("setuser(");
					break;
			case SET_USERPASS_T:
					DBG("setuserpass(");
					break;
			case SET_PORT_T:
					DBG("setport(");
					break;
			case SET_URI_T:
					DBG("seturi(");
					break;
			case IF_T:
					DBG("if (");
					break;
			case MODULE_T:
					DBG(" external_module_call(");
					break;
			case FORCE_RPORT_T:
					DBG("force_rport(");
					break;
			case SET_ADV_ADDR_T:
					DBG("set_advertised_address(");
					break;
			case SET_ADV_PORT_T:
					DBG("set_advertised_port(");
					break;
			case FORCE_TCP_ALIAS_T:
					DBG("force_tcp_alias(");
					break;
			case LOAD_AVP_T:
					DBG("load_avp(");
					break;
			case AVP_TO_URI_T:
					DBG("avp_to_attr");
					break;
			case FORCE_SEND_SOCKET_T:
					DBG("force_send_socket");
					break;
			default:
					DBG("UNKNOWN(");
		}
		switch(t->p1_type){
			case STRING_ST:
					DBG("\"%s\"", ZSW(t->p1.string));
					break;
			case NUMBER_ST:
					DBG("%lu",t->p1.number);
					break;
			case IP_ST:
					print_ip("", (struct ip_addr*)t->p1.data, "");
					break;
			case EXPR_ST:
					print_expr((struct expr*)t->p1.data);
					break;
			case ACTIONS_ST:
					print_action((struct action*)t->p1.data);
					break;
			case CMDF_ST:
					DBG("f_ptr<%p>",t->p1.data);
					break;
			case SOCKID_ST:
					DBG("%d:%s:%d",
							((struct socket_id*)t->p1.data)->proto,
							ZSW(((struct socket_id*)t->p1.data)->name),
							((struct socket_id*)t->p1.data)->port
							);
					break;
			default:
					DBG("type<%d>", t->p1_type);
		}
		if (t->type==IF_T) DBG(") {");
		switch(t->p2_type){
			case NOSUBTYPE:
					break;
			case STRING_ST:
					DBG(", \"%s\"", ZSW(t->p2.string));
					break;
			case NUMBER_ST:
					DBG(", %lu",t->p2.number);
					break;
			case EXPR_ST:
					print_expr((struct expr*)t->p2.data);
					break;
			case ACTIONS_ST:
					print_action((struct action*)t->p2.data);
					break;
			case SOCKID_ST:
					DBG("%d:%s:%d",
							((struct socket_id*)t->p1.data)->proto,
							ZSW(((struct socket_id*)t->p1.data)->name),
							((struct socket_id*)t->p1.data)->port
							);
					break;
			default:
					DBG(", type<%d>", t->p2_type);
		}
		if (t->type==IF_T) DBG("} else {");
		switch(t->p3_type){
			case NOSUBTYPE:
					break;
			case STRING_ST:
					DBG(", \"%s\"", ZSW(t->p3.string));
					break;
			case NUMBER_ST:
					DBG(", %lu",t->p3.number);
					break;
			case EXPR_ST:
					print_expr((struct expr*)t->p3.data);
					break;
			case ACTIONS_ST:
					print_action((struct action*)t->p3.data);
					break;
			case SOCKID_ST:
					DBG("%d:%s:%d",
							((struct socket_id*)t->p1.data)->proto,
							ZSW(((struct socket_id*)t->p1.data)->name),
							((struct socket_id*)t->p1.data)->port
							);
					break;
			default:
					DBG(", type<%d>", t->p3_type);
		}
		if (t->type==IF_T) DBG("}; ");
		else	DBG("); ");
	}
}
			
	

	
	

