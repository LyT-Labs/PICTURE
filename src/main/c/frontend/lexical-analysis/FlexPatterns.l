%top{

#include "../../support/type/CompilationStatus.h"
#include "../../support/type/FlexContext.h"
#include "../../support/type/TokenLabel.h"
#include "../syntactic-analysis/AbstractSyntaxTree.h"
#include "../syntactic-analysis/BisonParser.h"

}

%{

/**
 * PICTURE lexical analyzer patterns.
 * @see https://westes.github.io/flex/manual/Definitions-Section.html#Definitions-Section
 */

extern CompilationStatus VariableDelimiterLexemeAction();
extern CompilationStatus SelectionOrderLexemeAction();
extern CompilationStatus IdentifierKeywordLexemeAction();
extern CompilationStatus ComponentIdLexemeAction();
extern CompilationStatus PropertyLexemeAction();
extern CompilationStatus ColonLexemeAction();
extern CompilationStatus CommaLexemeAction();
extern CompilationStatus StringLexemeAction();
extern CompilationStatus NumberLexemeAction();
extern CompilationStatus IdentifierLexemeAction();
extern CompilationStatus BuiltinLexemeAction();
extern CompilationStatus CommentLexemeAction();
extern CompilationStatus NewlineLexemeAction();
extern CompilationStatus IndentLexemeAction();
extern CompilationStatus DedentLexemeAction();
extern CompilationStatus EOFLexemeAction();
extern CompilationStatus IgnoredLexemeAction();
extern CompilationStatus UnknownLexemeAction();

%}

/**
 * Reusable patterns for PICTURE language.
 *
 * @see https://westes.github.io/flex/manual/Matching.html
 * @see https://westes.github.io/flex/manual/Patterns.html
 */
identifier			[a-zA-Z_][a-zA-Z0-9_]*
string				\"([^\"\\]|\\.)*\"
number				[0-9]+
builtin				(s|m|l|red|blue|green|white|black|center|left|right)
whitespace			[ ]
tab					\t

%%

%{

/**
 * PICTURE language lexical rules.
 * @see https://westes.github.io/flex/manual/Rules-Section.html#Rules-Section
 */

%}

"---"									{ return VariableDelimiterLexemeAction(); }
"*"{whitespace}*"selection_order"	{ return SelectionOrderLexemeAction(); }
"*"{whitespace}*"identifier"		{ return IdentifierKeywordLexemeAction(); }

^({tab}|{whitespace}{whitespace})+	{ return IndentLexemeAction(); }
^"#"{whitespace}*{identifier}		{ return ComponentIdLexemeAction(); }
^"-"{whitespace}*{identifier}		{ return PropertyLexemeAction(); }

"#"{whitespace}*{identifier}		{ return ComponentIdLexemeAction(); }
"-"{whitespace}*{identifier}		{ return PropertyLexemeAction(); }
":"									{ return ColonLexemeAction(); }
","									{ return CommaLexemeAction(); }

{string}							{ return StringLexemeAction(); }
{number}							{ return NumberLexemeAction(); }
{builtin}							{ return BuiltinLexemeAction(); }
{identifier}						{ return IdentifierLexemeAction(); }

"//"[^\r\n]*						{ return CommentLexemeAction(); }

[\r\n]+								{ return NewlineLexemeAction(); }
{whitespace}+						{ return IgnoredLexemeAction(); }
.									{ return UnknownLexemeAction(); }

<<EOF>>								{ return EOFLexemeAction(); }

%%

/**
 * @see https://westes.github.io/flex/manual/User-Code-Section.html#User-Code-Section
 */

#include "FlexActions.h"
#include "FlexExport.h"
