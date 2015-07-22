#ifdef _MSC_VER
#pragma warning(push, 1)
#pragma warning(disable : 4003)
#pragma warning(disable : 4702)
#endif
/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

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
#define YYBISON_VERSION "3.0.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         reteparse
#define yylex           retelex
#define yyerror         reteerror
#define yydebug         retedebug
#define yynerrs         retenerrs


/* Copy the first part of user declarations.  */
#line 34 "rules.yyy" /* yacc.c:339  */

#define YY_NO_UNISTD_H 1
#include <csignal>
#include <cstdio>
#include <sstream>
#include "rete_parser.h"

using namespace std;

namespace Rete {

  static unordered_map<string, shared_ptr<tuple<double, double, bool>>> * Flags(const string &flag, const shared_ptr<tuple<double, double, bool>> &value, std::unordered_map<std::string, std::shared_ptr<std::tuple<double, double, bool>>> * flags = nullptr) {
    if(!flags)
      flags = new std::unordered_map<std::string, std::shared_ptr<std::tuple<double, double, bool>>>;
    (*flags)[flag] = value;
    return flags;
  }

  static vector<Parser_Variable> Variables() {
    vector<Parser_Variable> rv;
    return rv;
  }
  static vector<Parser_Variable> Variables(const Parser_Variable &variable) {
    vector<Parser_Variable> rv;
    rv.push_back(variable);
    return rv;
  }
  static pair<Rete::Rete_Node_Ptr, vector<Parser_Variable>> *
  Rete_Node_Ptr_and_Variables(const Rete::Rete_Node_Ptr &rete_node, const vector<Parser_Variable> &variables) {
    return new pair<Rete::Rete_Node_Ptr, vector<Parser_Variable>>(rete_node, variables);
  }

  static Rete::WME_Token_Index find_index(const std::vector<Parser_Variable> &variables, const std::string &name) {
    for(const auto &variable : variables) {
      for(const auto &ni : variable.name_index) {
        if(!ni.second.existential && ni.first == name)
          return ni.second;
      }
    }
    return Rete::WME_Token_Index(-1, -1, -1);
  }

  static Rete::WME_Bindings join_bindings(const std::vector<Parser_Variable> &lhs,
                                          const std::vector<Parser_Variable> &rhs)
  {
    Rete::WME_Bindings bindings;
    unordered_set<string> joined;
    for(const Parser_Variable &lvar : lhs) {
      for(const auto &lni : lvar.name_index) {
        if(!lni.second.existential && !lni.first.empty() && joined.find(lni.first) == joined.end()) {
          for(const Parser_Variable &rvar : rhs) {
            for(const auto &rni : rvar.name_index) {
              if(!rni.second.existential && lni.first == rni.first) {
                bindings.insert(Rete::WME_Binding(lni.second, rni.second));
                joined.insert(lni.first);
                goto DONE_JOINING;
              }
            }
          }
        }
        DONE_JOINING:
          ;
      }
    }
    return bindings;
  }

  static Rete::Variable_Indices_Ptr_C variables_to_indices(const vector<Parser_Variable> &variables) {
    const auto indices = std::make_shared<Rete::Variable_Indices>();
    for(const auto &variable : variables) {
      for(const auto &ni : variable.name_index) {
        if(ni.first.empty())
          continue;
        indices->insert(ni);
      }
    }
    return indices;
  }

  static vector<Parser_Variable> merge_variables(const vector<Parser_Variable> &lhs, const int64_t &lhs_size, const int64_t &lhs_token_size, const vector<Parser_Variable> &rhs, const bool &existential) {
    vector<Parser_Variable> variables(lhs);
    variables.reserve(variables.size() + rhs.size());
    for(const Parser_Variable &var_ : rhs) {
      Parser_Variable var(var_);
      for(auto &ni : var.name_index) {
        for(const Parser_Variable &existing_var : variables) {
          for(const auto &existing_ni : existing_var.name_index) {
            if(!existing_ni.second.existential && existing_ni.first == ni.first) {
              ni.first.clear();
              goto VAR_FOUND;
            }
          }
        }
        VAR_FOUND:
        ni.second.rete_row += lhs_size;
        ni.second.token_row += lhs_token_size;
        ni.second.existential = ni.second.existential | existential;
          ;
      }
      variables.push_back(var);
    }
    return variables;
  }

}


#line 180 "rules.tab.cpp" /* yacc.c:339  */

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
# define YYERROR_VERBOSE 1
#endif

/* In a future release of Bison, this section will be replaced
   by #include "rules.tab.hh".  */
#ifndef YY_RETE_RULES_TAB_HH_INCLUDED
# define YY_RETE_RULES_TAB_HH_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int retedebug;
#endif
/* "%code requires" blocks.  */
#line 14 "rules.yyy" /* yacc.c:355  */

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void *yyscan_t;
#endif

#line 217 "rules.tab.cpp" /* yacc.c:355  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    ARROW_RIGHT = 258,
    ASSIGNMENT = 259,
    COMMAND_EXCISE = 260,
    COMMAND_EXCISE_ALL = 261,
    COMMAND_EXIT = 262,
    COMMAND_INSERT_WME = 263,
    COMMAND_REMOVE_WME = 264,
    COMMAND_SET_TOTAL_STEP_COUNT = 265,
    COMMAND_SOURCE = 266,
    COMMAND_SP = 267,
    EXISTENTIAL_MATCH = 268,
    FLAG_CREATION_TIME = 269,
    FLAG_FEATURE = 270,
    FLOAT = 271,
    INT = 272,
    NODE_TYPE = 273,
    STRING = 274,
    STRING_PART_C = 275,
    STRING_PART_S = 276,
    VARIABLE = 277,
    PREDICATE = 278
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 142 "rules.yyy" /* yacc.c:355  */

  char cval;
  const char *csval;
  Rete::Parser_Flag *flag_ptr;
  double fval;
  int64_t ival;
  std::list<std::string> *slist;
  std::string *sval;
  Rete::Parser_Rete_Node *rete_node_ptr;
  Rete::Parser_Rule *rule_ptr;
  Rete::Symbol_Ptr_C *symbol_ptr;
  Rete::Rete_Predicate::Predicate predicate;

#line 267 "rules.tab.cpp" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



int reteparse (yyscan_t yyscanner, Carli::Agent &agent, const std::string &filename, const std::string &source_path);

#endif /* !YY_RETE_RULES_TAB_HH_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 295 "rules.tab.cpp" /* yacc.c:358  */
/* Unqualified %code blocks.  */
#line 21 "rules.yyy" /* yacc.c:359  */

#include "lex.rete.hh"

static volatile sig_atomic_t g_rete_exit = false;

static void reteerror(const YYLTYPE * const /*yylloc*/, yyscan_t const yyscanner, Rete::Rete_Agent &/*agent*/, const std::string &filename, const std::string &/*source_path*/, const char *msg) {
  if(filename.empty())
    cout << "rete-parse error: " << msg << endl;
  else
    cout << "rete-parse error " << filename << '(' << reteget_lineno(yyscanner) << "): " << msg << endl;
}

#line 310 "rules.tab.cpp" /* yacc.c:359  */

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
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

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
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   125

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  33
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  17
/* YYNRULES -- Number of rules.  */
#define YYNRULES  51
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  112

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   278

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
      24,    26,     2,    29,     2,    30,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    31,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    25,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    27,    32,    28,     2,     2,     2,     2,
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
      15,    16,    17,    18,    19,    20,    21,    22,    23
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   186,   186,   187,   190,   192,   193,   195,   200,   205,
     206,   218,   404,   412,   421,   433,   434,   440,   448,   456,
     469,   470,   471,   472,   477,   481,   485,   489,   498,   508,
     511,   514,   515,   518,   519,   520,   521,   524,   525,   528,
     531,   532,   533,   536,   537,   540,   541,   544,   547,   548,
     549,   550
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "ARROW_RIGHT", "ASSIGNMENT",
  "COMMAND_EXCISE", "COMMAND_EXCISE_ALL", "COMMAND_EXIT",
  "COMMAND_INSERT_WME", "COMMAND_REMOVE_WME",
  "COMMAND_SET_TOTAL_STEP_COUNT", "COMMAND_SOURCE", "COMMAND_SP",
  "EXISTENTIAL_MATCH", "FLAG_CREATION_TIME", "FLAG_FEATURE", "FLOAT",
  "INT", "NODE_TYPE", "STRING", "STRING_PART_C", "STRING_PART_S",
  "VARIABLE", "PREDICATE", "'('", "'^'", "')'", "'{'", "'}'", "'+'", "'-'",
  "'@'", "'|'", "$accept", "commands", "command", "rule", "flags",
  "final_conditions", "conditions", "condition_group", "condition_type",
  "condition", "symbol", "identifier", "symbol_constant", "number",
  "string_or_literal", "literal", "literal_parts", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,    40,    94,    41,   123,   125,    43,
      45,    64,   124
};
# endif

#define YYPACT_NINF -78

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-78)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int8 yypact[] =
{
     -78,    82,   -78,   -15,   -78,   -78,   -18,   -12,    -8,    -9,
       7,   -78,   -78,    19,    19,   -78,   -78,    50,   -78,   -78,
      -1,   -78,   -78,   -78,     6,    29,   -78,   -78,   -78,    32,
     -78,   -78,    63,   -78,   -78,    19,    19,   -78,   -78,   -78,
      18,    19,    19,    27,    44,    45,    74,   -19,   -19,    -2,
      83,   -78,   -78,   -78,    23,    76,   -78,    68,    53,   -11,
      83,    83,   101,   -19,   -78,    84,   -19,   -19,   -78,   -78,
     -78,    90,    24,   -78,    57,   -78,    14,   -78,   -78,    94,
      36,    43,   -78,   -78,    80,    47,    60,    88,    89,    91,
      92,   -78,    57,    93,    95,   100,   103,   -78,   -78,   -78,
     -78,    57,   -78,   -78,   -78,   -78,    57,    57,    57,   105,
      96,   -78
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     1,     0,     5,     6,     0,     0,     0,     0,
       0,     3,     4,     0,     0,     9,    45,     0,    10,    46,
       0,    11,    40,    41,     0,     0,    38,    37,    42,     0,
      50,    51,     0,    15,    39,     0,     0,    48,    49,    47,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      20,    32,    29,    31,     0,     0,    16,     0,     0,     0,
      21,    22,     0,     0,    12,     0,     0,     0,    24,     7,
       8,     0,     0,    30,     0,    23,     0,    25,    26,    19,
       0,     0,    44,    43,     0,     0,     0,     0,     0,     0,
       0,    14,     0,     0,     0,     0,     0,    36,    35,    34,
      33,     0,    28,    27,    17,    18,     0,     0,     0,     0,
       0,    13
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -78,   -78,   -78,   -78,   -78,    77,    52,   -78,   -47,   -78,
     -14,   -78,     0,   -77,   116,   -78,   -78
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     1,    11,    21,    40,    49,    50,    51,    52,    53,
      25,    26,    27,    84,    28,    19,    32
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      29,    62,    63,    68,    12,    45,    13,    92,    46,    15,
      16,    63,    14,    68,    68,   101,    75,    73,    33,    77,
      78,    41,    42,    17,   106,    34,    64,    54,    55,   107,
     108,   109,    43,    44,    20,    22,    23,    85,    16,    72,
      22,    23,    45,    16,    56,    46,    80,    47,    48,    69,
      24,    17,    22,    23,    35,    16,    17,    36,    87,    22,
      23,    57,    16,    22,    23,    89,    16,    58,    17,    93,
      30,    31,    81,    82,    83,    17,    95,    96,    72,    17,
      88,    90,     2,    37,    38,    94,    71,     3,     4,     5,
       6,     7,     8,     9,    10,    39,    82,    83,    45,    60,
      61,    46,    70,    47,    48,    74,    76,    65,    91,    79,
      46,    86,    66,    67,    97,    98,   104,    99,   100,   102,
     105,   103,   110,    59,   111,    18
};

static const yytype_uint8 yycheck[] =
{
      14,     3,    13,    50,    19,    24,    24,    84,    27,    17,
      19,    13,    24,    60,    61,    92,    63,    28,    19,    66,
      67,    35,    36,    32,   101,    19,    28,    41,    42,   106,
     107,   108,    14,    15,    27,    16,    17,    23,    19,    25,
      16,    17,    24,    19,    17,    27,    22,    29,    30,    26,
      31,    32,    16,    17,    25,    19,    32,    25,    22,    16,
      17,    17,    19,    16,    17,    22,    19,    22,    32,    22,
      20,    21,    72,    16,    17,    32,    16,    17,    25,    32,
      80,    81,     0,    20,    21,    85,    18,     5,     6,     7,
       8,     9,    10,    11,    12,    32,    16,    17,    24,    47,
      48,    27,    26,    29,    30,     4,    22,    24,    28,    19,
      27,    17,    29,    30,    26,    26,    16,    26,    26,    26,
      17,    26,    17,    46,    28,     9
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    34,     0,     5,     6,     7,     8,     9,    10,    11,
      12,    35,    19,    24,    24,    17,    19,    32,    47,    48,
      27,    36,    16,    17,    31,    43,    44,    45,    47,    43,
      20,    21,    49,    19,    19,    25,    25,    20,    21,    32,
      37,    43,    43,    14,    15,    24,    27,    29,    30,    38,
      39,    40,    41,    42,    43,    43,    17,    17,    22,    38,
      39,    39,     3,    13,    28,    24,    29,    30,    41,    26,
      26,    18,    25,    28,     4,    41,    22,    41,    41,    19,
      22,    45,    16,    17,    46,    23,    17,    22,    45,    22,
      45,    28,    46,    22,    45,    16,    17,    26,    26,    26,
      26,    46,    26,    26,    16,    17,    46,    46,    46,    46,
      17,    28
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    33,    34,    34,    35,    35,    35,    35,    35,    35,
      35,    35,    36,    36,    36,    37,    37,    37,    37,    37,
      38,    38,    38,    38,    39,    39,    39,    39,    39,    39,
      40,    41,    41,    42,    42,    42,    42,    43,    43,    44,
      45,    45,    45,    46,    46,    47,    47,    48,    49,    49,
      49,    49
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     1,     1,     7,     7,     2,
       2,     2,     5,    15,     8,     0,     3,     8,     8,     5,
       1,     2,     2,     3,     2,     3,     3,     6,     6,     1,
       3,     1,     1,     6,     6,     6,     6,     1,     1,     2,
       1,     1,     1,     1,     1,     1,     1,     3,     2,     2,
       1,     1
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
      yyerror (&yylloc, yyscanner, agent, filename, source_path, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


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


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static unsigned
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  unsigned res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
 }

#  define YY_LOCATION_PRINT(File, Loc)          \
  yy_location_print_ (File, &(Loc))

# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, Location, yyscanner, agent, filename, source_path); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, yyscan_t yyscanner, Carli::Agent &agent, const std::string &filename, const std::string &source_path)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (yylocationp);
  YYUSE (yyscanner);
  YYUSE (agent);
  YYUSE (filename);
  YYUSE (source_path);
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, yyscan_t yyscanner, Carli::Agent &agent, const std::string &filename, const std::string &source_path)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, yyscanner, agent, filename, source_path);
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, yyscan_t yyscanner, Carli::Agent &agent, const std::string &filename, const std::string &source_path)
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
                       , &(yylsp[(yyi + 1) - (yynrhs)])                       , yyscanner, agent, filename, source_path);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, yyscanner, agent, filename, source_path); \
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, yyscan_t yyscanner, Carli::Agent &agent, const std::string &filename, const std::string &source_path)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (yyscanner);
  YYUSE (agent);
  YYUSE (filename);
  YYUSE (source_path);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (yyscan_t yyscanner, Carli::Agent &agent, const std::string &filename, const std::string &source_path)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.
       'yyls': related to locations.

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

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[3];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yylsp = yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  yylsp[0] = yylloc;
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
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yyls1, yysize * sizeof (*yylsp),
                    &yystacksize);

        yyls = yyls1;
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
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

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
      yychar = yylex (&yylval, &yylloc, yyscanner);
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
  *++yylsp = yylloc;
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

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 3:
#line 187 "rules.yyy" /* yacc.c:1646  */
    { /*cerr << "Read in rule on line " << reteget_lineno(yyscanner) << endl;*/ }
#line 1572 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 190 "rules.yyy" /* yacc.c:1646  */
    { agent.excise_rule(*(yyvsp[0].sval), true);
                          delete (yyvsp[0].sval); }
#line 1579 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 192 "rules.yyy" /* yacc.c:1646  */
    { agent.excise_all(); }
#line 1585 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 193 "rules.yyy" /* yacc.c:1646  */
    { g_rete_exit = true;
                   YYACCEPT; }
#line 1592 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 195 "rules.yyy" /* yacc.c:1646  */
    { const auto wme = make_shared<Rete::WME>(*(yyvsp[-4].symbol_ptr), *(yyvsp[-2].symbol_ptr), *(yyvsp[-1].symbol_ptr));
                                                          agent.insert_wme(wme);
                                                          delete (yyvsp[-4].symbol_ptr);
                                                          delete (yyvsp[-2].symbol_ptr);
                                                          delete (yyvsp[-1].symbol_ptr); }
#line 1602 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 200 "rules.yyy" /* yacc.c:1646  */
    { const auto wme = make_shared<Rete::WME>(*(yyvsp[-4].symbol_ptr), *(yyvsp[-2].symbol_ptr), *(yyvsp[-1].symbol_ptr));
                                                          agent.remove_wme(wme);
                                                          delete (yyvsp[-4].symbol_ptr);
                                                          delete (yyvsp[-2].symbol_ptr);
                                                          delete (yyvsp[-1].symbol_ptr); }
#line 1612 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 205 "rules.yyy" /* yacc.c:1646  */
    { agent.set_total_step_count((yyvsp[0].ival)); }
#line 1618 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 206 "rules.yyy" /* yacc.c:1646  */
    { const int rv = rete_parse_file(agent, *(yyvsp[0].sval), source_path);
                                       if(rv) {
                                         ostringstream oss;
                                         oss << "Error sourcing '" << *(yyvsp[0].sval) << "'.";
                                         reteerror(&yylloc, yyscanner, agent, filename, source_path, oss.str().c_str());
                                         delete (yyvsp[0].sval);
                                         YYABORT;
                                       }
                                       delete (yyvsp[0].sval);
                                       if(g_rete_exit) {
                                         YYACCEPT;
                                       } }
#line 1635 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 218 "rules.yyy" /* yacc.c:1646  */
    { const auto &parent =                            get<0>(*(yyvsp[0].rule_ptr)).first;
                      const auto &variable_names =                    get<0>(*(yyvsp[0].rule_ptr)).second;
                      const string &name =                            get<1>(*(yyvsp[0].rule_ptr));
                      const int64_t &timestamp =              *get<0>(get<2>(*(yyvsp[0].rule_ptr)));
                      const int64_t &q_value_depth =   get<0>(*get<1>(get<2>(*(yyvsp[0].rule_ptr))));
                      const string &node_type =        get<1>(*get<1>(get<2>(*(yyvsp[0].rule_ptr))));
                      const string &parent_node_name = get<2>(*get<1>(get<2>(*(yyvsp[0].rule_ptr))));
                      const auto &feature =            get<3>(*get<1>(get<2>(*(yyvsp[0].rule_ptr))));
                      const auto &q_value_value =                     get<3>(*(yyvsp[0].rule_ptr));
                      const auto variable_indices = variables_to_indices(variable_names);
                      const auto parent_action = agent.get_rule(parent_node_name);

                      const auto existing = agent.unname_rule(name, true);
                      if(existing) {
                        parent->suppress_destruction(true);
                        existing->destroy(agent);
                      }

                      Rete::Rete_Action_Ptr new_action;
                      if(node_type.empty()) {
                        /// Non-RL node
                        auto node = agent.make_action_retraction(name, true,
                                                                 [](const Rete::Rete_Action &action, const Rete::WME_Token &wme_vector) {
                                                                   cout << wme_vector << "->" << action.get_name() << endl;
                                                                 }, [](const Rete::Rete_Action &action, const Rete::WME_Token &wme_vector) {
                                                                   cout << wme_vector << "<-" << action.get_name() << endl;
                                                                 }, parent, make_shared<Rete::Variable_Indices>());
                      }
                      else {
                        if(feature) {
                          auto test_node_rightmost = parent;
                          bool test_node_rightmost_existential = false;
                          while(test_node_rightmost->get_bindings()) {
                            test_node_rightmost_existential |= dynamic_cast<Rete::Rete_Existential_Join *>(test_node_rightmost.get()) || dynamic_cast<Rete::Rete_Negation_Join *>(test_node_rightmost.get());
                            test_node_rightmost = test_node_rightmost->parent_right();
                          }

                          /// Fill in Feature values / predicate tests
                          if(const auto feature_e = dynamic_cast<Carli::Feature_Enumerated<Carli::Feature> *>(feature)) {
                            /// TODO: Handle variable-to-variable relations
                            if(const auto predicate = dynamic_cast<Rete::Rete_Predicate *>(test_node_rightmost.get())) {
                              const auto rhs = predicate->get_rhs().get();
                              const auto symbol_i = dynamic_cast<const Rete::Symbol_Constant_Int *>(rhs);
                              assert(symbol_i);
                              feature_e->value = symbol_i->value;
                            }
                            else if(dynamic_cast<Rete::Rete_Existential_Join *>(parent.get()))
                              feature_e->value = true;
                            else if(dynamic_cast<Rete::Rete_Negation_Join *>(parent.get()))
                              feature_e->value = false;
                            else {
                              reteerror(&yylloc, yyscanner, agent, filename, source_path, "Error reading enumerated feature value / predicate test.");
                              YYABORT;
                            }
                          }
                          else if(const auto feature_r = dynamic_cast<Carli::Feature_Ranged<Carli::Feature> *>(feature)) {
                            const auto &predicate = dynamic_cast<Rete::Rete_Predicate &>(*test_node_rightmost);
                            feature_r->upper = predicate.get_predicate() == Rete::Rete_Predicate::GTE;
                          }
                          else {
                            reteerror(&yylloc, yyscanner, agent, filename, source_path, "Error reading enumerated/ranged feature value / predicate test.");
                            YYABORT;
                          }

                          /// Determine Feature axes
                          if(const auto predicate = dynamic_cast<const Rete::Rete_Predicate *>(test_node_rightmost.get())) {
                            feature->axis = predicate->get_lhs_index();
                            feature->axis.existential = test_node_rightmost_existential;
                          }
                          else if(dynamic_cast<Rete::Rete_Existential_Join *>(parent.get()) ||
                                  dynamic_cast<Rete::Rete_Negation_Join *>(parent.get()))
                          {
                            feature->axis = Rete::WME_Token_Index(-1, -1, -1);
                          }
                          else {
                            Rete::Rete_Node_Ptr test_node = parent;
                            const Rete::Rete_Filter * filter_node = dynamic_cast<const Rete::Rete_Filter *>(test_node->parent_right().get());
                            while(!filter_node) {
                              test_node = test_node->parent_right();
                              filter_node = dynamic_cast<const Rete::Rete_Filter *>(test_node->parent_right().get());
                            }

                            if(const auto bindings = test_node->get_bindings()) {
                              for(const auto binding : *bindings) {
                                if(binding.second == Rete::WME_Token_Index(0, 0, 0)) {
                                  feature->axis = binding.first;
                                  goto AXIS_FOUND;
                                }
                              }
                            }
                            reteerror(&yylloc, yyscanner, agent, filename, source_path, "Error determining Feature axis.");
                            YYABORT;
                            AXIS_FOUND:
                              ;
                          }

                          if(feature->get_depth() > 1) {
                            const auto predicate = dynamic_cast<const Rete::Rete_Predicate *>(parent.get());
                            assert(predicate);
                            if(predicate) {
                              auto ancestor = parent_action;
                              auto ancestor_prev = ancestor;
                              assert(ancestor && dynamic_cast<const Carli::Node *>(ancestor->data.get())->q_value->feature);
                              while(ancestor && dynamic_cast<const Carli::Node *>(ancestor->data.get())->q_value->feature &&
                                    ancestor->get_token_size() > predicate->get_lhs_index().token_row)
                              {
                                ancestor_prev = ancestor;
                                ancestor = dynamic_cast<const Carli::Node *>(ancestor->data.get())->parent_action.lock();
                              }

                              assert(ancestor);
                              if(ancestor) {
                                const auto ancestor_node = dynamic_cast<const Carli::Node *>(ancestor_prev->data.get());
                                assert(ancestor_node);
                                assert(ancestor_node->q_value);
                                assert(ancestor_node->q_value->feature);
                                feature->bindings = ancestor_node->q_value->feature->bindings;
                                feature->conditions = ancestor_node->q_value->feature->conditions;
                              }
                              else
                                feature->bindings.insert(std::make_pair(predicate->get_lhs_index(), Rete::WME_Token_Index(-1, -1, -1)));
                            }
                          }
                          else {
                            if(parent->get_bindings()) {
                              feature->bindings = *parent->get_bindings();
                              feature->conditions = parent->parent_right()->get_filter_wmes();
                            }
                            if(auto predicate = dynamic_cast<const Rete::Rete_Predicate *>(parent.get()))
                              feature->bindings.insert(std::make_pair(predicate->get_lhs_index(), Rete::WME_Token_Index(-1, -1, -1)));
                          }

                          if(feature->axis.rete_row != -1 && parent->get_bindings()) {
                            if(feature->axis.existential && dynamic_cast<const Carli::Feature_Ranged_Data *>(feature)) {
                              reteerror(&yylloc, yyscanner, agent, filename, source_path, "Refineable feature illegally declared existential.");
                              YYABORT;
                            }
                            feature->axis.rete_row += parent->parent_left()->get_size();
                            feature->axis.token_row += parent->parent_left()->get_token_size();
                          }
                          feature->indices = variable_indices;

                          //std::cerr << "LHS" << feature->axis << ' ' << *variable_indices << std::endl;
                        }

                        /// Make the new action
                        if(node_type == "split") {
                          const auto new_q_value = new Carli::Q_Value(q_value_value, Carli::Q_Value::Type::SPLIT, q_value_depth, feature, timestamp);
                          new_action = agent.make_standard_action(parent, name, true, variable_indices);
                          auto new_action_data = std::make_shared<Carli::Node_Split>(agent, parent_action, new_action, new_q_value);
                          new_action->data = new_action_data;
                        }
                        else if(node_type == "unsplit") {
                          const auto new_q_value = new Carli::Q_Value(q_value_value, Carli::Q_Value::Type::UNSPLIT, q_value_depth, feature, timestamp);
                          new_action = agent.make_standard_action(parent, name, true, variable_indices);
                          auto new_action_data = std::make_shared<Carli::Node_Unsplit>(agent, parent_action, new_action, new_q_value);
                          new_action->data = new_action_data;
                        }
                        else {
                          assert(node_type == "fringe");
                          const auto new_q_value = new Carli::Q_Value(q_value_value, Carli::Q_Value::Type::FRINGE, q_value_depth, feature, timestamp);
                          new_action = agent.make_standard_action(parent, name, true, variable_indices);
                          auto new_action_data = std::make_shared<Carli::Node_Fringe>(agent, parent_action, new_action, new_q_value);
                          new_action->data = new_action_data;
                          if(parent_action) {
                            auto &parent_data = dynamic_cast<Carli::Node_Unsplit &>(*parent_action->data);
                            parent_data.fringe_values[feature].values.push_back(new_action_data);
                          }
                          else {
                            reteerror(&yylloc, yyscanner, agent, filename, source_path, "Ancestral relationship specified in :feature fringe not found.");
                            YYABORT;
                          }
                        }

                        if(parent_action && parent->parent_left() != parent_action->parent_left()) {
                          if(!get_Option_Ranged<bool>(Options::get_global(), "rete-disable-node-sharing")) {
                            reteerror(&yylloc, yyscanner, agent, filename, source_path, "Illegal ancestral relationship specified in :feature.");
                            YYABORT;
                          }
                        }
                      }

                      parent->suppress_destruction(false);
                      delete (yyvsp[0].rule_ptr); }
#line 1824 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 404 "rules.yyy" /* yacc.c:1646  */
    { if(!get<0>(*(yyvsp[-2].flag_ptr)))
                                            get<0>(*(yyvsp[-2].flag_ptr)) = make_shared<int64_t>(agent.get_total_step_count());
                                          if(!get<1>(*(yyvsp[-2].flag_ptr)))
                                            get<1>(*(yyvsp[-2].flag_ptr)) = make_shared<tuple<int64_t, string, string, Carli::Feature *>>();
                                          (yyval.rule_ptr) = new tuple<pair<Rete::Rete_Node_Ptr, vector<Rete::Parser_Variable>>, string, tuple<shared_ptr<int64_t>, shared_ptr<tuple<int64_t, string, string, Carli::Feature *>>>, Carli::Q_Value::Token>(*(yyvsp[-1].rete_node_ptr), *(yyvsp[-3].sval), *(yyvsp[-2].flag_ptr), Carli::Q_Value::Token());
                                          delete (yyvsp[-3].sval);
                                          delete (yyvsp[-2].flag_ptr);
                                          delete (yyvsp[-1].rete_node_ptr); }
#line 1837 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 413 "rules.yyy" /* yacc.c:1646  */
    { if(!get<0>(*(yyvsp[-12].flag_ptr)))
            get<0>(*(yyvsp[-12].flag_ptr)) = make_shared<int64_t>(agent.get_total_step_count());
          if(!get<1>(*(yyvsp[-12].flag_ptr)))
            get<1>(*(yyvsp[-12].flag_ptr)) = make_shared<tuple<int64_t, string, string, Carli::Feature *>>();
          (yyval.rule_ptr) = new tuple<pair<Rete::Rete_Node_Ptr, vector<Rete::Parser_Variable>>, string, tuple<shared_ptr<int64_t>, shared_ptr<tuple<int64_t, string, string, Carli::Feature *>>>, Carli::Q_Value::Token>(*(yyvsp[-11].rete_node_ptr), *(yyvsp[-13].sval), *(yyvsp[-12].flag_ptr), Carli::Q_Value::Token((yyvsp[-8].fval), (yyvsp[-7].fval), (yyvsp[-6].fval), (yyvsp[-5].fval), (yyvsp[-4].fval), (yyvsp[-3].fval), (yyvsp[-2].fval), (yyvsp[-1].ival)));
          delete (yyvsp[-13].sval);
          delete (yyvsp[-12].flag_ptr);
          delete (yyvsp[-11].rete_node_ptr); }
#line 1850 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 422 "rules.yyy" /* yacc.c:1646  */
    { if(!get<0>(*(yyvsp[-5].flag_ptr)))
            get<0>(*(yyvsp[-5].flag_ptr)) = make_shared<int64_t>(agent.get_total_step_count());
          if(!get<1>(*(yyvsp[-5].flag_ptr)))
            get<1>(*(yyvsp[-5].flag_ptr)) = make_shared<tuple<int64_t, string, string, Carli::Feature *>>();
          (yyval.rule_ptr) = new tuple<pair<Rete::Rete_Node_Ptr, vector<Rete::Parser_Variable>>, string, tuple<shared_ptr<int64_t>, shared_ptr<tuple<int64_t, string, string, Carli::Feature *>>>, Carli::Q_Value::Token>(*(yyvsp[-4].rete_node_ptr), *(yyvsp[-6].sval), *(yyvsp[-5].flag_ptr), Carli::Q_Value::Token((yyvsp[-1].fval), 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0));
          delete (yyvsp[-6].sval);
          delete (yyvsp[-5].flag_ptr);
          delete (yyvsp[-4].rete_node_ptr); }
#line 1863 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 433 "rules.yyy" /* yacc.c:1646  */
    { (yyval.flag_ptr) = new tuple<shared_ptr<int64_t>, shared_ptr<tuple<int64_t, string, string, Carli::Feature *>>>; }
#line 1869 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 434 "rules.yyy" /* yacc.c:1646  */
    { if(get<0>(*(yyvsp[-2].flag_ptr))) {
                                     reteerror(&yylloc, yyscanner, agent, filename, source_path, "Flag :creation-time set more than once.");
                                     YYABORT;
                                   }
                                   (yyval.flag_ptr) = (yyvsp[-2].flag_ptr);
                                   get<0>(*(yyval.flag_ptr)) = make_shared<int64_t>((yyvsp[0].ival)); }
#line 1880 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 440 "rules.yyy" /* yacc.c:1646  */
    { if(get<1>(*(yyvsp[-7].flag_ptr))) {
                                                                reteerror(&yylloc, yyscanner, agent, filename, source_path, "Flag :feature set more than once.");
                                                                YYABORT;
                                                              }
                                                              (yyval.flag_ptr) = (yyvsp[-7].flag_ptr);
                                                              get<1>(*(yyval.flag_ptr)) = make_shared<tuple<int64_t, string, string, Carli::Feature *>>((yyvsp[-5].ival), *(yyvsp[-4].sval), *(yyvsp[-3].sval), new Carli::Feature_Ranged<Carli::Feature>(vector<Rete::WME>() /*FIXUP Later*/, Rete::WME_Bindings() /*FIXUP later*/, Rete::WME_Token_Index() /*FIXUP later*/, nullptr /*FIXUP later*/, (yyvsp[-1].fval), (yyvsp[0].fval), (yyvsp[-2].ival), false /*FIXUP later*/, false));
                                                              delete (yyvsp[-4].sval);
                                                              delete (yyvsp[-3].sval); }
#line 1893 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 448 "rules.yyy" /* yacc.c:1646  */
    { if(get<1>(*(yyvsp[-7].flag_ptr))) {
                                                            reteerror(&yylloc, yyscanner, agent, filename, source_path, "Flag :feature set more than once.");
                                                            YYABORT;
                                                          }
                                                          (yyval.flag_ptr) = (yyvsp[-7].flag_ptr);
                                                          get<1>(*(yyval.flag_ptr)) = make_shared<tuple<int64_t, string, string, Carli::Feature *>>((yyvsp[-5].ival), *(yyvsp[-4].sval), *(yyvsp[-3].sval), new Carli::Feature_Ranged<Carli::Feature>(vector<Rete::WME>() /*FIXUP Later*/, Rete::WME_Bindings() /*FIXUP later*/, Rete::WME_Token_Index() /*FIXUP later*/, nullptr /*FIXUP later*/, (yyvsp[-1].ival), (yyvsp[0].ival), (yyvsp[-2].ival), false /*FIXUP later*/, true));
                                                          delete (yyvsp[-4].sval);
                                                          delete (yyvsp[-3].sval); }
#line 1906 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 456 "rules.yyy" /* yacc.c:1646  */
    { if(get<1>(*(yyvsp[-4].flag_ptr))) {
                                                reteerror(&yylloc, yyscanner, agent, filename, source_path, "Flag :feature set more than once.");
                                                YYABORT;
                                              }
                                              (yyval.flag_ptr) = (yyvsp[-4].flag_ptr);
                                              if((yyvsp[-2].ival) > 1)
                                                get<1>(*(yyval.flag_ptr)) = make_shared<tuple<int64_t, string, string, Carli::Feature *>>((yyvsp[-2].ival), *(yyvsp[-1].sval), *(yyvsp[0].sval), new Carli::Feature_Enumerated<Carli::Feature>(vector<Rete::WME>() /*FIXUP Later*/, Rete::WME_Bindings() /*FIXUP later*/, Rete::WME_Token_Index() /*FIXUP later*/, nullptr /*FIXUP later*/, 0 /*FIXUP later*/));
                                              else
                                                get<1>(*(yyval.flag_ptr)) = make_shared<tuple<int64_t, string, string, Carli::Feature *>>((yyvsp[-2].ival), *(yyvsp[-1].sval), *(yyvsp[0].sval), nullptr);
                                              delete (yyvsp[-1].sval);
                                              delete (yyvsp[0].sval); }
#line 1922 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 469 "rules.yyy" /* yacc.c:1646  */
    { (yyval.rete_node_ptr) = (yyvsp[0].rete_node_ptr); }
#line 1928 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 470 "rules.yyy" /* yacc.c:1646  */
    { (yyval.rete_node_ptr) = Rete_Node_Ptr_and_Variables(Rete::Rete_Node_Ptr(agent.make_existential((yyvsp[0].rete_node_ptr)->first)), Rete::Variables()); delete (yyvsp[0].rete_node_ptr); }
#line 1934 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 471 "rules.yyy" /* yacc.c:1646  */
    { (yyval.rete_node_ptr) = Rete_Node_Ptr_and_Variables(Rete::Rete_Node_Ptr(agent.make_negation((yyvsp[0].rete_node_ptr)->first)), Rete::Variables()); delete (yyvsp[0].rete_node_ptr); }
#line 1940 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 472 "rules.yyy" /* yacc.c:1646  */
    { (yyval.rete_node_ptr) = Rete_Node_Ptr_and_Variables(Rete::Rete_Node_Ptr(agent.make_existential_join(Rete::WME_Bindings(), true, (yyvsp[-2].rete_node_ptr)->first, (yyvsp[0].rete_node_ptr)->first)), (yyvsp[-2].rete_node_ptr)->second);
                                                        delete (yyvsp[-2].rete_node_ptr);
                                                        delete (yyvsp[0].rete_node_ptr); }
#line 1948 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 477 "rules.yyy" /* yacc.c:1646  */
    { const vector<Rete::Parser_Variable> variables(merge_variables((yyvsp[-1].rete_node_ptr)->second, (yyvsp[-1].rete_node_ptr)->first->get_size(), (yyvsp[-1].rete_node_ptr)->first->get_token_size(), (yyvsp[0].rete_node_ptr)->second, false));
                              (yyval.rete_node_ptr) = Rete_Node_Ptr_and_Variables(Rete::Rete_Node_Ptr(agent.make_join(join_bindings((yyvsp[-1].rete_node_ptr)->second, (yyvsp[0].rete_node_ptr)->second), (yyvsp[-1].rete_node_ptr)->first, (yyvsp[0].rete_node_ptr)->first)), variables);
                              delete (yyvsp[-1].rete_node_ptr);
                              delete (yyvsp[0].rete_node_ptr); }
#line 1957 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 481 "rules.yyy" /* yacc.c:1646  */
    { const vector<Rete::Parser_Variable> variables(merge_variables((yyvsp[-2].rete_node_ptr)->second, (yyvsp[-2].rete_node_ptr)->first->get_size(), (yyvsp[-2].rete_node_ptr)->first->get_token_size(), (yyvsp[0].rete_node_ptr)->second, true));
                                    (yyval.rete_node_ptr) = Rete_Node_Ptr_and_Variables(Rete::Rete_Node_Ptr(agent.make_existential_join(join_bindings((yyvsp[-2].rete_node_ptr)->second, (yyvsp[0].rete_node_ptr)->second), false, (yyvsp[-2].rete_node_ptr)->first, (yyvsp[0].rete_node_ptr)->first)), variables);
                                    delete (yyvsp[-2].rete_node_ptr);
                                    delete (yyvsp[0].rete_node_ptr); }
#line 1966 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 485 "rules.yyy" /* yacc.c:1646  */
    { const vector<Rete::Parser_Variable> variables(merge_variables((yyvsp[-2].rete_node_ptr)->second, (yyvsp[-2].rete_node_ptr)->first->get_size(), (yyvsp[-2].rete_node_ptr)->first->get_token_size(), (yyvsp[0].rete_node_ptr)->second, true));
                                    (yyval.rete_node_ptr) = Rete_Node_Ptr_and_Variables(Rete::Rete_Node_Ptr(agent.make_negation_join(join_bindings((yyvsp[-2].rete_node_ptr)->second, (yyvsp[0].rete_node_ptr)->second), (yyvsp[-2].rete_node_ptr)->first, (yyvsp[0].rete_node_ptr)->first)), variables);
                                    delete (yyvsp[-2].rete_node_ptr);
                                    delete (yyvsp[0].rete_node_ptr); }
#line 1975 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 489 "rules.yyy" /* yacc.c:1646  */
    { const auto lhs_index = find_index((yyvsp[-5].rete_node_ptr)->second, *(yyvsp[-3].sval));
                                                            if(lhs_index.column > 2) {
                                                              reteerror(&yylloc, yyscanner, agent, filename, source_path, "Unbound variable tested by predicate.");
                                                              YYABORT;
                                                            }
                                                            (yyval.rete_node_ptr) = Rete_Node_Ptr_and_Variables(Rete::Rete_Node_Ptr(agent.make_predicate_vc((yyvsp[-2].predicate), lhs_index, *(yyvsp[-1].symbol_ptr), (yyvsp[-5].rete_node_ptr)->first)), (yyvsp[-5].rete_node_ptr)->second);
                                                            delete (yyvsp[-5].rete_node_ptr);
                                                            delete (yyvsp[-3].sval);
                                                            delete (yyvsp[-1].symbol_ptr); }
#line 1989 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 498 "rules.yyy" /* yacc.c:1646  */
    { const auto lhs_index = find_index((yyvsp[-5].rete_node_ptr)->second, *(yyvsp[-3].sval));
                                                     const auto rhs_index = find_index((yyvsp[-5].rete_node_ptr)->second, *(yyvsp[-1].sval));
                                                     if(lhs_index.column > 2 || rhs_index.column > 2) {
                                                       reteerror(&yylloc, yyscanner, agent, filename, source_path, "Unbound variable tested by predicate.");
                                                       YYABORT;
                                                     }
                                                     (yyval.rete_node_ptr) = Rete_Node_Ptr_and_Variables(Rete::Rete_Node_Ptr(agent.make_predicate_vv((yyvsp[-2].predicate), lhs_index, rhs_index, (yyvsp[-5].rete_node_ptr)->first)), (yyvsp[-5].rete_node_ptr)->second);
                                                     delete (yyvsp[-5].rete_node_ptr);
                                                     delete (yyvsp[-3].sval);
                                                     delete (yyvsp[-1].sval); }
#line 2004 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 508 "rules.yyy" /* yacc.c:1646  */
    { (yyval.rete_node_ptr) = (yyvsp[0].rete_node_ptr); }
#line 2010 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 511 "rules.yyy" /* yacc.c:1646  */
    { (yyval.rete_node_ptr) = (yyvsp[-1].rete_node_ptr); }
#line 2016 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 514 "rules.yyy" /* yacc.c:1646  */
    { (yyval.rete_node_ptr) = (yyvsp[0].rete_node_ptr); }
#line 2022 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 515 "rules.yyy" /* yacc.c:1646  */
    { (yyval.rete_node_ptr) = (yyvsp[0].rete_node_ptr); }
#line 2028 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 518 "rules.yyy" /* yacc.c:1646  */
    { (yyval.rete_node_ptr) = Rete_Node_Ptr_and_Variables(Rete::Rete_Node_Ptr(agent.make_filter(Rete::WME(make_shared<Rete::Symbol_Variable>(Rete::Symbol_Variable::First), *(yyvsp[-2].symbol_ptr), *(yyvsp[-1].symbol_ptr)))), Variables(Rete::Parser_Variable(*(yyvsp[-4].sval), "", ""))); delete (yyvsp[-4].sval); delete (yyvsp[-2].symbol_ptr); delete (yyvsp[-1].symbol_ptr); }
#line 2034 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 519 "rules.yyy" /* yacc.c:1646  */
    { (yyval.rete_node_ptr) = Rete_Node_Ptr_and_Variables(Rete::Rete_Node_Ptr(agent.make_filter(Rete::WME(make_shared<Rete::Symbol_Variable>(Rete::Symbol_Variable::First), *(yyvsp[-2].symbol_ptr), make_shared<Rete::Symbol_Variable>(*(yyvsp[-4].sval) == *(yyvsp[-1].sval) ? Rete::Symbol_Variable::First : Rete::Symbol_Variable::Third)))), Variables(Rete::Parser_Variable(*(yyvsp[-4].sval), "", *(yyvsp[-1].sval)))); delete (yyvsp[-4].sval); delete (yyvsp[-2].symbol_ptr); delete (yyvsp[-1].sval); }
#line 2040 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 520 "rules.yyy" /* yacc.c:1646  */
    { (yyval.rete_node_ptr) = Rete_Node_Ptr_and_Variables(Rete::Rete_Node_Ptr(agent.make_filter(Rete::WME(make_shared<Rete::Symbol_Variable>(Rete::Symbol_Variable::First), make_shared<Rete::Symbol_Variable>(*(yyvsp[-4].sval) == *(yyvsp[-2].sval) ? Rete::Symbol_Variable::First : Rete::Symbol_Variable::Second), *(yyvsp[-1].symbol_ptr)))), Variables(Rete::Parser_Variable(*(yyvsp[-4].sval), *(yyvsp[-2].sval), ""))); delete (yyvsp[-4].sval); delete (yyvsp[-2].sval); delete (yyvsp[-1].symbol_ptr); }
#line 2046 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 521 "rules.yyy" /* yacc.c:1646  */
    { (yyval.rete_node_ptr) = Rete_Node_Ptr_and_Variables(Rete::Rete_Node_Ptr(agent.make_filter(Rete::WME(make_shared<Rete::Symbol_Variable>(Rete::Symbol_Variable::First), make_shared<Rete::Symbol_Variable>(*(yyvsp[-4].sval) == *(yyvsp[-2].sval) ? Rete::Symbol_Variable::First : Rete::Symbol_Variable::Second), make_shared<Rete::Symbol_Variable>(*(yyvsp[-4].sval) == *(yyvsp[-1].sval) ? Rete::Symbol_Variable::First : *(yyvsp[-2].sval) == *(yyvsp[-1].sval) ? Rete::Symbol_Variable::Second : Rete::Symbol_Variable::Third)))), Variables(Rete::Parser_Variable(*(yyvsp[-4].sval), *(yyvsp[-2].sval), *(yyvsp[-1].sval)))); delete (yyvsp[-4].sval); delete (yyvsp[-2].sval); delete (yyvsp[-1].sval); }
#line 2052 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 524 "rules.yyy" /* yacc.c:1646  */
    { (yyval.symbol_ptr) = (yyvsp[0].symbol_ptr); }
#line 2058 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 525 "rules.yyy" /* yacc.c:1646  */
    { (yyval.symbol_ptr) = (yyvsp[0].symbol_ptr); }
#line 2064 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 528 "rules.yyy" /* yacc.c:1646  */
    { (yyval.symbol_ptr) = new Rete::Symbol_Ptr_C(make_shared<Rete::Symbol_Identifier>(*(yyvsp[0].sval))); delete (yyvsp[0].sval); }
#line 2070 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 531 "rules.yyy" /* yacc.c:1646  */
    { (yyval.symbol_ptr) = new Rete::Symbol_Ptr_C(make_shared<Rete::Symbol_Constant_Float>((yyvsp[0].fval))); }
#line 2076 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 532 "rules.yyy" /* yacc.c:1646  */
    { (yyval.symbol_ptr) = new Rete::Symbol_Ptr_C(make_shared<Rete::Symbol_Constant_Int>((yyvsp[0].ival))); }
#line 2082 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 42:
#line 533 "rules.yyy" /* yacc.c:1646  */
    { (yyval.symbol_ptr) = new Rete::Symbol_Ptr_C(make_shared<Rete::Symbol_Constant_String>(*(yyvsp[0].sval))); delete (yyvsp[0].sval); }
#line 2088 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 43:
#line 536 "rules.yyy" /* yacc.c:1646  */
    { (yyval.fval) = double((yyvsp[0].ival)); }
#line 2094 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 44:
#line 537 "rules.yyy" /* yacc.c:1646  */
    { (yyval.fval) = (yyvsp[0].fval); }
#line 2100 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 45:
#line 540 "rules.yyy" /* yacc.c:1646  */
    { (yyval.sval) = (yyvsp[0].sval); }
#line 2106 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 46:
#line 541 "rules.yyy" /* yacc.c:1646  */
    { (yyval.sval) = (yyvsp[0].sval); }
#line 2112 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 47:
#line 544 "rules.yyy" /* yacc.c:1646  */
    { (yyval.sval) = (yyvsp[-1].sval); }
#line 2118 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 48:
#line 547 "rules.yyy" /* yacc.c:1646  */
    { (yyval.sval) = (yyvsp[-1].sval); *(yyval.sval) += (yyvsp[0].cval); }
#line 2124 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 49:
#line 548 "rules.yyy" /* yacc.c:1646  */
    { (yyval.sval) = (yyvsp[-1].sval); *(yyval.sval) += *(yyvsp[0].sval); delete (yyvsp[0].sval); }
#line 2130 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 50:
#line 549 "rules.yyy" /* yacc.c:1646  */
    { (yyval.sval) = new string; *(yyval.sval) += (yyvsp[0].cval); }
#line 2136 "rules.tab.cpp" /* yacc.c:1646  */
    break;

  case 51:
#line 550 "rules.yyy" /* yacc.c:1646  */
    { (yyval.sval) = (yyvsp[0].sval); }
#line 2142 "rules.tab.cpp" /* yacc.c:1646  */
    break;


#line 2146 "rules.tab.cpp" /* yacc.c:1646  */
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
  *++yylsp = yyloc;

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
      yyerror (&yylloc, yyscanner, agent, filename, source_path, YY_("syntax error"));
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
        yyerror (&yylloc, yyscanner, agent, filename, source_path, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }

  yyerror_range[1] = yylloc;

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
                      yytoken, &yylval, &yylloc, yyscanner, agent, filename, source_path);
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

  yyerror_range[1] = yylsp[1-yylen];
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

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, yylsp, yyscanner, agent, filename, source_path);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
  *++yylsp = yyloc;

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
  yyerror (&yylloc, yyscanner, agent, filename, source_path, YY_("memory exhausted"));
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
                  yytoken, &yylval, &yylloc, yyscanner, agent, filename, source_path);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, yylsp, yyscanner, agent, filename, source_path);
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
#line 553 "rules.yyy" /* yacc.c:1906  */


#include "rete_parser.cxx"
#ifdef _MSC_VER
#pragma warning(pop)
#endif
