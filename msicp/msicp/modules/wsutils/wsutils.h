#ifndef __FUN_H__
#define __FUN_H__
#include <stdio.h>
#include <stdlib.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#include "../../msg_translator.h"
#include "../../data_lump.h"
#include "../../parser/msg_parser.h"
#include "../../parser/parse_uri.h"
#include "../../parser/contact/parse_contact.h"
#include "../../parser/contact/contact.h"
#include "../mysql/dbase.h"
#include "../../db/db.h"
#include "../../sr_module.h"

int get_macid(struct sip_msg *msg , char * macid, char *macaddrlen);
int get_agentid(struct sip_msg *msg, char *agentid, char *s2);

#endif

