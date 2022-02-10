/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_CFG_TAB_H_INCLUDED
# define YY_YY_CFG_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    FORWARD = 258,
    FORWARD_TCP = 259,
    FORWARD_TLS = 260,
    FORWARD_UDP = 261,
    SEND = 262,
    SEND_TCP = 263,
    DROP = 264,
    LOG_TOK = 265,
    ERROR = 266,
    ROUTE = 267,
    ROUTE_FAILURE = 268,
    ROUTE_ONREPLY = 269,
    EXEC = 270,
    SET_HOST = 271,
    SET_HOSTPORT = 272,
    PREFIX = 273,
    STRIP = 274,
    STRIP_TAIL = 275,
    APPEND_BRANCH = 276,
    SET_USER = 277,
    SET_USERPASS = 278,
    SET_PORT = 279,
    SET_URI = 280,
    REVERT_URI = 281,
    FORCE_RPORT = 282,
    FORCE_TCP_ALIAS = 283,
    IF = 284,
    ELSE = 285,
    SET_ADV_ADDRESS = 286,
    SET_ADV_PORT = 287,
    FORCE_SEND_SOCKET = 288,
    URIHOST = 289,
    URIPORT = 290,
    MAX_LEN = 291,
    SETFLAG = 292,
    RESETFLAG = 293,
    ISFLAGSET = 294,
    METHOD = 295,
    URI = 296,
    FROM_URI = 297,
    TO_URI = 298,
    SRCIP = 299,
    SRCPORT = 300,
    DSTIP = 301,
    DSTPORT = 302,
    PROTO = 303,
    AF = 304,
    MYSELF = 305,
    MSGLEN = 306,
    UDP = 307,
    TCP = 308,
    TLS = 309,
    DEBUG = 310,
    FORK = 311,
    LOGSTDERROR = 312,
    LOGFACILITY = 313,
    LISTEN = 314,
    ALIAS = 315,
    DNS = 316,
    REV_DNS = 317,
    PORT = 318,
    STAT = 319,
    CHILDREN = 320,
    CHECK_VIA = 321,
    WSDB_FIFO_COUNT = 322,
    WS_DUPLICATE_ROUTENO = 323,
    WS_CTC_CALL_FLAG = 324,
    WS_ACC_DB_USER = 325,
    WS_ACC_DB_HOST = 326,
    WS_ACC_DB_PSWD = 327,
    WS_ACC_DB_NAME = 328,
    WS_DB_FIFO_NAME = 329,
    SYN_BRANCH = 330,
    MEMLOG = 331,
    SIP_WARNING = 332,
    FIFO = 333,
    FIFO_DIR = 334,
    SOCK_MODE = 335,
    SOCK_USER = 336,
    SOCK_GROUP = 337,
    FIFO_DB_URL = 338,
    UNIX_SOCK = 339,
    UNIX_SOCK_CHILDREN = 340,
    UNIX_TX_TIMEOUT = 341,
    SERVER_SIGNATURE = 342,
    REPLY_TO_VIA = 343,
    LOADMODULE = 344,
    MODPARAM = 345,
    MAXBUFFER = 346,
    USER = 347,
    GROUP = 348,
    CHROOT = 349,
    WDIR = 350,
    MHOMED = 351,
    DISABLE_TCP = 352,
    TCP_ACCEPT_ALIASES = 353,
    TCP_CHILDREN = 354,
    TCP_CONNECT_TIMEOUT = 355,
    TCP_SEND_TIMEOUT = 356,
    DISABLE_TLS = 357,
    TLSLOG = 358,
    TLS_PORT_NO = 359,
    TLS_METHOD = 360,
    TLS_HANDSHAKE_TIMEOUT = 361,
    TLS_SEND_TIMEOUT = 362,
    SSLv23 = 363,
    SSLv2 = 364,
    SSLv3 = 365,
    TLSv1 = 366,
    TLS_VERIFY = 367,
    TLS_REQUIRE_CERTIFICATE = 368,
    TLS_CERTIFICATE = 369,
    TLS_PRIVATE_KEY = 370,
    TLS_CA_LIST = 371,
    ADVERTISED_ADDRESS = 372,
    ADVERTISED_PORT = 373,
    DISABLE_CORE = 374,
    OPEN_FD_LIMIT = 375,
    MCAST_LOOPBACK = 376,
    MCAST_TTL = 377,
    EQUAL = 378,
    EQUAL_T = 379,
    GT = 380,
    LT = 381,
    GTE = 382,
    LTE = 383,
    DIFF = 384,
    MATCH = 385,
    OR = 386,
    AND = 387,
    NOT = 388,
    PLUS = 389,
    MINUS = 390,
    NUMBER = 391,
    ID = 392,
    STRING = 393,
    IPV6ADDR = 394,
    COMMA = 395,
    SEMICOLON = 396,
    RPAREN = 397,
    LPAREN = 398,
    LBRACE = 399,
    RBRACE = 400,
    LBRACK = 401,
    RBRACK = 402,
    SLASH = 403,
    DOT = 404,
    CR = 405,
    COLON = 406,
    STAR = 407
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 120 "cfg.y" /* yacc.c:1909  */

	long intval;
	unsigned long uval;
	char* strval;
	struct expr* expr;
	struct action* action;
	struct net* ipnet;
	struct ip_addr* ipaddr;
	struct socket_id* sockid;

#line 218 "cfg.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_CFG_TAB_H_INCLUDED  */
