%{
/*******************************************************************************
   Project : C script
   File    : lex.l
   Date    : 2018.3.18-
   Note    : 
*******************************************************************************/
#include "parser.h"
#include "name.h"
#include "decl.h"
#include "expr.h"
#include "stmt.h"
#include "btree.h"
#include "hashmap.h"
#include "parse.tab.h"

#define YY_DECL int Ebcscript_Parser_yylex(ebcscript_parser *Prs)

static int yywrap(void)
{
	return 1;
}

%}
%pointer
%x COMMENT

/* digit */
DG	[0-9]

/* nonzero-digit */
NZ	[1-9]

/* nondigit */
ND	[a-zA-Z_]

/* octal-digit */
OD	[0-7]

/* hexadecimal-digit */
HD	[a-fA-F0-9]

/* unsigned-suffix */
US	[uU]

/* long-suffix */
LS	[lL]

/* floating-suffix */
FS	[flFL]

/* integer-suffix */
IS	({US}{LS}?|{LS}{US}?)

/* exponent-part */
EP	([eE][+-]?{DG}+)

%%
";"		{ return(';'); }
"{"		{ return('{'); }
"}"		{ return('}'); }
","		{ return(','); }
":"		{ return(':'); }
"="		{ return('='); }
"("		{ return('('); }
")"		{ return(')'); }
"["		{ return('['); }
"]"		{ return(']'); }
"."		{ return('.'); }
"&"		{ return('&'); }
"!"		{ return('!'); }
"~"		{ return('~'); }
"-"		{ return('-'); }
"+"		{ return('+'); }
"*"		{ return('*'); }
"/"		{ return('/'); }
"%"		{ return('%'); }
"<"		{ return('<'); }
">"		{ return('>'); }
"^"		{ return('^'); }
"|"		{ return('|'); }
"?"		{ return('?'); }
">>"		{ return(RIGHT_OP); }
"<<"		{ return(LEFT_OP); }
"++"		{ return(INC_OP); }
"--"		{ return(DEC_OP); }
"->"		{ return(PTR_OP); }
"&&"		{ return(AND_OP); }
"||"		{ return(OR_OP); }
"<="		{ return(LE_OP); }
">="		{ return(GE_OP); }
"=="		{ return(EQ_OP); }
"!="		{ return(NE_OP); }
">>="		{ return(RIGHT_ASSIGN); }
"<<="		{ return(LEFT_ASSIGN); }
"+="		{ return(ADD_ASSIGN); }
"-="		{ return(SUB_ASSIGN); }
"*="		{ return(MUL_ASSIGN); }
"/="		{ return(DIV_ASSIGN); }
"%="		{ return(MOD_ASSIGN); }
"&="		{ return(AND_ASSIGN); }
"^="		{ return(XOR_ASSIGN); }
"|="		{ return(OR_ASSIGN); }
"/*"		{ BEGIN COMMENT; }
[ \t\v\f\r]	;
\n		{ Prs->Line++; }

break		{ return(BREAK); }
case		{ return(CASE); }
char		{ return(CHAR); }
const		{ return(CONST); }
continue	{ return(CONTINUE); }
default		{ return(DEFAULT); }
do		{ return(DO); }
double		{ return(DOUBLE); }
else		{ return(ELSE); }
enum		{ return(ENUM); }
extern		{ return(EXTERN); }
float		{ return(FLOAT); }
for		{ return(FOR); }
goto		{ return(GOTO); }
if		{ return(IF); }
int		{ return(INT); }
long		{ return(LONG); }
return		{ return(RETURN); }
short		{ return(SHORT); }
signed		{ return(SIGNED); }
sizeof		{ return(SIZEOF); }
static		{ return(STATIC); }
struct		{ return(STRUCT); }
switch		{ return(SWITCH); }
typedef		{ return(TYPEDEF); }
union		{ return(UNION); }
unsigned	{ return(UNSIGNED); }
void		{ return(VOID); }
volatile	{ return(VOLATILE); }
while		{ return(WHILE); }

{ND}({ND}|{DG})* {
		  ebcscript_name **N;
		  ebcscript_parser_blockstack *BS;

		  /* typedef名として登録済み？ */
		  for (BS = Prs->BS; BS != NULL; BS = BS->Prev) {
		    if ((N = Ebcscript_Parser_Blockstack_findVarFuncTypeEnum(
			                                           BS, yytext))
		     && (*N)->Kind == EBCSCRIPT_NAME_KIND_TYPEDEF) {
		      Ebcscript_Parser_yylval.Name = *N;
		      return(TYPEDEF_NAME);
		    }
		  }
		  Ebcscript_Parser_yylval.Identifier =
		                                  Ebcscript_Name_malloc(yyleng);
		  strcpy(Ebcscript_Parser_yylval.Identifier, yytext);
		  return(IDENTIFIER);
		}

0(x|X){HD}+{IS}? |
0{OD}*{IS}?	|
{NZ}{DG}*{IS}?	{
		  ebcscript_literal *L;
		  char *PEnd;

		  L = Ebcscript_newLiteral(yytext);
		  L->Kind = EBCSCRIPT_LITERAL_KIND_INTEGER;
		  L->TypeTree = Ebcscript_newType_long();
		  errno = 0;
		  L->As.Integer = strtol(yytext, &PEnd, 0);
		  if (errno == ERANGE) {
	            Ebcscript_Parser_log(
	             "%s:%d: warning: overflow in implicit constant "
		                                                 "conversion\n",
	             Prs->Filename, Prs->Line);
		  }
		  Ebcscript_Parser_yylval.Literal = L;
		  return(CONSTANT);
		}

{DG}+{EP}{FS}?	|
(({DG}*"."{DG}+)|({DG}+".")){EP}?{FS}? {
		  ebcscript_literal *L;
		  char *PEnd;

		  L = Ebcscript_newLiteral(yytext);
		  L->Kind = EBCSCRIPT_LITERAL_KIND_FLOATING;
		  L->TypeTree = Ebcscript_newType_double();
		  errno = 0;
		  L->As.Floating = strtod(yytext, &PEnd);
		  if (errno == ERANGE) {
	            Ebcscript_Parser_log(
	             "%s:%d: warning: overflow in implicit constant "
		                                                 "conversion\n",
	             Prs->Filename, Prs->Line);
		  }
		  Ebcscript_Parser_yylval.Literal = L;
		  return(CONSTANT);
		}

\"([^\"]|\\\")*\"([ \n\t\v\f\r]*\"([^\"]|\\\")*\")* {
		  ebcscript_literal *L;

		  L = Ebcscript_newLiteral_string(yytext);
		  L->Kind = EBCSCRIPT_LITERAL_KIND_STRING;
		  L->TypeTree = Ebcscript_newType_pointer();
		  L->TypeTree->As.Pointer.Value = Ebcscript_newType_char();
		  L->As.String = L->Text;
		  Ebcscript_Parser_yylval.Literal = L;
		  return(STRING_LITERAL);
		}

.		{ return(yytext[0]); }

<COMMENT>"*/"	{ BEGIN INITIAL; }
<COMMENT>\n	;
<COMMENT>.	;

%%
