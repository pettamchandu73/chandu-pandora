/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 65 "cfg.y" /* yacc.c:339  */


#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include "route_struct.h"
#include "globals.h"
#include "route.h"
#include "dprint.h"
#include "sr_module.h"
#include "modparam.h"
#include "ip_addr.h"
#include "resolve.h"
#include "socket_info.h"
#include "name_alias.h"
#include "ut.h"
#include "dset.h"


#include "config.h"
#ifdef USE_TLS
#include "tls/tls_config.h"
#endif

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

/* hack to avoid alloca usage in the generated C file (needed for compiler
 with no built in alloca, like icc*/
#undef _ALLOCA_H


extern int yylex();
static void yyerror(char* s);
static char* tmp;
static int i_tmp;
static void* f_tmp;
static struct socket_id* lst_tmp;
static int rt;  /* Type of route block for find_export */
static str* str_tmp;
static str s_tmp;
static struct ip_addr* ip_tmp;

static void warn(char* s);
static struct socket_id* mk_listen_id(char*, int, int);
 


#line 121 "cfg.tab.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "cfg.tab.h".  */
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
#line 120 "cfg.y" /* yacc.c:355  */

	long intval;
	unsigned long uval;
	char* strval;
	struct expr* expr;
	struct action* action;
	struct net* ipnet;
	struct ip_addr* ipaddr;
	struct socket_id* sockid;

#line 325 "cfg.tab.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_CFG_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 342 "cfg.tab.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  142
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1284

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  153
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  36
/* YYNRULES -- Number of rules.  */
#define YYNRULES  396
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  783

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   407

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   320,   320,   323,   324,   325,   328,   329,   330,   330,
     331,   331,   332,   332,   333,   336,   351,   359,   369,   370,
     371,   372,   375,   376,   379,   380,   381,   382,   383,   386,
     387,   391,   392,   393,   394,   395,   396,   397,   403,   404,
     405,   406,   407,   408,   409,   414,   415,   416,   417,   418,
     419,   420,   421,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   484,
     485,   492,   493,   500,   501,   508,   509,   516,   517,   524,
     525,   532,   533,   540,   541,   548,   555,   562,   569,   578,
     585,   586,   593,   595,   602,   603,   610,   611,   618,   619,
     626,   627,   634,   635,   636,   637,   638,   639,   652,   654,
     659,   660,   664,   666,   679,   681,   684,   685,   688,   689,
     696,   697,   704,   705,   708,   713,   714,   719,   724,   728,
     729,   732,   766,   786,   787,   791,   793,   801,   804,   812,
     815,   823,   845,   846,   847,   848,   849,   852,   853,   856,
     857,   858,   859,   860,   863,   864,   867,   868,   869,   872,
     875,   878,   879,   882,   885,   888,   891,   892,   895,   897,
     898,   899,   901,   902,   903,   905,   908,   909,   911,   912,
     913,   915,   917,   918,   919,   922,   936,   939,   942,   944,
     946,   949,   963,   966,   969,   971,   973,   976,   979,   982,
     984,   987,   988,   991,   992,  1003,  1004,  1011,  1012,  1015,
    1016,  1028,  1032,  1033,  1034,  1037,  1038,  1041,  1042,  1043,
    1046,  1047,  1048,  1049,  1052,  1060,  1070,  1076,  1082,  1088,
    1094,  1100,  1106,  1115,  1122,  1129,  1130,  1132,  1138,  1144,
    1150,  1157,  1164,  1171,  1180,  1187,  1194,  1195,  1197,  1203,
    1209,  1215,  1222,  1229,  1235,  1244,  1251,  1258,  1259,  1261,
    1274,  1287,  1300,  1313,  1326,  1339,  1354,  1367,  1380,  1381,
    1384,  1390,  1396,  1402,  1408,  1414,  1420,  1421,  1423,  1429,
    1435,  1441,  1447,  1453,  1459,  1460,  1462,  1463,  1464,  1467,
    1473,  1474,  1476,  1478,  1479,  1481,  1482,  1484,  1485,  1491,
    1492,  1494,  1497,  1498,  1500,  1503,  1505,  1506,  1509,  1511,
    1512,  1514,  1516,  1517,  1520,  1522,  1523,  1525,  1534,  1536,
    1538,  1540,  1542,  1543,  1545,  1547,  1548,  1550,  1552,  1553,
    1555,  1557,  1558,  1560,  1562,  1563,  1565,  1566,  1567,  1568,
    1569,  1577,  1584,  1591,  1594,  1606,  1608,  1609,  1627,  1629,
    1630,  1634,  1636,  1637,  1655,  1673,  1694
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "FORWARD", "FORWARD_TCP", "FORWARD_TLS",
  "FORWARD_UDP", "SEND", "SEND_TCP", "DROP", "LOG_TOK", "ERROR", "ROUTE",
  "ROUTE_FAILURE", "ROUTE_ONREPLY", "EXEC", "SET_HOST", "SET_HOSTPORT",
  "PREFIX", "STRIP", "STRIP_TAIL", "APPEND_BRANCH", "SET_USER",
  "SET_USERPASS", "SET_PORT", "SET_URI", "REVERT_URI", "FORCE_RPORT",
  "FORCE_TCP_ALIAS", "IF", "ELSE", "SET_ADV_ADDRESS", "SET_ADV_PORT",
  "FORCE_SEND_SOCKET", "URIHOST", "URIPORT", "MAX_LEN", "SETFLAG",
  "RESETFLAG", "ISFLAGSET", "METHOD", "URI", "FROM_URI", "TO_URI", "SRCIP",
  "SRCPORT", "DSTIP", "DSTPORT", "PROTO", "AF", "MYSELF", "MSGLEN", "UDP",
  "TCP", "TLS", "DEBUG", "FORK", "LOGSTDERROR", "LOGFACILITY", "LISTEN",
  "ALIAS", "DNS", "REV_DNS", "PORT", "STAT", "CHILDREN", "CHECK_VIA",
  "WSDB_FIFO_COUNT", "WS_DUPLICATE_ROUTENO", "WS_CTC_CALL_FLAG",
  "WS_ACC_DB_USER", "WS_ACC_DB_HOST", "WS_ACC_DB_PSWD", "WS_ACC_DB_NAME",
  "WS_DB_FIFO_NAME", "SYN_BRANCH", "MEMLOG", "SIP_WARNING", "FIFO",
  "FIFO_DIR", "SOCK_MODE", "SOCK_USER", "SOCK_GROUP", "FIFO_DB_URL",
  "UNIX_SOCK", "UNIX_SOCK_CHILDREN", "UNIX_TX_TIMEOUT", "SERVER_SIGNATURE",
  "REPLY_TO_VIA", "LOADMODULE", "MODPARAM", "MAXBUFFER", "USER", "GROUP",
  "CHROOT", "WDIR", "MHOMED", "DISABLE_TCP", "TCP_ACCEPT_ALIASES",
  "TCP_CHILDREN", "TCP_CONNECT_TIMEOUT", "TCP_SEND_TIMEOUT", "DISABLE_TLS",
  "TLSLOG", "TLS_PORT_NO", "TLS_METHOD", "TLS_HANDSHAKE_TIMEOUT",
  "TLS_SEND_TIMEOUT", "SSLv23", "SSLv2", "SSLv3", "TLSv1", "TLS_VERIFY",
  "TLS_REQUIRE_CERTIFICATE", "TLS_CERTIFICATE", "TLS_PRIVATE_KEY",
  "TLS_CA_LIST", "ADVERTISED_ADDRESS", "ADVERTISED_PORT", "DISABLE_CORE",
  "OPEN_FD_LIMIT", "MCAST_LOOPBACK", "MCAST_TTL", "EQUAL", "EQUAL_T", "GT",
  "LT", "GTE", "LTE", "DIFF", "MATCH", "OR", "AND", "NOT", "PLUS", "MINUS",
  "NUMBER", "ID", "STRING", "IPV6ADDR", "COMMA", "SEMICOLON", "RPAREN",
  "LPAREN", "LBRACE", "RBRACE", "LBRACK", "RBRACK", "SLASH", "DOT", "CR",
  "COLON", "STAR", "$accept", "cfg", "statements", "statement", "$@1",
  "$@2", "$@3", "listen_id", "proto", "port", "phostport", "id_lst",
  "assign_stm", "module_stm", "ip", "ipv4", "ipv6addr", "ipv6",
  "route_stm", "failure_route_stm", "onreply_route_stm", "exp", "equalop",
  "intop", "strop", "uri_type", "exp_elem", "ipnet", "host_sep", "host",
  "exp_stm", "stm", "actions", "action", "if_cmd", "cmd", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,   406,   407
};
# endif

#define YYPACT_NINF -378

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-378)))

#define YYTABLE_NINF -248

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     320,   -82,   -74,   -59,   -49,   -23,   -14,     0,     9,    13,
      23,    28,   181,   192,   196,   197,   200,   201,   215,   216,
     219,   220,   228,   233,   234,   242,   425,   426,   447,   460,
     473,   474,   485,   489,   495,   503,    81,    10,   513,   516,
     520,   521,   522,   523,   524,   527,   528,   533,   534,   554,
     679,   681,   682,   683,   686,   687,   688,   693,   694,   695,
     697,   698,   699,   701,   702,   705,  -378,    75,   172,  -378,
     125,   340,   416,  -378,  -378,  -378,   677,   690,   691,    22,
       6,    25,   799,   842,   846,   449,   847,   848,   862,   863,
     879,   570,   576,   581,   592,   609,   883,   884,   885,   615,
     620,   889,    83,   372,   631,   648,   890,   891,   893,   894,
    -378,  -378,  -378,   510,   895,   451,   453,   497,   525,   897,
     898,   960,   961,  1027,  1036,  1037,  1038,  1039,    20,  1040,
    1043,  1056,  1057,   652,   653,   654,   159,  1073,  1077,  1083,
    1085,  1086,  -378,   -82,  -378,    43,  -378,    29,  -378,    30,
    -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,
    -378,  -378,  -378,   470,  -378,  -378,  -378,   519,  -378,   501,
     680,   -25,  -378,  -378,  -378,  -378,  -378,   -97,  -378,  -378,
    -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,
    -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,
    -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,
    -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,
    -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,
    -378,  -378,  -378,  -378,  -378,  -378,  -378,   689,  -378,  -378,
    -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,
    -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,
    -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,
    -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,
    -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,
    -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,
    -378,  -378,  1044,   696,  -378,   703,  -378,   704,   706,   667,
      18,   492,  -378,  -378,   594,   707,   692,    12,    14,    24,
      45,    52,    54,   708,    62,    72,    79,   709,    80,    82,
     137,   138,   139,   710,   140,   211,   305,   306,   712,   714,
     718,   899,   308,   312,   313,   316,   348,   349,   722,  -378,
     457,  -378,  -378,   330,   713,   721,   723,   684,  -378,  -378,
    -378,  -378,  -378,   685,  -378,  -378,   727,  -378,    65,  -378,
      70,  -378,   164,  -378,   190,  -378,   209,  -378,   225,   729,
    -378,   529,  -378,   658,  -378,  1087,   731,  -378,   659,  -378,
     660,  -378,   663,  -378,  1089,  -378,  1090,   -52,  -378,   665,
    -378,   669,  -378,   670,  -378,   674,   730,   732,   351,  1107,
    -378,  -378,  -378,  1110,  1127,  1114,  1133,  1139,  1145,  1103,
    1151,   899,  -378,   899,  1044,   948,  1117,  -378,  -378,  -378,
    -378,  -378,   307,  -378,  1092,  -378,    44,  -378,   737,  -378,
     739,  -378,   740,   427,  -378,  -378,  -378,  -378,  -378,   733,
     734,   735,   745,  -101,   -99,   741,  -100,   -48,   -39,   -92,
     746,   -22,   -18,     7,   -47,   747,     8,   226,   227,   176,
     758,   230,   289,   438,   195,   770,   444,   461,   464,   771,
     469,   482,   465,  -378,   787,   742,   791,   903,   906,   957,
     958,   959,   963,   964,   965,   967,   968,   970,   971,   972,
     974,   975,   483,  -378,   977,   978,   982,   983,   988,   989,
     990,   991,  -378,  -378,   993,   994,  -378,  -378,  -378,  -378,
    -378,  -378,   531,  -378,   481,   536,  -378,  -378,  -378,  -378,
    -378,  -378,  1093,  -378,   720,   575,  -378,  1094,  -378,    16,
    -378,  1097,  -378,    64,  -378,   798,  -378,   -70,   496,   899,
     899,  1044,   805,  -378,  -378,   788,   614,   995,   996,   997,
     999,  1000,  1001,  1002,  1003,  1005,  1006,   500,  -378,  1044,
    1044,  1044,   738,  -378,  1007,  1008,  -378,   -11,  -378,   966,
    -378,  1015,  -378,  1017,  -378,  -378,    -3,  -378,  1018,  -378,
    1019,  -378,  1020,  -378,  -378,    -2,  -378,  1021,  -378,  1022,
    -378,  1023,  -378,  -378,    -1,  -378,  1024,  -378,  1025,  -378,
    1026,  -378,  -378,  1028,  -378,  1029,  -378,  1030,  -378,  -378,
    1031,  -378,  1032,  -378,  1033,  -378,  -378,  1042,  -378,  -378,
    1045,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,
    -378,  -378,  -378,  -378,  1046,  -378,  -378,  -378,  -378,  -378,
    -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,
     955,  -378,  -378,  -378,   -97,  -378,  -378,  -378,  -378,  -378,
    -378,   -97,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,
    -378,  -378,  -378,  -378,  -378,  -378,  -378,  1050,  -378,   535,
     985,  -378,  -378,  -378,   -97,  -378,  -378,  -378,  -378,  -378,
    -378,  -378,  -378,  -378,  -378,  1048,  -378,   678,   717,   756,
    1034,  -378,  -378,  1035,  1047,  1049,  1052,  1053,  1054,  1055,
    1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1068,
    1069,  1070,  1072,  1074,  1075,  1076,  1078,  1082,  1100,  1106,
    1140,  1141,   311,  -378,  -378,  1142,  -378,  -378,  -378,  -378,
    -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,
    -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,
    -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,
     470,  -378,  -378
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,     0,     0,     4,
       0,     0,     0,     6,     7,   163,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     165,   164,   168,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     1,     5,     3,     0,     9,     0,    11,     0,
      13,    32,    31,    34,    33,    36,    35,    38,    37,   148,
      18,    19,    20,     0,   249,    16,   172,     0,    21,    24,
       0,    29,   147,    15,   169,   173,   170,    17,   150,   149,
      40,    39,    42,    41,    47,    43,    44,    49,    48,    51,
      50,    57,    56,    53,    52,    55,    54,    59,    58,    61,
      60,    63,    62,    65,    64,    67,    66,    69,    68,    71,
      70,    73,    72,    75,    74,    77,    76,    79,    78,    82,
      81,    80,    85,    84,    83,    87,    86,    89,    88,    91,
      90,    93,    92,   144,   143,   146,   145,     0,    46,    45,
      96,    95,    94,    99,    98,    97,   102,   101,   100,   105,
     104,   103,   107,   106,   109,   108,   111,   110,   113,   112,
     115,   114,   117,   116,   119,   118,   121,   120,   123,   122,
     128,   124,   125,   126,   127,   140,   139,   142,   141,   130,
     129,   132,   131,   134,   133,   136,   135,   138,   137,   152,
     151,   154,   153,   156,   155,   158,   157,   160,   159,   162,
     161,   177,     0,     0,   179,     0,   181,     0,     0,     0,
       0,     0,    30,   248,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   327,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   360,     0,     0,     0,     0,   377,   379,
     382,     0,     0,     0,     0,     0,     0,     0,     0,   262,
       0,   258,   261,     0,     0,     0,     0,     0,   174,    28,
      22,    23,    25,    26,   251,   250,     0,   275,     0,   297,
       0,   308,     0,   286,     0,   316,     0,   324,     0,     0,
     330,     0,   339,     0,   342,     0,     0,   346,     0,   362,
       0,   349,     0,   355,     0,   352,     0,     0,   368,     0,
     371,     0,   365,     0,   374,     0,     0,     0,     0,     0,
     196,   197,   198,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   242,     0,     0,     0,     0,   186,   241,   253,
     252,   386,     0,   389,     0,   392,     0,   333,     0,   335,
       0,   337,     0,     0,   259,   175,   257,   263,   260,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   326,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   359,     0,     0,     0,     0,     0,     0,
       0,     0,   376,   378,     0,     0,   381,   202,   187,   188,
     195,   194,     0,   229,   194,     0,   210,   190,   191,   192,
     193,   189,     0,   235,   194,     0,   213,     0,   216,     0,
     219,     0,   240,     0,   223,     0,   184,     0,     0,     0,
       0,     0,   264,   255,   207,   194,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   393,     0,
       0,     0,     0,    27,     0,     0,   276,     0,   274,     0,
     267,     0,   268,     0,   266,   298,     0,   296,     0,   289,
       0,   290,     0,   288,   309,     0,   307,     0,   300,     0,
     301,     0,   299,   287,     0,   285,     0,   278,     0,   279,
       0,   277,   317,     0,   311,     0,   312,     0,   310,   325,
       0,   319,     0,   320,     0,   318,   331,     0,   328,   340,
       0,   343,   341,   344,   347,   345,   363,   361,   350,   348,
     356,   354,   353,   351,     0,   358,   369,   367,   372,   370,
     366,   364,   375,   373,   383,   380,   201,   200,   199,   227,
     245,   224,   228,   225,   226,   209,   208,   233,   230,   234,
     231,   232,   212,   211,   215,   214,   218,   217,   239,   237,
     238,   236,   222,   221,   220,   185,   254,   183,   182,     0,
       0,   205,   206,   203,   204,   385,   384,   388,   387,   391,
     390,   332,   334,   336,   396,     0,   394,     0,     0,     0,
       0,   167,   166,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   256,   265,     0,   176,   178,   180,   171,
     272,   273,   270,   271,   269,   294,   295,   292,   293,   291,
     305,   306,   303,   304,   302,   283,   284,   281,   282,   280,
     314,   315,   313,   322,   323,   321,   329,   338,   357,   246,
     244,   243,   395
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -378,  -378,  -378,   769,  -378,  -378,  -378,  -133,   302,   392,
     410,   -45,  -378,  -378,  -368,  -378,  1004,  -378,  -378,  -378,
    -378,   700,   166,  -301,  -359,   584,  -378,   644,  -378,  -356,
    -378,   160,  -377,  -349,  -336,  -332
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    67,    68,    69,    70,    71,    72,   169,   170,   362,
     171,   172,    73,    74,   173,   174,   175,   176,   146,   148,
     150,   425,   531,   532,   522,   426,   427,   661,   315,   177,
     428,   552,   350,   351,   352,   353
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     458,   446,   463,   290,   468,   429,   473,   159,   477,   430,
     481,   112,   459,   367,   464,   369,   469,   674,   474,   359,
     478,   270,   482,   157,   713,   371,   178,   160,   161,   162,
     304,   306,   718,   723,   728,   360,   179,   574,   313,   575,
     577,    75,   578,   313,   301,   561,   373,   548,   583,    76,
     584,   361,   314,   375,   525,   377,   535,   314,   160,   161,
     162,   549,   550,   380,    77,   678,   455,   556,   160,   161,
     162,   460,   685,   382,    78,   142,   553,   160,   161,   162,
     384,   387,   110,   389,   219,   429,   502,   429,   313,   430,
     503,   430,   579,   592,   580,   593,   160,   161,   162,   456,
      79,   581,   314,   582,   461,   410,   411,   412,   679,    80,
     680,   163,   164,   165,   166,   537,   539,   541,   586,   545,
     587,   167,   588,    81,   589,   714,   312,   168,   271,   272,
     273,   274,    82,   719,   724,   729,    83,   145,   391,   393,
     395,   398,   163,   164,   165,   166,    84,   590,   595,   591,
     596,    85,   167,   113,   360,   368,   660,   370,   168,   158,
     289,   163,   164,   165,   166,   465,   660,   372,   168,   664,
     361,   167,    -2,   143,   689,   305,   307,   168,   363,   671,
     163,   164,   165,   166,    -8,   -10,   -12,   302,   374,   303,
     167,   470,   707,   708,   709,   376,   168,   378,   466,   446,
     694,   163,   164,   457,   166,   381,   163,   164,   462,   166,
     475,   167,   400,   429,   429,   383,   167,   430,   430,   111,
     220,   221,   385,   388,   471,   390,   479,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
     392,   394,   396,   399,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,   163,   164,   165,   166,   558,
     163,   164,   467,   166,    86,   167,   402,   404,   557,   431,
     167,   313,   779,   433,   435,    87,   601,   437,   602,    88,
      89,     1,    66,    90,    91,   314,   163,   164,   472,   166,
     313,   447,    -8,   -10,   -12,   610,   167,   611,    92,    93,
     446,   553,    94,    95,   314,   163,   164,   476,   166,   439,
     441,    96,   514,   147,   401,   167,    97,    98,   446,   446,
     446,   163,   164,   480,   166,    99,   597,   599,   598,   600,
     604,   167,   605,   222,   781,     2,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,   566,   606,
     149,   607,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,   163,   164,   165,   166,   780,   403,   405,
     166,   432,   240,   167,   243,   434,   436,   167,   444,   438,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
      66,   448,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   515,   342,   343,
     344,   440,   442,   516,   345,   346,   347,   444,   246,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   223,
     224,   327,   328,   329,   330,   331,   332,   333,   334,   335,
     336,   337,   338,   339,   340,   341,   249,   342,   343,   344,
     484,   659,   656,   345,   346,   347,   444,   662,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   100,   101,
     327,   328,   329,   330,   331,   332,   333,   334,   335,   336,
     337,   338,   339,   340,   341,   567,   342,   343,   344,   568,
     102,   197,   345,   346,   347,   521,   669,   199,   608,   524,
     609,   534,   201,   103,   613,   543,   614,   186,   241,   242,
     244,   245,   555,   203,   348,   364,   104,   105,   349,   313,
     313,   615,   445,   616,   617,   624,   618,   625,   106,   620,
     205,   621,   107,   314,   314,   692,   213,   163,   108,   308,
     166,   215,   622,   644,   623,   645,   109,   167,   163,   164,
     165,   166,   225,   348,   247,   248,   114,   349,   167,   115,
     705,   686,   706,   116,   117,   118,   119,   120,   237,   227,
     121,   122,   310,   283,   285,   287,   123,   124,   166,   487,
     492,   494,   250,   251,   496,   485,   504,   486,   657,   658,
     506,   508,   348,   164,   663,   510,   349,   125,   151,   444,
     743,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   153,   155,   327,   328,   329,   330,   331,   332,   333,
     334,   335,   336,   337,   338,   339,   340,   341,   198,   342,
     343,   344,   164,   670,   200,   345,   346,   347,   444,   202,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     204,  -247,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   206,   342,   343,
     344,   164,   693,   214,   345,   346,   347,   444,   216,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   226,
     667,   327,   328,   329,   330,   331,   332,   333,   334,   335,
     336,   337,   338,   339,   340,   341,   228,   342,   343,   344,
     284,   286,   288,   345,   346,   347,   488,   493,   495,   682,
     180,   497,   126,   505,   127,   128,   129,   507,   509,   130,
     131,   132,   511,   152,   358,   348,   133,   134,   135,   349,
     136,   137,   138,   746,   139,   140,   154,   156,   141,   316,
     366,   311,   354,   452,   683,   690,   453,   144,   691,   355,
     356,   675,   357,   182,   365,   573,   562,   184,   187,   189,
     744,   379,   386,   397,   348,   406,   163,   407,   349,   166,
     449,   408,   747,   191,   193,   443,   167,   454,   450,   491,
     451,   483,   512,   563,   513,   564,   565,   569,   570,   571,
     195,   572,   627,   576,   207,   209,   211,   710,   585,   594,
     217,   229,   231,   348,   233,   235,   238,   349,   252,   254,
     603,   748,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   612,   619,   327,   328,   329,   330,   331,   332,
     333,   334,   335,   336,   337,   338,   339,   340,   341,   626,
     342,   343,   344,   628,   684,   181,   345,   346,   347,   409,
     410,   411,   412,   413,   414,   415,   416,   417,   418,   419,
     420,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   256,   258,   327,   328,   329,   330,   331,   332,   333,
     334,   335,   336,   337,   338,   339,   340,   341,   183,   342,
     343,   344,   185,   188,   190,   345,   346,   347,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   192,   194,
     327,   328,   329,   330,   331,   332,   333,   334,   335,   336,
     337,   338,   339,   340,   341,   196,   342,   343,   344,   208,
     210,   212,   345,   346,   347,   218,   230,   232,   260,   234,
     236,   239,   421,   253,   255,   422,   348,   262,   264,   266,
     268,   275,   423,   424,   277,   629,   630,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   279,   281,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,   337,
     338,   339,   340,   341,   291,   342,   343,   344,   293,   549,
     550,   345,   346,   347,   295,   348,   297,   299,   489,   349,
     498,   500,   551,   559,   665,   672,   257,   259,   676,   631,
     632,   633,   715,   742,   542,   634,   635,   636,   517,   637,
     638,   523,   639,   640,   641,   533,   642,   643,   554,   646,
     647,   546,   348,   547,   648,   649,   349,   681,   526,   551,
     650,   651,   652,   653,   536,   654,   655,   695,   696,   697,
     538,   698,   699,   700,   701,   702,   540,   703,   704,   711,
     712,   716,   544,   717,   720,   721,   722,   725,   726,   727,
     730,   731,   732,   261,   733,   734,   735,   736,   737,   738,
     749,   309,   263,   265,   267,   269,   276,   750,   668,   278,
     739,   348,   550,   740,   741,   349,   745,     0,     0,   751,
       0,   752,   280,   282,   753,   754,   755,   756,     0,     0,
     757,   758,   759,   760,   761,   762,   763,   764,   765,   292,
     766,   767,   768,   294,   769,     0,   770,   771,   772,   296,
     773,   298,   300,   490,   774,   499,   501,   518,   560,   666,
     673,   518,   519,   677,   518,     0,   519,   520,   518,   519,
     520,   518,   775,   519,   520,     0,   519,   520,   776,   687,
     688,   518,   527,   528,   529,   530,   519,   518,   527,   528,
     529,   530,   519,   518,   527,   528,   529,   530,   519,   518,
     527,   528,   529,   530,   519,   518,   527,   528,   529,   530,
     519,     0,   777,   778,   782
};

static const yytype_int16 yycheck[] =
{
     368,   350,   370,   136,   372,   341,   374,     1,   376,   341,
     378,     1,   368,     1,   370,     1,   372,     1,   374,     1,
     376,     1,   378,     1,    35,     1,     1,    52,    53,    54,
       1,     1,    35,    35,    35,   136,    81,   136,   135,   138,
     140,   123,   142,   135,     1,     1,     1,   424,   140,   123,
     142,   152,   149,     1,   413,     1,   415,   149,    52,    53,
      54,   131,   132,     1,   123,     1,     1,   426,    52,    53,
      54,     1,   142,     1,   123,     0,   425,    52,    53,    54,
       1,     1,     1,     1,     1,   421,   138,   423,   135,   421,
     142,   423,   140,   140,   142,   142,    52,    53,    54,    34,
     123,   140,   149,   142,    34,    41,    42,    43,    44,   123,
      46,   136,   137,   138,   139,   416,   417,   418,   140,   420,
     142,   146,   140,   123,   142,   136,   171,   152,   108,   109,
     110,   111,   123,   136,   136,   136,   123,    12,     1,     1,
       1,     1,   136,   137,   138,   139,   123,   140,   140,   142,
     142,   123,   146,   143,   136,   143,   524,   143,   152,   137,
       1,   136,   137,   138,   139,     1,   534,   143,   152,   525,
     152,   146,     0,     1,   551,   146,   146,   152,   311,   535,
     136,   137,   138,   139,    12,    13,    14,   144,   143,   146,
     146,     1,   569,   570,   571,   143,   152,   143,    34,   548,
     556,   136,   137,   138,   139,   143,   136,   137,   138,   139,
       1,   146,     1,   549,   550,   143,   146,   549,   550,   138,
     137,   138,   143,   143,    34,   143,     1,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     143,   143,   143,   143,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   136,   137,   138,   139,   432,
     136,   137,   138,   139,   123,   146,     1,     1,     1,     1,
     146,   135,     1,     1,     1,   123,   140,     1,   142,   123,
     123,     1,   150,   123,   123,   149,   136,   137,   138,   139,
     135,     1,    12,    13,    14,   140,   146,   142,   123,   123,
     689,   690,   123,   123,   149,   136,   137,   138,   139,     1,
       1,   123,     1,    13,   143,   146,   123,   123,   707,   708,
     709,   136,   137,   138,   139,   123,   140,   140,   142,   142,
     140,   146,   142,     1,   742,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,     1,   140,
      14,   142,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   136,   137,   138,   139,   136,   143,   143,
     139,   143,     1,   146,     1,   143,   143,   146,     1,   143,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
     150,   141,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,   136,    31,    32,
      33,   143,   143,   142,    37,    38,    39,     1,     1,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,   137,
     138,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     1,    31,    32,    33,
       1,    50,     1,    37,    38,    39,     1,     1,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,   123,   123,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,   138,    31,    32,    33,   142,
     123,     1,    37,    38,    39,   409,     1,     1,   140,   413,
     142,   415,     1,   123,   140,   419,   142,   138,   137,   138,
     137,   138,   426,     1,   137,     1,   123,   123,   141,   135,
     135,   140,   145,   142,   140,   140,   142,   142,   123,   140,
       1,   142,   123,   149,   149,     1,     1,   136,   123,   149,
     139,     1,   140,   140,   142,   142,   123,   146,   136,   137,
     138,   139,     1,   137,   137,   138,   123,   141,   146,   123,
     140,   145,   142,   123,   123,   123,   123,   123,   138,     1,
     123,   123,   151,     1,     1,     1,   123,   123,   139,     1,
       1,     1,   137,   138,     1,   136,     1,   138,   137,   138,
       1,     1,   137,   137,   138,     1,   141,   123,     1,     1,
     145,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,     1,     1,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,   138,    31,
      32,    33,   137,   138,   138,    37,    38,    39,     1,   138,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
     138,   137,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,   138,    31,    32,
      33,   137,   138,   138,    37,    38,    39,     1,   138,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,   138,
      50,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,   138,    31,    32,    33,
     138,   138,   138,    37,    38,    39,   138,   138,   138,     1,
       1,   138,   123,   138,   123,   123,   123,   138,   138,   123,
     123,   123,   138,   136,   147,   137,   123,   123,   123,   141,
     123,   123,   123,   145,   123,   123,   136,   136,   123,   140,
     138,   151,   136,   149,    36,    30,   151,    68,    50,   136,
     136,   539,   136,     1,   137,   453,   436,     1,     1,     1,
     690,   143,   143,   143,   137,   143,   136,   143,   141,   139,
     147,   143,   145,     1,     1,   143,   146,   140,   147,   138,
     147,   142,   142,   136,   142,   136,   136,   144,   144,   144,
       1,   136,   140,   142,     1,     1,     1,   149,   142,   142,
       1,     1,     1,   137,     1,     1,     1,   141,     1,     1,
     142,   145,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,   142,   142,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,   142,
      31,    32,    33,   142,   136,   136,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,     1,     1,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,   136,    31,
      32,    33,   136,   136,   136,    37,    38,    39,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,   136,   136,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,   136,    31,    32,    33,   136,
     136,   136,    37,    38,    39,   136,   136,   136,     1,   136,
     136,   136,   133,   136,   136,   136,   137,     1,     1,     1,
       1,     1,   143,   144,     1,   142,   140,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,     1,     1,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,     1,    31,    32,    33,     1,   131,
     132,    37,    38,    39,     1,   137,     1,     1,     1,   141,
       1,     1,   144,     1,     1,     1,   136,   136,     1,   142,
     142,   142,   136,   148,     1,   142,   142,   142,     1,   142,
     142,     1,   142,   142,   142,     1,   142,   142,     1,   142,
     142,   421,   137,   423,   142,   142,   141,   543,     1,   144,
     142,   142,   142,   142,     1,   142,   142,   142,   142,   142,
       1,   142,   142,   142,   142,   142,     1,   142,   142,   142,
     142,   136,     1,   136,   136,   136,   136,   136,   136,   136,
     136,   136,   136,   136,   136,   136,   136,   136,   136,   136,
     136,   167,   136,   136,   136,   136,   136,   142,   534,   136,
     138,   137,   132,   138,   138,   141,   138,    -1,    -1,   142,
      -1,   142,   136,   136,   142,   142,   142,   142,    -1,    -1,
     142,   142,   142,   142,   142,   142,   142,   142,   142,   136,
     142,   142,   142,   136,   142,    -1,   142,   142,   142,   136,
     142,   136,   136,   136,   142,   136,   136,   124,   136,   136,
     136,   124,   129,   136,   124,    -1,   129,   130,   124,   129,
     130,   124,   142,   129,   130,    -1,   129,   130,   142,   549,
     550,   124,   125,   126,   127,   128,   129,   124,   125,   126,
     127,   128,   129,   124,   125,   126,   127,   128,   129,   124,
     125,   126,   127,   128,   129,   124,   125,   126,   127,   128,
     129,    -1,   142,   142,   142
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   150,   154,   155,   156,
     157,   158,   159,   165,   166,   123,   123,   123,   123,   123,
     123,   123,   123,   123,   123,   123,   123,   123,   123,   123,
     123,   123,   123,   123,   123,   123,   123,   123,   123,   123,
     123,   123,   123,   123,   123,   123,   123,   123,   123,   123,
       1,   138,     1,   143,   123,   123,   123,   123,   123,   123,
     123,   123,   123,   123,   123,   123,   123,   123,   123,   123,
     123,   123,   123,   123,   123,   123,   123,   123,   123,   123,
     123,   123,     0,     1,   156,    12,   171,    13,   172,    14,
     173,     1,   136,     1,   136,     1,   136,     1,   137,     1,
      52,    53,    54,   136,   137,   138,   139,   146,   152,   160,
     161,   163,   164,   167,   168,   169,   170,   182,     1,   164,
       1,   136,     1,   136,     1,   136,   138,     1,   136,     1,
     136,     1,   136,     1,   136,     1,   136,     1,   138,     1,
     138,     1,   138,     1,   138,     1,   138,     1,   136,     1,
     136,     1,   136,     1,   138,     1,   138,     1,   136,     1,
     137,   138,     1,   137,   138,     1,   138,     1,   138,     1,
     136,     1,   136,     1,   136,     1,   136,   138,     1,   136,
       1,   137,   138,     1,   137,   138,     1,   137,   138,     1,
     137,   138,     1,   136,     1,   136,     1,   136,     1,   136,
       1,   136,     1,   136,     1,   136,     1,   136,     1,   136,
       1,   108,   109,   110,   111,     1,   136,     1,   136,     1,
     136,     1,   136,     1,   138,     1,   138,     1,   138,     1,
     160,     1,   136,     1,   136,     1,   136,     1,   136,     1,
     136,     1,   144,   146,     1,   146,     1,   146,   149,   169,
     151,   151,   164,   135,   149,   181,   140,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    31,    32,    33,    37,    38,    39,   137,   141,
     185,   186,   187,   188,   136,   136,   136,   136,   147,     1,
     136,   152,   162,   160,     1,   137,   138,     1,   143,     1,
     143,     1,   143,     1,   143,     1,   143,     1,   143,   143,
       1,   143,     1,   143,     1,   143,   143,     1,   143,     1,
     143,     1,   143,     1,   143,     1,   143,   143,     1,   143,
       1,   143,     1,   143,     1,   143,   143,   143,   143,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,   133,   136,   143,   144,   174,   178,   179,   183,   187,
     188,     1,   143,     1,   143,     1,   143,     1,   143,     1,
     143,     1,   143,   143,     1,   145,   186,     1,   141,   147,
     147,   147,   149,   151,   140,     1,    34,   138,   167,   182,
       1,    34,   138,   167,   182,     1,    34,   138,   167,   182,
       1,    34,   138,   167,   182,     1,   138,   167,   182,     1,
     138,   167,   182,   142,     1,   136,   138,     1,   138,     1,
     136,   138,     1,   138,     1,   138,     1,   138,     1,   136,
       1,   136,   138,   142,     1,   138,     1,   138,     1,   138,
       1,   138,   142,   142,     1,   136,   142,     1,   124,   129,
     130,   175,   177,     1,   175,   177,     1,   125,   126,   127,
     128,   175,   176,     1,   175,   177,     1,   176,     1,   176,
       1,   176,     1,   175,     1,   176,   174,   174,   185,   131,
     132,   144,   184,   186,     1,   175,   177,     1,   160,     1,
     136,     1,   163,   136,   136,   136,     1,   138,   142,   144,
     144,   144,   136,   162,   136,   138,   142,   140,   142,   140,
     142,   140,   142,   140,   142,   142,   140,   142,   140,   142,
     140,   142,   140,   142,   142,   140,   142,   140,   142,   140,
     142,   140,   142,   142,   140,   142,   140,   142,   140,   142,
     140,   142,   142,   140,   142,   140,   142,   140,   142,   142,
     140,   142,   140,   142,   140,   142,   142,   140,   142,   142,
     140,   142,   142,   142,   142,   142,   142,   142,   142,   142,
     142,   142,   142,   142,   140,   142,   142,   142,   142,   142,
     142,   142,   142,   142,   142,   142,     1,   137,   138,    50,
     167,   180,     1,   138,   182,     1,   136,    50,   180,     1,
     138,   182,     1,   136,     1,   161,     1,   136,     1,    44,
      46,   178,     1,    36,   136,   142,   145,   174,   174,   185,
      30,    50,     1,   138,   182,   142,   142,   142,   142,   142,
     142,   142,   142,   142,   142,   140,   142,   185,   185,   185,
     149,   142,   142,    35,   136,   136,   136,   136,    35,   136,
     136,   136,   136,    35,   136,   136,   136,   136,    35,   136,
     136,   136,   136,   136,   136,   136,   136,   136,   136,   138,
     138,   138,   148,   145,   184,   138,   145,   145,   145,   136,
     142,   142,   142,   142,   142,   142,   142,   142,   142,   142,
     142,   142,   142,   142,   142,   142,   142,   142,   142,   142,
     142,   142,   142,   142,   142,   142,   142,   142,   142,     1,
     136,   167,   142
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   153,   154,   155,   155,   155,   156,   156,   157,   156,
     158,   156,   159,   156,   156,   160,   160,   160,   161,   161,
     161,   161,   162,   162,   163,   163,   163,   163,   163,   164,
     164,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   166,   166,   166,   166,   166,   167,
     167,   168,   169,   170,   170,   171,   171,   171,   172,   172,
     173,   173,   174,   174,   174,   174,   174,   175,   175,   176,
     176,   176,   176,   176,   177,   177,   178,   178,   178,   179,
     179,   179,   179,   179,   179,   179,   179,   179,   179,   179,
     179,   179,   179,   179,   179,   179,   179,   179,   179,   179,
     179,   179,   179,   179,   179,   179,   179,   179,   179,   179,
     179,   179,   179,   179,   179,   179,   179,   179,   179,   179,
     179,   179,   179,   180,   180,   180,   180,   181,   181,   182,
     182,   182,   183,   183,   183,   184,   184,   185,   185,   185,
     186,   186,   186,   186,   187,   187,   188,   188,   188,   188,
     188,   188,   188,   188,   188,   188,   188,   188,   188,   188,
     188,   188,   188,   188,   188,   188,   188,   188,   188,   188,
     188,   188,   188,   188,   188,   188,   188,   188,   188,   188,
     188,   188,   188,   188,   188,   188,   188,   188,   188,   188,
     188,   188,   188,   188,   188,   188,   188,   188,   188,   188,
     188,   188,   188,   188,   188,   188,   188,   188,   188,   188,
     188,   188,   188,   188,   188,   188,   188,   188,   188,   188,
     188,   188,   188,   188,   188,   188,   188,   188,   188,   188,
     188,   188,   188,   188,   188,   188,   188,   188,   188,   188,
     188,   188,   188,   188,   188,   188,   188,   188,   188,   188,
     188,   188,   188,   188,   188,   188,   188,   188,   188,   188,
     188,   188,   188,   188,   188,   188,   188,   188,   188,   188,
     188,   188,   188,   188,   188,   188,   188
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     1,     2,     1,     1,     0,     2,
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     3,     5,     3,     1,
       2,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     2,     2,     2,     8,     8,     2,     1,
       1,     7,     1,     1,     3,     4,     7,     2,     7,     2,
       7,     2,     3,     3,     2,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       3,     3,     2,     3,     3,     3,     3,     2,     3,     3,
       2,     3,     3,     2,     3,     3,     2,     3,     3,     2,
       3,     3,     3,     2,     3,     3,     3,     3,     3,     2,
       3,     3,     3,     3,     3,     2,     3,     3,     3,     3,
       2,     1,     1,     3,     3,     1,     3,     1,     1,     1,
       3,     3,     1,     1,     3,     1,     3,     2,     1,     2,
       2,     1,     1,     2,     3,     5,     4,     4,     4,     6,
       6,     6,     6,     6,     4,     2,     4,     4,     4,     4,
       6,     6,     6,     6,     6,     4,     2,     4,     4,     4,
       4,     6,     6,     6,     6,     6,     4,     2,     4,     4,
       4,     4,     6,     6,     6,     6,     6,     4,     2,     4,
       4,     4,     4,     6,     6,     6,     2,     4,     4,     4,
       4,     6,     6,     6,     2,     4,     3,     1,     4,     6,
       2,     4,     4,     2,     4,     2,     4,     2,     6,     2,
       4,     4,     2,     4,     4,     4,     2,     4,     4,     2,
       4,     4,     2,     4,     4,     2,     4,     6,     4,     3,
       1,     4,     2,     4,     4,     2,     4,     4,     2,     4,
       4,     2,     4,     4,     2,     4,     3,     1,     3,     1,
       4,     3,     1,     4,     4,     4,     2,     4,     4,     2,
       4,     4,     2,     3,     4,     6,     4
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 3:
#line 323 "cfg.y" /* yacc.c:1646  */
    {}
#line 2081 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 324 "cfg.y" /* yacc.c:1646  */
    {}
#line 2087 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 325 "cfg.y" /* yacc.c:1646  */
    { yyerror(""); YYABORT;}
#line 2093 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 330 "cfg.y" /* yacc.c:1646  */
    {rt=REQUEST_ROUTE;}
#line 2099 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 331 "cfg.y" /* yacc.c:1646  */
    {rt=FAILURE_ROUTE;}
#line 2105 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 332 "cfg.y" /* yacc.c:1646  */
    {rt=ONREPLY_ROUTE;}
#line 2111 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 336 "cfg.y" /* yacc.c:1646  */
    {	tmp=ip_addr2a((yyvsp[0].ipaddr));
		 					if(tmp==0){
								LOG(L_CRIT, "ERROR: cfg. parser: bad ip "
										"address.\n");
								(yyval.strval)=0;
							}else{
								(yyval.strval)=pkg_malloc(strlen(tmp)+1);
								if ((yyval.strval)==0){
									LOG(L_CRIT, "ERROR: cfg. parser: out of "
											"memory.\n");
								}else{
									strncpy((yyval.strval), tmp, strlen(tmp)+1);
								}
							}
						}
#line 2131 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 351 "cfg.y" /* yacc.c:1646  */
    {	(yyval.strval)=pkg_malloc(strlen((yyvsp[0].strval))+1);
		 					if ((yyval.strval)==0){
									LOG(L_CRIT, "ERROR: cfg. parser: out of "
											"memory.\n");
							}else{
									strncpy((yyval.strval), (yyvsp[0].strval), strlen((yyvsp[0].strval))+1);
							}
						}
#line 2144 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 359 "cfg.y" /* yacc.c:1646  */
    {	(yyval.strval)=pkg_malloc(strlen((yyvsp[0].strval))+1);
		 					if ((yyval.strval)==0){
									LOG(L_CRIT, "ERROR: cfg. parser: out of "
											"memory.\n");
							}else{
									strncpy((yyval.strval), (yyvsp[0].strval), strlen((yyvsp[0].strval))+1);
							}
						}
#line 2157 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 369 "cfg.y" /* yacc.c:1646  */
    { (yyval.intval)=PROTO_UDP; }
#line 2163 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 370 "cfg.y" /* yacc.c:1646  */
    { (yyval.intval)=PROTO_TCP; }
#line 2169 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 371 "cfg.y" /* yacc.c:1646  */
    { (yyval.intval)=PROTO_TLS; }
#line 2175 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 372 "cfg.y" /* yacc.c:1646  */
    { (yyval.intval)=0; }
#line 2181 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 375 "cfg.y" /* yacc.c:1646  */
    { (yyval.intval)=(yyvsp[0].intval); }
#line 2187 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 376 "cfg.y" /* yacc.c:1646  */
    { (yyval.intval)=0; }
#line 2193 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 379 "cfg.y" /* yacc.c:1646  */
    { (yyval.sockid)=mk_listen_id((yyvsp[0].strval), 0, 0); }
#line 2199 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 380 "cfg.y" /* yacc.c:1646  */
    { (yyval.sockid)=mk_listen_id((yyvsp[-2].strval), 0, (yyvsp[0].intval)); }
#line 2205 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 381 "cfg.y" /* yacc.c:1646  */
    { (yyval.sockid)=mk_listen_id((yyvsp[0].strval), (yyvsp[-2].intval), 0); }
#line 2211 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 382 "cfg.y" /* yacc.c:1646  */
    { (yyval.sockid)=mk_listen_id((yyvsp[-2].strval), (yyvsp[-4].intval), (yyvsp[0].intval));}
#line 2217 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 383 "cfg.y" /* yacc.c:1646  */
    { (yyval.sockid)=0; yyerror(" port number expected"); }
#line 2223 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 386 "cfg.y" /* yacc.c:1646  */
    {  (yyval.sockid)=(yyvsp[0].sockid) ; }
#line 2229 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 387 "cfg.y" /* yacc.c:1646  */
    { (yyval.sockid)=(yyvsp[-1].sockid); (yyval.sockid)->next=(yyvsp[0].sockid); }
#line 2235 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 391 "cfg.y" /* yacc.c:1646  */
    { debug=(yyvsp[0].intval); }
#line 2241 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 392 "cfg.y" /* yacc.c:1646  */
    { yyerror("number  expected"); }
#line 2247 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 393 "cfg.y" /* yacc.c:1646  */
    { dont_fork= ! (yyvsp[0].intval); }
#line 2253 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 394 "cfg.y" /* yacc.c:1646  */
    { yyerror("boolean value expected"); }
#line 2259 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 395 "cfg.y" /* yacc.c:1646  */
    { if (!config_check) log_stderr=(yyvsp[0].intval); }
#line 2265 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 396 "cfg.y" /* yacc.c:1646  */
    { yyerror("boolean value expected"); }
#line 2271 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 397 "cfg.y" /* yacc.c:1646  */
    {
					if ( (i_tmp=str2facility((yyvsp[0].strval)))==-1)
						yyerror("bad facility (see syslog(3) man page)");
					if (!config_check)
						log_facility=i_tmp;
									}
#line 2282 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 403 "cfg.y" /* yacc.c:1646  */
    { yyerror("ID expected"); }
#line 2288 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 404 "cfg.y" /* yacc.c:1646  */
    { received_dns|= ((yyvsp[0].intval))?DO_DNS:0; }
#line 2294 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 405 "cfg.y" /* yacc.c:1646  */
    { yyerror("boolean value expected"); }
#line 2300 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 406 "cfg.y" /* yacc.c:1646  */
    { received_dns|= ((yyvsp[0].intval))?DO_REV_DNS:0; }
#line 2306 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 407 "cfg.y" /* yacc.c:1646  */
    { yyerror("boolean value expected"); }
#line 2312 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 408 "cfg.y" /* yacc.c:1646  */
    { port_no=(yyvsp[0].intval); }
#line 2318 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 409 "cfg.y" /* yacc.c:1646  */
    {
					#ifdef STATS
							stat_file=(yyvsp[0].strval);
					#endif
							}
#line 2328 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 414 "cfg.y" /* yacc.c:1646  */
    { maxbuffer=(yyvsp[0].intval); }
#line 2334 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 415 "cfg.y" /* yacc.c:1646  */
    { yyerror("number expected"); }
#line 2340 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 416 "cfg.y" /* yacc.c:1646  */
    { yyerror("number expected"); }
#line 2346 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 417 "cfg.y" /* yacc.c:1646  */
    { children_no=(yyvsp[0].intval); }
#line 2352 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 418 "cfg.y" /* yacc.c:1646  */
    { yyerror("number expected"); }
#line 2358 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 419 "cfg.y" /* yacc.c:1646  */
    { check_via=(yyvsp[0].intval); }
#line 2364 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 420 "cfg.y" /* yacc.c:1646  */
    { yyerror("boolean value expected"); }
#line 2370 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 421 "cfg.y" /* yacc.c:1646  */
    { ws_duplicate_routeno=(yyvsp[0].intval); }
#line 2376 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 422 "cfg.y" /* yacc.c:1646  */
    { yyerror("int value expected"); }
#line 2382 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 423 "cfg.y" /* yacc.c:1646  */
    { ws_ctccall_flag=(yyvsp[0].intval); }
#line 2388 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 424 "cfg.y" /* yacc.c:1646  */
    { yyerror("int value expected"); }
#line 2394 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 425 "cfg.y" /* yacc.c:1646  */
    { wsdb_fifo_count=(yyvsp[0].intval); }
#line 2400 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 426 "cfg.y" /* yacc.c:1646  */
    { yyerror("int value expected"); }
#line 2406 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 427 "cfg.y" /* yacc.c:1646  */
    { ws_acc_db_user=(yyvsp[0].strval); }
#line 2412 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 428 "cfg.y" /* yacc.c:1646  */
    { yyerror("string value expected"); }
#line 2418 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 429 "cfg.y" /* yacc.c:1646  */
    { ws_acc_db_host=(yyvsp[0].strval); }
#line 2424 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 430 "cfg.y" /* yacc.c:1646  */
    { yyerror("string value expected"); }
#line 2430 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 431 "cfg.y" /* yacc.c:1646  */
    { ws_acc_db_pswd=(yyvsp[0].strval); }
#line 2436 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 432 "cfg.y" /* yacc.c:1646  */
    { yyerror("string value expected"); }
#line 2442 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 433 "cfg.y" /* yacc.c:1646  */
    { ws_acc_db_name=(yyvsp[0].strval); }
#line 2448 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 434 "cfg.y" /* yacc.c:1646  */
    { yyerror("string value expected"); }
#line 2454 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 435 "cfg.y" /* yacc.c:1646  */
    { ws_db_fifo_name=(yyvsp[0].strval); }
#line 2460 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 436 "cfg.y" /* yacc.c:1646  */
    { yyerror("string value expected"); }
#line 2466 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 437 "cfg.y" /* yacc.c:1646  */
    { syn_branch=(yyvsp[0].intval); }
#line 2472 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 438 "cfg.y" /* yacc.c:1646  */
    { yyerror("boolean value expected"); }
#line 2478 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 439 "cfg.y" /* yacc.c:1646  */
    { memlog=(yyvsp[0].intval); }
#line 2484 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 440 "cfg.y" /* yacc.c:1646  */
    { yyerror("int value expected"); }
#line 2490 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 441 "cfg.y" /* yacc.c:1646  */
    { sip_warning=(yyvsp[0].intval); }
#line 2496 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 442 "cfg.y" /* yacc.c:1646  */
    { yyerror("boolean value expected"); }
#line 2502 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 443 "cfg.y" /* yacc.c:1646  */
    { fifo=(yyvsp[0].strval); }
#line 2508 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 444 "cfg.y" /* yacc.c:1646  */
    { yyerror("string value expected"); }
#line 2514 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 445 "cfg.y" /* yacc.c:1646  */
    { fifo_dir=(yyvsp[0].strval); }
#line 2520 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 446 "cfg.y" /* yacc.c:1646  */
    { yyerror("string value expected"); }
#line 2526 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 447 "cfg.y" /* yacc.c:1646  */
    { sock_mode=(yyvsp[0].intval); }
#line 2532 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 448 "cfg.y" /* yacc.c:1646  */
    { yyerror("int value expected"); }
#line 2538 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 449 "cfg.y" /* yacc.c:1646  */
    { sock_user=(yyvsp[0].strval); }
#line 2544 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 450 "cfg.y" /* yacc.c:1646  */
    { sock_user=(yyvsp[0].strval); }
#line 2550 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 451 "cfg.y" /* yacc.c:1646  */
    { yyerror("string value expected"); }
#line 2556 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 452 "cfg.y" /* yacc.c:1646  */
    { sock_group=(yyvsp[0].strval); }
#line 2562 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 453 "cfg.y" /* yacc.c:1646  */
    { sock_group=(yyvsp[0].strval); }
#line 2568 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 454 "cfg.y" /* yacc.c:1646  */
    { yyerror("string value expected"); }
#line 2574 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 455 "cfg.y" /* yacc.c:1646  */
    { fifo_db_url=(yyvsp[0].strval); }
#line 2580 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 456 "cfg.y" /* yacc.c:1646  */
    { yyerror("string value expected"); }
#line 2586 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 457 "cfg.y" /* yacc.c:1646  */
    { unixsock_name=(yyvsp[0].strval); }
#line 2592 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 458 "cfg.y" /* yacc.c:1646  */
    { yyerror("string value expected"); }
#line 2598 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 459 "cfg.y" /* yacc.c:1646  */
    { unixsock_children=(yyvsp[0].intval); }
#line 2604 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 460 "cfg.y" /* yacc.c:1646  */
    { yyerror("int value expected\n"); }
#line 2610 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 461 "cfg.y" /* yacc.c:1646  */
    { unixsock_tx_timeout=(yyvsp[0].intval); }
#line 2616 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 462 "cfg.y" /* yacc.c:1646  */
    { yyerror("int value expected\n"); }
#line 2622 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 463 "cfg.y" /* yacc.c:1646  */
    { user=(yyvsp[0].strval); }
#line 2628 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 464 "cfg.y" /* yacc.c:1646  */
    { user=(yyvsp[0].strval); }
#line 2634 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 465 "cfg.y" /* yacc.c:1646  */
    { yyerror("string value expected"); }
#line 2640 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 466 "cfg.y" /* yacc.c:1646  */
    { group=(yyvsp[0].strval); }
#line 2646 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 467 "cfg.y" /* yacc.c:1646  */
    { group=(yyvsp[0].strval); }
#line 2652 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 468 "cfg.y" /* yacc.c:1646  */
    { yyerror("string value expected"); }
#line 2658 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 469 "cfg.y" /* yacc.c:1646  */
    { chroot_dir=(yyvsp[0].strval); }
#line 2664 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 470 "cfg.y" /* yacc.c:1646  */
    { chroot_dir=(yyvsp[0].strval); }
#line 2670 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 471 "cfg.y" /* yacc.c:1646  */
    { yyerror("string value expected"); }
#line 2676 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 472 "cfg.y" /* yacc.c:1646  */
    { working_dir=(yyvsp[0].strval); }
#line 2682 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 473 "cfg.y" /* yacc.c:1646  */
    { working_dir=(yyvsp[0].strval); }
#line 2688 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 474 "cfg.y" /* yacc.c:1646  */
    { yyerror("string value expected"); }
#line 2694 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 475 "cfg.y" /* yacc.c:1646  */
    { mhomed=(yyvsp[0].intval); }
#line 2700 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 476 "cfg.y" /* yacc.c:1646  */
    { yyerror("boolean value expected"); }
#line 2706 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 477 "cfg.y" /* yacc.c:1646  */
    {
									#ifdef USE_TCP
										tcp_disable=(yyvsp[0].intval);
									#else
										warn("tcp support not compiled in");
									#endif
									}
#line 2718 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 484 "cfg.y" /* yacc.c:1646  */
    { yyerror("boolean value expected"); }
#line 2724 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 485 "cfg.y" /* yacc.c:1646  */
    {
									#ifdef USE_TCP
										tcp_accept_aliases=(yyvsp[0].intval);
									#else
										warn("tcp support not compiled in");
									#endif
									}
#line 2736 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 492 "cfg.y" /* yacc.c:1646  */
    { yyerror("boolean value expected"); }
#line 2742 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 493 "cfg.y" /* yacc.c:1646  */
    {
									#ifdef USE_TCP
										tcp_children_no=(yyvsp[0].intval);
									#else
										warn("tcp support not compiled in");
									#endif
									}
#line 2754 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 500 "cfg.y" /* yacc.c:1646  */
    { yyerror("number expected"); }
#line 2760 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 501 "cfg.y" /* yacc.c:1646  */
    {
									#ifdef USE_TCP
										tcp_connect_timeout=(yyvsp[0].intval);
									#else
										warn("tcp support not compiled in");
									#endif
									}
#line 2772 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 508 "cfg.y" /* yacc.c:1646  */
    { yyerror("number expected"); }
#line 2778 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 509 "cfg.y" /* yacc.c:1646  */
    {
									#ifdef USE_TCP
										tcp_send_timeout=(yyvsp[0].intval);
									#else
										warn("tcp support not compiled in");
									#endif
									}
#line 2790 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 516 "cfg.y" /* yacc.c:1646  */
    { yyerror("number expected"); }
#line 2796 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 517 "cfg.y" /* yacc.c:1646  */
    {
									#ifdef USE_TLS
										tls_disable=(yyvsp[0].intval);
									#else
										warn("tls support not compiled in");
									#endif
									}
#line 2808 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 524 "cfg.y" /* yacc.c:1646  */
    { yyerror("boolean value expected"); }
#line 2814 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 525 "cfg.y" /* yacc.c:1646  */
    { 
									#ifdef USE_TLS
										tls_log=(yyvsp[0].intval);
									#else
										warn("tls support not compiled in");
									#endif
									}
#line 2826 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 532 "cfg.y" /* yacc.c:1646  */
    { yyerror("int value expected"); }
#line 2832 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 533 "cfg.y" /* yacc.c:1646  */
    {
									#ifdef USE_TLS
										tls_port_no=(yyvsp[0].intval);
									#else
										warn("tls support not compiled in");
									#endif
									}
#line 2844 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 540 "cfg.y" /* yacc.c:1646  */
    { yyerror("number expected"); }
#line 2850 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 541 "cfg.y" /* yacc.c:1646  */
    {
									#ifdef USE_TLS
										tls_method=TLS_USE_SSLv23;
									#else
										warn("tls support not compiled in");
									#endif
									}
#line 2862 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 548 "cfg.y" /* yacc.c:1646  */
    {
									#ifdef USE_TLS
										tls_method=TLS_USE_SSLv2;
									#else
										warn("tls support not compiled in");
									#endif
									}
#line 2874 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 555 "cfg.y" /* yacc.c:1646  */
    {
									#ifdef USE_TLS
										tls_method=TLS_USE_SSLv3;
									#else
										warn("tls support not compiled in");
									#endif
									}
#line 2886 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 562 "cfg.y" /* yacc.c:1646  */
    {
									#ifdef USE_TLS
										tls_method=TLS_USE_TLSv1;
									#else
										warn("tls support not compiled in");
									#endif
									}
#line 2898 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 569 "cfg.y" /* yacc.c:1646  */
    {
									#ifdef USE_TLS
										yyerror("SSLv23, SSLv2, SSLv3 or TLSv1"
													" expected");
									#else
										warn("tls support not compiled in");
									#endif
									}
#line 2911 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 578 "cfg.y" /* yacc.c:1646  */
    {
									#ifdef USE_TLS
										tls_verify_cert=(yyvsp[0].intval);
									#else
										warn("tls support not compiled in");
									#endif
									}
#line 2923 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 585 "cfg.y" /* yacc.c:1646  */
    { yyerror("boolean value expected"); }
#line 2929 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 131:
#line 586 "cfg.y" /* yacc.c:1646  */
    {
									#ifdef USE_TLS
										tls_require_cert=(yyvsp[0].intval);
									#else
										warn( "tls support not compiled in");
									#endif
									}
#line 2941 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 593 "cfg.y" /* yacc.c:1646  */
    { yyerror("boolean value"
																" expected"); }
#line 2948 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 595 "cfg.y" /* yacc.c:1646  */
    { 
									#ifdef USE_TLS
											tls_cert_file=(yyvsp[0].strval);
									#else
										warn("tls support not compiled in");
									#endif
									}
#line 2960 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 134:
#line 602 "cfg.y" /* yacc.c:1646  */
    { yyerror("string value expected"); }
#line 2966 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 603 "cfg.y" /* yacc.c:1646  */
    { 
									#ifdef USE_TLS
											tls_pkey_file=(yyvsp[0].strval);
									#else
										warn("tls support not compiled in");
									#endif
									}
#line 2978 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 610 "cfg.y" /* yacc.c:1646  */
    { yyerror("string value expected"); }
#line 2984 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 611 "cfg.y" /* yacc.c:1646  */
    { 
									#ifdef USE_TLS
											tls_ca_file=(yyvsp[0].strval);
									#else
										warn("tls support not compiled in");
									#endif
									}
#line 2996 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 618 "cfg.y" /* yacc.c:1646  */
    { yyerror("string value expected"); }
#line 3002 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 619 "cfg.y" /* yacc.c:1646  */
    {
									#ifdef USE_TLS
										tls_handshake_timeout=(yyvsp[0].intval);
									#else
										warn("tls support not compiled in");
									#endif
									}
#line 3014 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 626 "cfg.y" /* yacc.c:1646  */
    { yyerror("number expected"); }
#line 3020 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 627 "cfg.y" /* yacc.c:1646  */
    {
									#ifdef USE_TLS
										tls_send_timeout=(yyvsp[0].intval);
									#else
										warn("tls support not compiled in");
									#endif
									}
#line 3032 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 634 "cfg.y" /* yacc.c:1646  */
    { yyerror("number expected"); }
#line 3038 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 635 "cfg.y" /* yacc.c:1646  */
    { server_signature=(yyvsp[0].intval); }
#line 3044 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 636 "cfg.y" /* yacc.c:1646  */
    { yyerror("boolean value expected"); }
#line 3050 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 637 "cfg.y" /* yacc.c:1646  */
    { reply_to_via=(yyvsp[0].intval); }
#line 3056 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 638 "cfg.y" /* yacc.c:1646  */
    { yyerror("boolean value expected"); }
#line 3062 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 639 "cfg.y" /* yacc.c:1646  */
    {
							for(lst_tmp=(yyvsp[0].sockid); lst_tmp; lst_tmp=lst_tmp->next){
								if (add_listen_iface(	lst_tmp->name,
														lst_tmp->port,
														lst_tmp->proto,
														0
													)!=0){
									LOG(L_CRIT,  "ERROR: cfg. parser: failed"
											" to add listen address\n");
									break;
								}
							}
							 }
#line 3080 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 652 "cfg.y" /* yacc.c:1646  */
    { yyerror("ip address or hostname"
						"expected"); }
#line 3087 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 149:
#line 654 "cfg.y" /* yacc.c:1646  */
    { 
							for(lst_tmp=(yyvsp[0].sockid); lst_tmp; lst_tmp=lst_tmp->next)
								add_alias(lst_tmp->name, strlen(lst_tmp->name),
											lst_tmp->port, lst_tmp->proto);
							  }
#line 3097 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 659 "cfg.y" /* yacc.c:1646  */
    { yyerror(" hostname expected"); }
#line 3103 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 660 "cfg.y" /* yacc.c:1646  */
    {
								default_global_address.s=(yyvsp[0].strval);
								default_global_address.len=strlen((yyvsp[0].strval));
								}
#line 3112 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 664 "cfg.y" /* yacc.c:1646  */
    {yyerror("ip address or hostname "
												"expected"); }
#line 3119 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 666 "cfg.y" /* yacc.c:1646  */
    {
								tmp=int2str((yyvsp[0].intval), &i_tmp);
								if ((default_global_port.s=pkg_malloc(i_tmp))
										==0){
										LOG(L_CRIT, "ERROR: cfg. parser:"
													" out of memory.\n");
										default_global_port.len=0;
								}else{
									default_global_port.len=i_tmp;
									memcpy(default_global_port.s, tmp,
											default_global_port.len);
								};
								}
#line 3137 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 679 "cfg.y" /* yacc.c:1646  */
    {yyerror("ip address or hostname "
												"expected"); }
#line 3144 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 681 "cfg.y" /* yacc.c:1646  */
    {
										disable_core_dump=(yyvsp[0].intval);
									}
#line 3152 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 156:
#line 684 "cfg.y" /* yacc.c:1646  */
    { yyerror("boolean value expected"); }
#line 3158 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 157:
#line 685 "cfg.y" /* yacc.c:1646  */
    {
										open_files_limit=(yyvsp[0].intval);
									}
#line 3166 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 158:
#line 688 "cfg.y" /* yacc.c:1646  */
    { yyerror("number expected"); }
#line 3172 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 159:
#line 689 "cfg.y" /* yacc.c:1646  */
    {
								#ifdef USE_MCAST
										mcast_loopback=(yyvsp[0].intval);
								#else
									warn("no multicast support compiled in");
								#endif
		  }
#line 3184 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 160:
#line 696 "cfg.y" /* yacc.c:1646  */
    { yyerror("boolean value expected"); }
#line 3190 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 161:
#line 697 "cfg.y" /* yacc.c:1646  */
    {
								#ifdef USE_MCAST
										mcast_ttl=(yyvsp[0].intval);
								#else
									warn("no multicast support compiled in");
								#endif
		  }
#line 3202 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 162:
#line 704 "cfg.y" /* yacc.c:1646  */
    { yyerror("number expected"); }
#line 3208 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 163:
#line 705 "cfg.y" /* yacc.c:1646  */
    { yyerror("unknown config variable"); }
#line 3214 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 164:
#line 708 "cfg.y" /* yacc.c:1646  */
    { DBG("loading module %s\n", (yyvsp[0].strval));
		  						  if (load_module((yyvsp[0].strval))!=0){
								  		yyerror("failed to load module");
								  }
								}
#line 3224 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 165:
#line 713 "cfg.y" /* yacc.c:1646  */
    { yyerror("string expected");  }
#line 3230 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 166:
#line 714 "cfg.y" /* yacc.c:1646  */
    {
			 if (set_mod_param_regex((yyvsp[-5].strval), (yyvsp[-3].strval), STR_PARAM, (yyvsp[-1].strval)) != 0) {
				 yyerror("Can't set module parameter");
			 }
		   }
#line 3240 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 167:
#line 719 "cfg.y" /* yacc.c:1646  */
    {
			 if (set_mod_param_regex((yyvsp[-5].strval), (yyvsp[-3].strval), INT_PARAM, (void*)(yyvsp[-1].intval)) != 0) {
				 yyerror("Can't set module parameter");
			 }
		   }
#line 3250 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 168:
#line 724 "cfg.y" /* yacc.c:1646  */
    { yyerror("Invalid arguments"); }
#line 3256 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 169:
#line 728 "cfg.y" /* yacc.c:1646  */
    { (yyval.ipaddr)=(yyvsp[0].ipaddr); }
#line 3262 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 170:
#line 729 "cfg.y" /* yacc.c:1646  */
    { (yyval.ipaddr)=(yyvsp[0].ipaddr); }
#line 3268 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 171:
#line 732 "cfg.y" /* yacc.c:1646  */
    { 
											(yyval.ipaddr)=pkg_malloc(
													sizeof(struct ip_addr));
											if ((yyval.ipaddr)==0){
												LOG(L_CRIT, "ERROR: cfg. "
													"parser: out of memory.\n"
													);
											}else{
												memset((yyval.ipaddr), 0, 
													sizeof(struct ip_addr));
												(yyval.ipaddr)->af=AF_INET;
												(yyval.ipaddr)->len=4;
												if (((yyvsp[-6].intval)>255) || ((yyvsp[-6].intval)<0) ||
													((yyvsp[-4].intval)>255) || ((yyvsp[-4].intval)<0) ||
													((yyvsp[-2].intval)>255) || ((yyvsp[-2].intval)<0) ||
													((yyvsp[0].intval)>255) || ((yyvsp[0].intval)<0)){
													yyerror("invalid ipv4"
															"address");
													(yyval.ipaddr)->u.addr32[0]=0;
													/* $$=0; */
												}else{
													(yyval.ipaddr)->u.addr[0]=(yyvsp[-6].intval);
													(yyval.ipaddr)->u.addr[1]=(yyvsp[-4].intval);
													(yyval.ipaddr)->u.addr[2]=(yyvsp[-2].intval);
													(yyval.ipaddr)->u.addr[3]=(yyvsp[0].intval);
													/*
													$$=htonl( ($1<<24)|
													($3<<16)| ($5<<8)|$7 );
													*/
												}
											}
												}
#line 3305 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 172:
#line 766 "cfg.y" /* yacc.c:1646  */
    {
					(yyval.ipaddr)=pkg_malloc(sizeof(struct ip_addr));
					if ((yyval.ipaddr)==0){
						LOG(L_CRIT, "ERROR: cfg. parser: out of memory.\n");
					}else{
						memset((yyval.ipaddr), 0, sizeof(struct ip_addr));
						(yyval.ipaddr)->af=AF_INET6;
						(yyval.ipaddr)->len=16;
					#ifdef USE_IPV6
						if (inet_pton(AF_INET6, (yyvsp[0].strval), (yyval.ipaddr)->u.addr)<=0){
							yyerror("bad ipv6 address");
						}
					#else
						yyerror("ipv6 address & no ipv6 support compiled in");
						YYABORT;
					#endif
					}
				}
#line 3328 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 173:
#line 786 "cfg.y" /* yacc.c:1646  */
    { (yyval.ipaddr)=(yyvsp[0].ipaddr); }
#line 3334 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 174:
#line 787 "cfg.y" /* yacc.c:1646  */
    {(yyval.ipaddr)=(yyvsp[-1].ipaddr); }
#line 3340 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 175:
#line 791 "cfg.y" /* yacc.c:1646  */
    { push((yyvsp[-1].action), &rlist[DEFAULT_RT]); }
#line 3346 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 176:
#line 793 "cfg.y" /* yacc.c:1646  */
    { 
										if (((yyvsp[-4].intval)<RT_NO) && ((yyvsp[-4].intval)>=0)){
											push((yyvsp[-1].action), &rlist[(yyvsp[-4].intval)]);
										}else{
											yyerror("invalid routing "
													"table number");
											YYABORT; }
										}
#line 3359 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 177:
#line 801 "cfg.y" /* yacc.c:1646  */
    { yyerror("invalid  route  statement"); }
#line 3365 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 178:
#line 804 "cfg.y" /* yacc.c:1646  */
    {
										if (((yyvsp[-4].intval)<FAILURE_RT_NO)&&((yyvsp[-4].intval)>=1)){
											push((yyvsp[-1].action), &failure_rlist[(yyvsp[-4].intval)]);
										} else {
											yyerror("invalid reply routing"
												"table number");
											YYABORT; }
										}
#line 3378 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 179:
#line 812 "cfg.y" /* yacc.c:1646  */
    { yyerror("invalid failure_route statement"); }
#line 3384 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 180:
#line 815 "cfg.y" /* yacc.c:1646  */
    {
										if (((yyvsp[-4].intval)<ONREPLY_RT_NO)&&((yyvsp[-4].intval)>=1)){
											push((yyvsp[-1].action), &onreply_rlist[(yyvsp[-4].intval)]);
										} else {
											yyerror("invalid reply routing"
												"table number");
											YYABORT; }
										}
#line 3397 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 181:
#line 823 "cfg.y" /* yacc.c:1646  */
    { yyerror("invalid onreply_route statement"); }
#line 3403 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 182:
#line 845 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=mk_exp(AND_OP, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3409 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 183:
#line 846 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=mk_exp(OR_OP, (yyvsp[-2].expr), (yyvsp[0].expr));  }
#line 3415 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 184:
#line 847 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=mk_exp(NOT_OP, (yyvsp[0].expr), 0);  }
#line 3421 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 185:
#line 848 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=(yyvsp[-1].expr); }
#line 3427 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 186:
#line 849 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=(yyvsp[0].expr); }
#line 3433 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 187:
#line 852 "cfg.y" /* yacc.c:1646  */
    {(yyval.intval)=EQUAL_OP; }
#line 3439 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 188:
#line 853 "cfg.y" /* yacc.c:1646  */
    {(yyval.intval)=DIFF_OP; }
#line 3445 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 189:
#line 856 "cfg.y" /* yacc.c:1646  */
    {(yyval.intval)=(yyvsp[0].intval); }
#line 3451 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 190:
#line 857 "cfg.y" /* yacc.c:1646  */
    {(yyval.intval)=GT_OP; }
#line 3457 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 191:
#line 858 "cfg.y" /* yacc.c:1646  */
    {(yyval.intval)=LT_OP; }
#line 3463 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 192:
#line 859 "cfg.y" /* yacc.c:1646  */
    {(yyval.intval)=GTE_OP; }
#line 3469 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 193:
#line 860 "cfg.y" /* yacc.c:1646  */
    {(yyval.intval)=LTE_OP; }
#line 3475 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 194:
#line 863 "cfg.y" /* yacc.c:1646  */
    {(yyval.intval)=(yyvsp[0].intval); }
#line 3481 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 195:
#line 864 "cfg.y" /* yacc.c:1646  */
    {(yyval.intval)=MATCH_OP; }
#line 3487 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 196:
#line 867 "cfg.y" /* yacc.c:1646  */
    {(yyval.intval)=URI_O;}
#line 3493 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 197:
#line 868 "cfg.y" /* yacc.c:1646  */
    {(yyval.intval)=FROM_URI_O;}
#line 3499 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 198:
#line 869 "cfg.y" /* yacc.c:1646  */
    {(yyval.intval)=TO_URI_O;}
#line 3505 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 199:
#line 872 "cfg.y" /* yacc.c:1646  */
    {(yyval.expr)= mk_elem(	(yyvsp[-1].intval), STRING_ST, 
													METHOD_O, (yyvsp[0].strval));
									}
#line 3513 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 200:
#line 875 "cfg.y" /* yacc.c:1646  */
    {(yyval.expr) = mk_elem(	(yyvsp[-1].intval), STRING_ST,
											METHOD_O, (yyvsp[0].strval)); 
				 			}
#line 3521 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 201:
#line 878 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=0; yyerror("string expected"); }
#line 3527 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 202:
#line 879 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=0; yyerror("invalid operator,"
										"== , !=, or =~ expected");
						}
#line 3535 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 203:
#line 882 "cfg.y" /* yacc.c:1646  */
    {(yyval.expr) = mk_elem(	(yyvsp[-1].intval), STRING_ST,
												(yyvsp[-2].intval), (yyvsp[0].strval)); 
				 				}
#line 3543 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 204:
#line 885 "cfg.y" /* yacc.c:1646  */
    {(yyval.expr) = mk_elem(	(yyvsp[-1].intval), STRING_ST,
											(yyvsp[-2].intval), (yyvsp[0].strval)); 
				 			}
#line 3551 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 205:
#line 888 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), MYSELF_ST,
													(yyvsp[-2].intval), 0);
								}
#line 3559 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 206:
#line 891 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=0; yyerror("string or MYSELF expected"); }
#line 3565 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 207:
#line 892 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=0; yyerror("invalid operator,"
									" == , != or =~ expected");
					}
#line 3573 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 208:
#line 895 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), NUMBER_ST,
												SRCPORT_O, (void *) (yyvsp[0].intval) ); }
#line 3580 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 209:
#line 897 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=0; yyerror("number expected"); }
#line 3586 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 210:
#line 898 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=0; yyerror("==, !=, <,>, >= or <=  expected"); }
#line 3592 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 211:
#line 899 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), NUMBER_ST,
												DSTPORT_O, (void *) (yyvsp[0].intval) ); }
#line 3599 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 212:
#line 901 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=0; yyerror("number expected"); }
#line 3605 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 213:
#line 902 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=0; yyerror("==, !=, <,>, >= or <=  expected"); }
#line 3611 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 214:
#line 903 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), NUMBER_ST,
												PROTO_O, (void *) (yyvsp[0].intval) ); }
#line 3618 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 215:
#line 905 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=0;
								yyerror("protocol expected (udp, tcp or tls)");
							}
#line 3626 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 216:
#line 908 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=0; yyerror("equal/!= operator expected"); }
#line 3632 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 217:
#line 909 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), NUMBER_ST,
												AF_O, (void *) (yyvsp[0].intval) ); }
#line 3639 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 218:
#line 911 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=0; yyerror("number expected"); }
#line 3645 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 219:
#line 912 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=0; yyerror("equal/!= operator expected"); }
#line 3651 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 220:
#line 913 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), NUMBER_ST,
												MSGLEN_O, (void *) (yyvsp[0].intval) ); }
#line 3658 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 221:
#line 915 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), NUMBER_ST,
												MSGLEN_O, (void *) BUF_SIZE); }
#line 3665 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 222:
#line 917 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=0; yyerror("number expected"); }
#line 3671 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 223:
#line 918 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=0; yyerror("equal/!= operator expected"); }
#line 3677 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 224:
#line 919 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), NET_ST,
												SRCIP_O, (yyvsp[0].ipnet));
								}
#line 3685 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 225:
#line 922 "cfg.y" /* yacc.c:1646  */
    {	s_tmp.s=(yyvsp[0].strval);
									s_tmp.len=strlen((yyvsp[0].strval));
									ip_tmp=str2ip(&s_tmp);
									if (ip_tmp==0)
										ip_tmp=str2ip6(&s_tmp);
									if (ip_tmp){
										(yyval.expr)=mk_elem(	(yyvsp[-1].intval), NET_ST, SRCIP_O,
												mk_net_bitlen(ip_tmp, 
														ip_tmp->len*8) );
									}else{
										(yyval.expr)=mk_elem(	(yyvsp[-1].intval), STRING_ST,
												SRCIP_O, (yyvsp[0].strval));
									}
								}
#line 3704 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 226:
#line 936 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), STRING_ST,
												SRCIP_O, (yyvsp[0].strval));
								}
#line 3712 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 227:
#line 939 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), MYSELF_ST,
												SRCIP_O, 0);
								}
#line 3720 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 228:
#line 942 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=0; yyerror( "ip address or hostname"
						 "expected" ); }
#line 3727 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 229:
#line 944 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=0; 
						 yyerror("invalid operator, ==, != or =~ expected");}
#line 3734 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 230:
#line 946 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), NET_ST,
												DSTIP_O, (yyvsp[0].ipnet));
								}
#line 3742 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 231:
#line 949 "cfg.y" /* yacc.c:1646  */
    {	s_tmp.s=(yyvsp[0].strval);
									s_tmp.len=strlen((yyvsp[0].strval));
									ip_tmp=str2ip(&s_tmp);
									if (ip_tmp==0)
										ip_tmp=str2ip6(&s_tmp);
									if (ip_tmp){
										(yyval.expr)=mk_elem(	(yyvsp[-1].intval), NET_ST, DSTIP_O,
												mk_net_bitlen(ip_tmp, 
														ip_tmp->len*8) );
									}else{
										(yyval.expr)=mk_elem(	(yyvsp[-1].intval), STRING_ST,
												DSTIP_O, (yyvsp[0].strval));
									}
								}
#line 3761 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 232:
#line 963 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), STRING_ST,
												DSTIP_O, (yyvsp[0].strval));
								}
#line 3769 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 233:
#line 966 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), MYSELF_ST,
												DSTIP_O, 0);
								}
#line 3777 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 234:
#line 969 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=0; yyerror( "ip address or hostname"
						 			"expected" ); }
#line 3784 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 235:
#line 971 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=0; 
						yyerror("invalid operator, ==, != or =~ expected");}
#line 3791 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 236:
#line 973 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), MYSELF_ST,
													(yyvsp[0].intval), 0);
								}
#line 3799 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 237:
#line 976 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), MYSELF_ST,
												SRCIP_O, 0);
								}
#line 3807 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 238:
#line 979 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), MYSELF_ST,
												DSTIP_O, 0);
								}
#line 3815 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 239:
#line 982 "cfg.y" /* yacc.c:1646  */
    {	(yyval.expr)=0; 
									yyerror(" URI, SRCIP or DSTIP expected"); }
#line 3822 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 240:
#line 984 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=0; 
							yyerror ("invalid operator, == or != expected");
						}
#line 3830 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 241:
#line 987 "cfg.y" /* yacc.c:1646  */
    { (yyval.expr)=mk_elem( NO_OP, ACTIONS_ST, ACTION_O, (yyvsp[0].action) );  }
#line 3836 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 242:
#line 988 "cfg.y" /* yacc.c:1646  */
    {(yyval.expr)=mk_elem( NO_OP, NUMBER_ST, NUMBER_O, (void*)(yyvsp[0].intval) ); }
#line 3842 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 243:
#line 991 "cfg.y" /* yacc.c:1646  */
    { (yyval.ipnet)=mk_net((yyvsp[-2].ipaddr), (yyvsp[0].ipaddr)); }
#line 3848 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 244:
#line 992 "cfg.y" /* yacc.c:1646  */
    {	if (((yyvsp[0].intval)<0) || ((yyvsp[0].intval)>(yyvsp[-2].ipaddr)->len*8)){
								yyerror("invalid bit number in netmask");
								(yyval.ipnet)=0;
							}else{
								(yyval.ipnet)=mk_net_bitlen((yyvsp[-2].ipaddr), (yyvsp[0].intval));
							/*
								$$=mk_net($1, 
										htonl( ($3)?~( (1<<(32-$3))-1 ):0 ) );
							*/
							}
						}
#line 3864 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 245:
#line 1003 "cfg.y" /* yacc.c:1646  */
    { (yyval.ipnet)=mk_net_bitlen((yyvsp[0].ipaddr), (yyvsp[0].ipaddr)->len*8); }
#line 3870 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 246:
#line 1004 "cfg.y" /* yacc.c:1646  */
    { (yyval.ipnet)=0;
						 yyerror("netmask (eg:255.0.0.0 or 8) expected");
						}
#line 3878 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 247:
#line 1011 "cfg.y" /* yacc.c:1646  */
    {(yyval.strval)=".";}
#line 3884 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 248:
#line 1012 "cfg.y" /* yacc.c:1646  */
    {(yyval.strval)="-"; }
#line 3890 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 249:
#line 1015 "cfg.y" /* yacc.c:1646  */
    { (yyval.strval)=(yyvsp[0].strval); }
#line 3896 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 250:
#line 1016 "cfg.y" /* yacc.c:1646  */
    { (yyval.strval)=(char*)pkg_malloc(strlen((yyvsp[-2].strval))+1+strlen((yyvsp[0].strval))+1);
						  if ((yyval.strval)==0){
						  	LOG(L_CRIT, "ERROR: cfg. parser: memory allocation"
										" failure while parsing host\n");
						  }else{
							memcpy((yyval.strval), (yyvsp[-2].strval), strlen((yyvsp[-2].strval)));
							(yyval.strval)[strlen((yyvsp[-2].strval))]=*(yyvsp[-1].strval);
							memcpy((yyval.strval)+strlen((yyvsp[-2].strval))+1, (yyvsp[0].strval), strlen((yyvsp[0].strval)));
							(yyval.strval)[strlen((yyvsp[-2].strval))+1+strlen((yyvsp[0].strval))]=0;
						  }
						  pkg_free((yyvsp[-2].strval)); pkg_free((yyvsp[0].strval));
						}
#line 3913 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 251:
#line 1028 "cfg.y" /* yacc.c:1646  */
    { (yyval.strval)=0; pkg_free((yyvsp[-2].strval)); yyerror("invalid hostname"); }
#line 3919 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 252:
#line 1032 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=(yyvsp[0].action); }
#line 3925 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 253:
#line 1033 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=(yyvsp[0].action); }
#line 3931 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 254:
#line 1034 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=(yyvsp[-1].action); }
#line 3937 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 255:
#line 1037 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=(yyvsp[0].action); }
#line 3943 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 256:
#line 1038 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=(yyvsp[-1].action); }
#line 3949 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 257:
#line 1041 "cfg.y" /* yacc.c:1646  */
    {(yyval.action)=append_action((yyvsp[-1].action), (yyvsp[0].action)); }
#line 3955 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 258:
#line 1042 "cfg.y" /* yacc.c:1646  */
    {(yyval.action)=(yyvsp[0].action);}
#line 3961 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 259:
#line 1043 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("bad command"); }
#line 3967 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 260:
#line 1046 "cfg.y" /* yacc.c:1646  */
    {(yyval.action)=(yyvsp[-1].action);}
#line 3973 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 261:
#line 1047 "cfg.y" /* yacc.c:1646  */
    {(yyval.action)=(yyvsp[0].action);}
#line 3979 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 262:
#line 1048 "cfg.y" /* yacc.c:1646  */
    {(yyval.action)=0;}
#line 3985 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 263:
#line 1049 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("bad command: missing ';'?"); }
#line 3991 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 264:
#line 1052 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action3( IF_T,
													 EXPR_ST,
													 ACTIONS_ST,
													 NOSUBTYPE,
													 (yyvsp[-1].expr),
													 (yyvsp[0].action),
													 0);
									}
#line 4004 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 265:
#line 1060 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action3( IF_T,
													 EXPR_ST,
													 ACTIONS_ST,
													 ACTIONS_ST,
													 (yyvsp[-3].expr),
													 (yyvsp[-2].action),
													 (yyvsp[0].action));
									}
#line 4017 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 266:
#line 1070 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(	FORWARD_T,
														STRING_ST,
														NUMBER_ST,
														(yyvsp[-1].strval),
														0);
										}
#line 4028 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 267:
#line 1076 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(	FORWARD_T,
														STRING_ST,
														NUMBER_ST,
														(yyvsp[-1].strval),
														0);
										}
#line 4039 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 268:
#line 1082 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(	FORWARD_T,
														IP_ST,
														NUMBER_ST,
														(void*)(yyvsp[-1].ipaddr),
														0);
										}
#line 4050 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 269:
#line 1088 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(FORWARD_T,
																 STRING_ST,
																 NUMBER_ST,
																(yyvsp[-3].strval),
																(void*)(yyvsp[-1].intval));
												 }
#line 4061 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 270:
#line 1094 "cfg.y" /* yacc.c:1646  */
    {(yyval.action)=mk_action(FORWARD_T,
																 STRING_ST,
																 NUMBER_ST,
																(yyvsp[-3].strval),
																(void*)(yyvsp[-1].intval));
													}
#line 4072 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 271:
#line 1100 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(FORWARD_T,
																 IP_ST,
																 NUMBER_ST,
																 (void*)(yyvsp[-3].ipaddr),
																(void*)(yyvsp[-1].intval));
												  }
#line 4083 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 272:
#line 1106 "cfg.y" /* yacc.c:1646  */
    {
													(yyval.action)=mk_action(FORWARD_T,
																 URIHOST_ST,
																 URIPORT_ST,
																0,
																0);
													}
#line 4095 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 273:
#line 1115 "cfg.y" /* yacc.c:1646  */
    {
													(yyval.action)=mk_action(FORWARD_T,
																 URIHOST_ST,
																 NUMBER_ST,
																0,
																(void*)(yyvsp[-1].intval));
													}
#line 4107 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 274:
#line 1122 "cfg.y" /* yacc.c:1646  */
    {
													(yyval.action)=mk_action(FORWARD_T,
																 URIHOST_ST,
																 NUMBER_ST,
																0,
																0);
										}
#line 4119 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 275:
#line 1129 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); }
#line 4125 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 276:
#line 1130 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("bad forward"
										"argument"); }
#line 4132 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 277:
#line 1132 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(	FORWARD_UDP_T,
														STRING_ST,
														NUMBER_ST,
														(yyvsp[-1].strval),
														0);
										}
#line 4143 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 278:
#line 1138 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(	FORWARD_UDP_T,
														STRING_ST,
														NUMBER_ST,
														(yyvsp[-1].strval),
														0);
										}
#line 4154 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 279:
#line 1144 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(	FORWARD_UDP_T,
														IP_ST,
														NUMBER_ST,
														(void*)(yyvsp[-1].ipaddr),
														0);
										}
#line 4165 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 280:
#line 1150 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(
																FORWARD_UDP_T,
																 STRING_ST,
																 NUMBER_ST,
																(yyvsp[-3].strval),
																(void*)(yyvsp[-1].intval));
												 }
#line 4177 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 281:
#line 1157 "cfg.y" /* yacc.c:1646  */
    {(yyval.action)=mk_action(
																FORWARD_UDP_T,
																 STRING_ST,
																 NUMBER_ST,
																(yyvsp[-3].strval),
																(void*)(yyvsp[-1].intval));
													}
#line 4189 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 282:
#line 1164 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(
																FORWARD_UDP_T,
																 IP_ST,
																 NUMBER_ST,
																 (void*)(yyvsp[-3].ipaddr),
																(void*)(yyvsp[-1].intval));
												  }
#line 4201 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 283:
#line 1171 "cfg.y" /* yacc.c:1646  */
    {
													(yyval.action)=mk_action(FORWARD_UDP_T,
																 URIHOST_ST,
																 URIPORT_ST,
																0,
																0);
													}
#line 4213 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 284:
#line 1180 "cfg.y" /* yacc.c:1646  */
    {
													(yyval.action)=mk_action(FORWARD_UDP_T,
																 URIHOST_ST,
																 NUMBER_ST,
																0,
																(void*)(yyvsp[-1].intval));
													}
#line 4225 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 285:
#line 1187 "cfg.y" /* yacc.c:1646  */
    {
													(yyval.action)=mk_action(FORWARD_UDP_T,
																 URIHOST_ST,
																 NUMBER_ST,
																0,
																0);
										}
#line 4237 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 286:
#line 1194 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); }
#line 4243 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 287:
#line 1195 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("bad forward_udp"
										"argument"); }
#line 4250 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 288:
#line 1197 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(	FORWARD_TCP_T,
														STRING_ST,
														NUMBER_ST,
														(yyvsp[-1].strval),
														0);
										}
#line 4261 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 289:
#line 1203 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(	FORWARD_TCP_T,
														STRING_ST,
														NUMBER_ST,
														(yyvsp[-1].strval),
														0);
										}
#line 4272 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 290:
#line 1209 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(	FORWARD_TCP_T,
														IP_ST,
														NUMBER_ST,
														(void*)(yyvsp[-1].ipaddr),
														0);
										}
#line 4283 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 291:
#line 1215 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(
																FORWARD_TCP_T,
																 STRING_ST,
																 NUMBER_ST,
																(yyvsp[-3].strval),
																(void*)(yyvsp[-1].intval));
												 }
#line 4295 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 292:
#line 1222 "cfg.y" /* yacc.c:1646  */
    {(yyval.action)=mk_action(
																FORWARD_TCP_T,
																 STRING_ST,
																 NUMBER_ST,
																(yyvsp[-3].strval),
																(void*)(yyvsp[-1].intval));
													}
#line 4307 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 293:
#line 1229 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(FORWARD_TCP_T,
																 IP_ST,
																 NUMBER_ST,
																 (void*)(yyvsp[-3].ipaddr),
																(void*)(yyvsp[-1].intval));
												  }
#line 4318 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 294:
#line 1235 "cfg.y" /* yacc.c:1646  */
    {
													(yyval.action)=mk_action(FORWARD_TCP_T,
																 URIHOST_ST,
																 URIPORT_ST,
																0,
																0);
													}
#line 4330 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 295:
#line 1244 "cfg.y" /* yacc.c:1646  */
    {
													(yyval.action)=mk_action(FORWARD_TCP_T,
																 URIHOST_ST,
																 NUMBER_ST,
																0,
																(void*)(yyvsp[-1].intval));
													}
#line 4342 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 296:
#line 1251 "cfg.y" /* yacc.c:1646  */
    {
													(yyval.action)=mk_action(FORWARD_TCP_T,
																 URIHOST_ST,
																 NUMBER_ST,
																0,
																0);
										}
#line 4354 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 297:
#line 1258 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); }
#line 4360 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 298:
#line 1259 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("bad forward_tcp"
										"argument"); }
#line 4367 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 299:
#line 1261 "cfg.y" /* yacc.c:1646  */
    {
										#ifdef USE_TLS
											(yyval.action)=mk_action(	FORWARD_TLS_T,
														STRING_ST,
														NUMBER_ST,
														(yyvsp[-1].strval),
														0);
										#else
											(yyval.action)=0;
											yyerror("tls support not "
													"compiled in");
										#endif
										}
#line 4385 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 300:
#line 1274 "cfg.y" /* yacc.c:1646  */
    {
										#ifdef USE_TLS
											(yyval.action)=mk_action(	FORWARD_TLS_T,
															STRING_ST,
															NUMBER_ST,
															(yyvsp[-1].strval),
															0);
										#else
											(yyval.action)=0;
											yyerror("tls support not "
													"compiled in");
										#endif
										}
#line 4403 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 301:
#line 1287 "cfg.y" /* yacc.c:1646  */
    { 
										#ifdef USE_TLS
											(yyval.action)=mk_action(	FORWARD_TLS_T,
															IP_ST,
															NUMBER_ST,
															(void*)(yyvsp[-1].ipaddr),
															0);
										#else
											(yyval.action)=0;
											yyerror("tls support not "
													"compiled in");
										#endif
										}
#line 4421 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 302:
#line 1300 "cfg.y" /* yacc.c:1646  */
    { 
										#ifdef USE_TLS
											(yyval.action)=mk_action(	FORWARD_TLS_T,
															 STRING_ST,
															 NUMBER_ST,
															(yyvsp[-3].strval),
															(void*)(yyvsp[-1].intval));
										#else
											(yyval.action)=0;
											yyerror("tls support not "
													"compiled in");
										#endif
												 }
#line 4439 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 303:
#line 1313 "cfg.y" /* yacc.c:1646  */
    {
										#ifdef USE_TLS
											(yyval.action)=mk_action(	FORWARD_TLS_T,
															 STRING_ST,
															 NUMBER_ST,
															(yyvsp[-3].strval),
															(void*)(yyvsp[-1].intval));
										#else
											(yyval.action)=0;
											yyerror("tls support not "
													"compiled in");
										#endif
													}
#line 4457 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 304:
#line 1326 "cfg.y" /* yacc.c:1646  */
    {
										#ifdef USE_TLS
											(yyval.action)=mk_action(	FORWARD_TLS_T,
															 IP_ST,
															 NUMBER_ST,
															 (void*)(yyvsp[-3].ipaddr),
															(void*)(yyvsp[-1].intval));
										#else
											(yyval.action)=0;
											yyerror("tls support not "
													"compiled in");
										#endif
												  }
#line 4475 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 305:
#line 1339 "cfg.y" /* yacc.c:1646  */
    {
										#ifdef USE_TLS
											(yyval.action)=mk_action(	FORWARD_TLS_T,
															 URIHOST_ST,
															 URIPORT_ST,
															0,
															0);
										#else
											(yyval.action)=0;
											yyerror("tls support not "
													"compiled in");
										#endif
													}
#line 4493 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 306:
#line 1354 "cfg.y" /* yacc.c:1646  */
    {
										#ifdef USE_TLS
											(yyval.action)=mk_action(	FORWARD_TLS_T,
															 URIHOST_ST,
															 NUMBER_ST,
															0,
															(void*)(yyvsp[-1].intval));
										#else
											(yyval.action)=0;
											yyerror("tls support not "
													"compiled in");
										#endif
													}
#line 4511 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 307:
#line 1367 "cfg.y" /* yacc.c:1646  */
    {
										#ifdef USE_TLS
											(yyval.action)=mk_action(	FORWARD_TLS_T,
															 URIHOST_ST,
															 NUMBER_ST,
															0,
															0);
										#else
											(yyval.action)=0;
											yyerror("tls support not "
													"compiled in");
										#endif
										}
#line 4529 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 308:
#line 1380 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); }
#line 4535 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 309:
#line 1381 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("bad forward_tls"
										"argument"); }
#line 4542 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 310:
#line 1384 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(	SEND_T,
													STRING_ST,
													NUMBER_ST,
													(yyvsp[-1].strval),
													0);
									}
#line 4553 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 311:
#line 1390 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(	SEND_T,
													STRING_ST,
													NUMBER_ST,
													(yyvsp[-1].strval),
													0);
									}
#line 4564 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 312:
#line 1396 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(	SEND_T,
													IP_ST,
													NUMBER_ST,
													(void*)(yyvsp[-1].ipaddr),
													0);
									}
#line 4575 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 313:
#line 1402 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(	SEND_T,
																STRING_ST,
																NUMBER_ST,
																(yyvsp[-3].strval),
																(void*)(yyvsp[-1].intval));
												}
#line 4586 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 314:
#line 1408 "cfg.y" /* yacc.c:1646  */
    {(yyval.action)=mk_action(	SEND_T,
																STRING_ST,
																NUMBER_ST,
																(yyvsp[-3].strval),
																(void*)(yyvsp[-1].intval));
												}
#line 4597 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 315:
#line 1414 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(	SEND_T,
																IP_ST,
																NUMBER_ST,
																(void*)(yyvsp[-3].ipaddr),
																(void*)(yyvsp[-1].intval));
											   }
#line 4608 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 316:
#line 1420 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); }
#line 4614 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 317:
#line 1421 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("bad send"
													"argument"); }
#line 4621 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 318:
#line 1423 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(	SEND_TCP_T,
													STRING_ST,
													NUMBER_ST,
													(yyvsp[-1].strval),
													0);
									}
#line 4632 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 319:
#line 1429 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(	SEND_TCP_T,
													STRING_ST,
													NUMBER_ST,
													(yyvsp[-1].strval),
													0);
									}
#line 4643 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 320:
#line 1435 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(	SEND_TCP_T,
													IP_ST,
													NUMBER_ST,
													(void*)(yyvsp[-1].ipaddr),
													0);
									}
#line 4654 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 321:
#line 1441 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(	SEND_TCP_T,
																STRING_ST,
																NUMBER_ST,
																(yyvsp[-3].strval),
																(void*)(yyvsp[-1].intval));
												}
#line 4665 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 322:
#line 1447 "cfg.y" /* yacc.c:1646  */
    {(yyval.action)=mk_action(	SEND_TCP_T,
																STRING_ST,
																NUMBER_ST,
																(yyvsp[-3].strval),
																(void*)(yyvsp[-1].intval));
												}
#line 4676 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 323:
#line 1453 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(	SEND_TCP_T,
																IP_ST,
																NUMBER_ST,
																(void*)(yyvsp[-3].ipaddr),
																(void*)(yyvsp[-1].intval));
											   }
#line 4687 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 324:
#line 1459 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); }
#line 4693 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 325:
#line 1460 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("bad send_tcp"
													"argument"); }
#line 4700 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 326:
#line 1462 "cfg.y" /* yacc.c:1646  */
    {(yyval.action)=mk_action(DROP_T,0, 0, 0, 0); }
#line 4706 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 327:
#line 1463 "cfg.y" /* yacc.c:1646  */
    {(yyval.action)=mk_action(DROP_T,0, 0, 0, 0); }
#line 4712 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 328:
#line 1464 "cfg.y" /* yacc.c:1646  */
    {(yyval.action)=mk_action(	LOG_T, NUMBER_ST, 
													STRING_ST,(void*)4,(yyvsp[-1].strval));
									}
#line 4720 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 329:
#line 1467 "cfg.y" /* yacc.c:1646  */
    {(yyval.action)=mk_action(	LOG_T,
																NUMBER_ST, 
																STRING_ST,
																(void*)(yyvsp[-3].intval),
																(yyvsp[-1].strval));
												}
#line 4731 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 330:
#line 1473 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); }
#line 4737 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 331:
#line 1474 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("bad log"
									"argument"); }
#line 4744 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 332:
#line 1476 "cfg.y" /* yacc.c:1646  */
    {(yyval.action)=mk_action( SETFLAG_T, NUMBER_ST, 0,
													(void *)(yyvsp[-1].intval), 0 ); }
#line 4751 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 333:
#line 1478 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("missing '(' or ')'?"); }
#line 4757 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 334:
#line 1479 "cfg.y" /* yacc.c:1646  */
    {(yyval.action)=mk_action(	RESETFLAG_T, NUMBER_ST, 0,
													(void *)(yyvsp[-1].intval), 0 ); }
#line 4764 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 335:
#line 1481 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("missing '(' or ')'?"); }
#line 4770 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 336:
#line 1482 "cfg.y" /* yacc.c:1646  */
    {(yyval.action)=mk_action(	ISFLAGSET_T, NUMBER_ST, 0,
													(void *)(yyvsp[-1].intval), 0 ); }
#line 4777 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 337:
#line 1484 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("missing '(' or ')'?"); }
#line 4783 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 338:
#line 1485 "cfg.y" /* yacc.c:1646  */
    {(yyval.action)=mk_action(ERROR_T,
																STRING_ST, 
																STRING_ST,
																(yyvsp[-3].strval),
																(yyvsp[-1].strval));
												  }
#line 4794 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 339:
#line 1491 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); }
#line 4800 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 340:
#line 1492 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("bad error"
														"argument"); }
#line 4807 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 341:
#line 1494 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(ROUTE_T, NUMBER_ST,
														0, (void*)(yyvsp[-1].intval), 0);
										}
#line 4815 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 342:
#line 1497 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); }
#line 4821 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 343:
#line 1498 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("bad route"
						"argument"); }
#line 4828 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 344:
#line 1500 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(	EXEC_T, STRING_ST, 0,
													(yyvsp[-1].strval), 0);
									}
#line 4836 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 345:
#line 1503 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(SET_HOST_T, STRING_ST,
														0, (yyvsp[-1].strval), 0); }
#line 4843 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 346:
#line 1505 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); }
#line 4849 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 347:
#line 1506 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("bad argument, "
														"string expected"); }
#line 4856 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 348:
#line 1509 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(PREFIX_T, STRING_ST,
														0, (yyvsp[-1].strval), 0); }
#line 4863 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 349:
#line 1511 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); }
#line 4869 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 350:
#line 1512 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("bad argument, "
														"string expected"); }
#line 4876 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 351:
#line 1514 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(STRIP_TAIL_T, 
									NUMBER_ST, 0, (void *) (yyvsp[-1].intval), 0); }
#line 4883 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 352:
#line 1516 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); }
#line 4889 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 353:
#line 1517 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("bad argument, "
														"number expected"); }
#line 4896 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 354:
#line 1520 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(STRIP_T, NUMBER_ST,
														0, (void *) (yyvsp[-1].intval), 0); }
#line 4903 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 355:
#line 1522 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); }
#line 4909 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 356:
#line 1523 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("bad argument, "
														"number expected"); }
#line 4916 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 357:
#line 1525 "cfg.y" /* yacc.c:1646  */
    { 
		    {   qvalue_t q;
			if (str2q(&q, (yyvsp[-1].strval), strlen((yyvsp[-1].strval))) < 0) {
				yyerror("bad argument, q value expected");
			}
			(yyval.action)=mk_action(APPEND_BRANCH_T, STRING_ST, NUMBER_ST, (yyvsp[-3].strval), 
							(void *)(long)q); } 
		}
#line 4929 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 358:
#line 1534 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action( APPEND_BRANCH_T,
													STRING_ST, NUMBER_ST, (yyvsp[-1].strval), (void *)Q_UNSPECIFIED) ; }
#line 4936 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 359:
#line 1536 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action( APPEND_BRANCH_T,
													STRING_ST, NUMBER_ST, 0, (void *)Q_UNSPECIFIED ) ; }
#line 4943 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 360:
#line 1538 "cfg.y" /* yacc.c:1646  */
    {  (yyval.action)=mk_action( APPEND_BRANCH_T, STRING_ST, 0, 0, 0 ) ; }
#line 4949 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 361:
#line 1540 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action( SET_HOSTPORT_T, 
														STRING_ST, 0, (yyvsp[-1].strval), 0); }
#line 4956 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 362:
#line 1542 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); }
#line 4962 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 363:
#line 1543 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("bad argument,"
												" string expected"); }
#line 4969 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 364:
#line 1545 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action( SET_PORT_T, STRING_ST,
														0, (yyvsp[-1].strval), 0); }
#line 4976 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 365:
#line 1547 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); }
#line 4982 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 366:
#line 1548 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("bad argument, "
														"string expected"); }
#line 4989 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 367:
#line 1550 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action( SET_USER_T, STRING_ST,
														0, (yyvsp[-1].strval), 0); }
#line 4996 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 368:
#line 1552 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); }
#line 5002 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 369:
#line 1553 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("bad argument, "
														"string expected"); }
#line 5009 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 370:
#line 1555 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action( SET_USERPASS_T, 
														STRING_ST, 0, (yyvsp[-1].strval), 0); }
#line 5016 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 371:
#line 1557 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); }
#line 5022 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 372:
#line 1558 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("bad argument, "
														"string expected"); }
#line 5029 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 373:
#line 1560 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action( SET_URI_T, STRING_ST, 
														0, (yyvsp[-1].strval), 0); }
#line 5036 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 374:
#line 1562 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); }
#line 5042 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 375:
#line 1563 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("bad argument, "
										"string expected"); }
#line 5049 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 376:
#line 1565 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action( REVERT_URI_T, 0,0,0,0); }
#line 5055 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 377:
#line 1566 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action( REVERT_URI_T, 0,0,0,0); }
#line 5061 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 378:
#line 1567 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=mk_action(FORCE_RPORT_T,0, 0, 0, 0); }
#line 5067 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 379:
#line 1568 "cfg.y" /* yacc.c:1646  */
    {(yyval.action)=mk_action(FORCE_RPORT_T,0, 0, 0, 0); }
#line 5073 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 380:
#line 1569 "cfg.y" /* yacc.c:1646  */
    {
					#ifdef USE_TCP
						(yyval.action)=mk_action(FORCE_TCP_ALIAS_T,NUMBER_ST, 0,
										(void*)(yyvsp[-1].intval), 0);
					#else
						yyerror("tcp support not compiled in");
					#endif
												}
#line 5086 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 381:
#line 1577 "cfg.y" /* yacc.c:1646  */
    {
					#ifdef USE_TCP
						(yyval.action)=mk_action(FORCE_TCP_ALIAS_T,0, 0, 0, 0); 
					#else
						yyerror("tcp support not compiled in");
					#endif
										}
#line 5098 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 382:
#line 1584 "cfg.y" /* yacc.c:1646  */
    {
					#ifdef USE_TCP
						(yyval.action)=mk_action(FORCE_TCP_ALIAS_T,0, 0, 0, 0);
					#else
						yyerror("tcp support not compiled in");
					#endif
										}
#line 5110 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 383:
#line 1591 "cfg.y" /* yacc.c:1646  */
    {(yyval.action)=0; 
					yyerror("bad argument, number expected");
					}
#line 5118 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 384:
#line 1594 "cfg.y" /* yacc.c:1646  */
    {
								(yyval.action)=0;
								if ((str_tmp=pkg_malloc(sizeof(str)))==0){
										LOG(L_CRIT, "ERROR: cfg. parser:"
													" out of memory.\n");
								}else{
										str_tmp->s=(yyvsp[-1].strval);
										str_tmp->len=strlen((yyvsp[-1].strval));
										(yyval.action)=mk_action(SET_ADV_ADDR_T, STR_ST,
										             0, str_tmp, 0);
								}
												  }
#line 5135 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 385:
#line 1606 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("bad argument, "
														"string expected"); }
#line 5142 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 386:
#line 1608 "cfg.y" /* yacc.c:1646  */
    {(yyval.action)=0; yyerror("missing '(' or ')' ?"); }
#line 5148 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 387:
#line 1609 "cfg.y" /* yacc.c:1646  */
    {
								(yyval.action)=0;
								tmp=int2str((yyvsp[-1].intval), &i_tmp);
								if ((str_tmp=pkg_malloc(sizeof(str)))==0){
										LOG(L_CRIT, "ERROR: cfg. parser:"
													" out of memory.\n");
								}else{
									if ((str_tmp->s=pkg_malloc(i_tmp))==0){
										LOG(L_CRIT, "ERROR: cfg. parser:"
													" out of memory.\n");
									}else{
										memcpy(str_tmp->s, tmp, i_tmp);
										str_tmp->len=i_tmp;
										(yyval.action)=mk_action(SET_ADV_PORT_T, STR_ST,
													0, str_tmp, 0);
									}
								}
								            }
#line 5171 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 388:
#line 1627 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("bad argument, "
														"string expected"); }
#line 5178 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 389:
#line 1629 "cfg.y" /* yacc.c:1646  */
    {(yyval.action)=0; yyerror("missing '(' or ')' ?"); }
#line 5184 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 390:
#line 1630 "cfg.y" /* yacc.c:1646  */
    {
										(yyval.action)=mk_action(FORCE_SEND_SOCKET_T,
														SOCKID_ST, 0, (yyvsp[-1].sockid), 0);
													}
#line 5193 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 391:
#line 1634 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("bad argument,"
											" [proto:]host[:port] expected"); }
#line 5200 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 392:
#line 1636 "cfg.y" /* yacc.c:1646  */
    {(yyval.action)=0; yyerror("missing '(' or ')' ?"); }
#line 5206 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 393:
#line 1637 "cfg.y" /* yacc.c:1646  */
    { f_tmp=(void*)find_export((yyvsp[-2].strval), 0, rt);
									   if (f_tmp==0){
										   if (find_export((yyvsp[-2].strval), 0, 0)) {
											   yyerror("Command cannot be used in the block\n");
										   } else {
											   yyerror("unknown command, missing"
												   " loadmodule?\n");
										   }
										(yyval.action)=0;
									   }else{
										(yyval.action)=mk_action(	MODULE_T,
														CMDF_ST,
														0,
														f_tmp,
														0
													);
									   }
									}
#line 5229 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 394:
#line 1655 "cfg.y" /* yacc.c:1646  */
    { f_tmp=(void*)find_export((yyvsp[-3].strval), 1, rt);
									if (f_tmp==0){
										if (find_export((yyvsp[-3].strval), 1, 0)) {
											yyerror("Command cannot be used in the block\n");
										} else {
											yyerror("unknown command, missing"
												" loadmodule?\n");
										}
										(yyval.action)=0;
									}else{
										(yyval.action)=mk_action(	MODULE_T,
														CMDF_ST,
														STRING_ST,
														f_tmp,
														(yyvsp[-1].strval)
													);
									}
								  }
#line 5252 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 395:
#line 1674 "cfg.y" /* yacc.c:1646  */
    { f_tmp=(void*)find_export((yyvsp[-5].strval), 2, rt);
									if (f_tmp==0){
										if (find_export((yyvsp[-5].strval), 2, 0)) {
											yyerror("Command cannot be used in the block\n");
										} else {
											yyerror("unknown command, missing"
												" loadmodule?\n");
										}
										(yyval.action)=0;
									}else{
										(yyval.action)=mk_action3(	MODULE_T,
														CMDF_ST,
														STRING_ST,
														STRING_ST,
														f_tmp,
														(yyvsp[-3].strval),
														(yyvsp[-1].strval)
													);
									}
								  }
#line 5277 "cfg.tab.c" /* yacc.c:1646  */
    break;

  case 396:
#line 1694 "cfg.y" /* yacc.c:1646  */
    { (yyval.action)=0; yyerror("bad arguments"); }
#line 5283 "cfg.tab.c" /* yacc.c:1646  */
    break;


#line 5287 "cfg.tab.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 1698 "cfg.y" /* yacc.c:1906  */


extern int line;
extern int column;
extern int startcolumn;
static void warn(char* s)
{
	LOG(L_WARN, "cfg. warning: (%d,%d-%d): %s\n", line, startcolumn, 
			column, s);
	cfg_errors++;
}

static void yyerror(char* s)
{
	LOG(L_CRIT, "parse error (%d,%d-%d): %s\n", line, startcolumn, 
			column, s);
	cfg_errors++;
}


static struct socket_id* mk_listen_id(char* host, int proto, int port)
{
	struct socket_id* l;
	l=pkg_malloc(sizeof(struct socket_id));
	if (l==0){
		LOG(L_CRIT,"ERROR: cfg. parser: out of memory.\n");
	}else{
		l->name=host;
		l->port=port;
		l->proto=proto;
		l->next=0;
	}
	return l;
}


/*
int main(int argc, char ** argv)
{
	if (yyparse()!=0)
		fprintf(stderr, "parsing error\n");
}
*/
