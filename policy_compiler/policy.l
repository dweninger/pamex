%{
#include "IOMngr.h"
#include "y.tab.h"

#define YY_INPUT(buf,result,max_size) \
	{ int c = getNextSourceChar(); \
          result = (c == EOF) ? YY_NULL : (buf[0] = c, 1); \
        }

%}


letter [A-Za-z]
digit [0-9]

%%

set		{return SET;}
allow		{return ALLOW;}
deny		{return DENY;}
read		{return READ;}
write		{return WRITE;}
execute		{return EXECUTE;}
extract		{return EXTRACT;}

\<		{return '<';}
\>		{return '>';}
\:		{return ':';}
\;		{return ';';}
\,		{return ';';}
\(		{return '(';}
\)		(return ')';}
\{		{return '{';}
\}		{return '}';}

.		;

%%

int yywrap() {
	return 1;
}
