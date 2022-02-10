unixsock_server.o unixsock_server.d : unixsock_server.c config.h types.h ut.h comp_defs.h \
 dprint.h str.h globals.h ip_addr.h trim.h pt.h timer.h socket_info.h \
 sr_module.h parser/msg_parser.h parser/../comp_defs.h parser/../str.h \
 parser/../lump_struct.h parser/../flags.h parser/../ip_addr.h \
 parser/../md5utils.h parser/../str.h parser/../config.h \
 parser/parse_def.h parser/parse_cseq.h parser/parse_to.h \
 parser/parse_via.h parser/parse_fline.h parser/hf.h version.h mem/mem.h \
 mem/../config.h mem/../dprint.h mem/f_malloc.h fifo_server.h \
 unixsock_server.h tsend.h
