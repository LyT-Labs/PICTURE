%{

#include "../../support/type/TokenLabel.h"
#include "AbstractSyntaxTree.h"
#include "BisonActions.h"

/**
 * The error reporting function for Bison parser.
 *
 * @todo Add location to the grammar and "pushToken" API function.
 *
 * @see https://www.gnu.org/software/bison/manual/html_node/Error-Reporting-Function.html
 * @see https://www.gnu.org/software/bison/manual/html_node/Tracking-Locations.html
 */
void yyerror(const YYLTYPE * location, const char * message) {}

%}

// You touch this, and you die.
%define api.pure full
%define api.push-pull push
%define api.value.union.name SemanticValue
%define parse.error detailed
%locations

%union {
	/** Terminals. */

	signed int integer;
	char * string;
	TokenLabel token;

	/** Non-terminals. */

	Value * value;
	Property * property;
	PropertyList * propertyList;
	Component * component;
	ComponentList * componentList;
	Variable * variable;
	VariableList * variableList;
	Program * program;

	/** Legacy types for backend compatibility. */
	Constant * constant;
	Expression * expression;
	Factor * factor;
}

/**
 * Destructors. This functions are executed after the parsing ends, so if the
 * AST must be used in the following phases of the compiler you shouldn't used
 * this approach for the AST root node ("program" non-terminal, in this
 * grammar), or it will drop the entire tree even if the parsing succeeds.
 *
 * @see https://www.gnu.org/software/bison/manual/html_node/Destructor-Decl.html
 */
/** Destructors for string tokens - critical to avoid leaks on parse errors */
%destructor { if ($$) free($$); } <string>

%destructor { destroyValue($$); } <value>
%destructor { destroyProperty($$); } <property>
%destructor { destroyPropertyList($$); } <propertyList>
%destructor { destroyComponent($$); } <component>
%destructor { destroyComponentList($$); } <componentList>
%destructor { destroyVariable($$); } <variable>
%destructor { destroyVariableList($$); } <variableList>

/** Legacy destructors - not used in grammar, only for backend compatibility */
/** Commented out to avoid Bison warnings about unused types */
// %destructor { destroyConstant($$); } <constant>
// %destructor { destroyExpression($$); } <expression>
// %destructor { destroyFactor($$); } <factor>

/** PICTURE Terminals. */
%token <token> VARIABLE_DELIMITER    // ---
%token <string> COMPONENT_ID         // #identifier
%token <string> PROPERTY             // - property_name
%token <token> COLON                 // :
%token <string> STRING               // "text"
%token <integer> NUMBER              // 123
%token <string> IDENTIFIER           // variable_name
%token <string> BUILTIN              // m, l, red, etc
%token <token> INDENT                // indentation increase
%token <token> DEDENT                // indentation decrease
%token <token> NEWLINE               // line break

%token <token> COMMENT               // // comment
%token <token> IGNORED               // whitespace, tabs
%token <token> UNKNOWN               // unrecognized characters

/** Non-terminals. */
%type <value> value
%type <property> property
%type <propertyList> property_list
%type <component> component
%type <componentList> component_list
%type <variable> variable
%type <variableList> variable_list
%type <variableList> variable_section
%type <program> program
%type <program> declarations

/**
 * No precedence needed for PICTURE - it's declarative, not expression-based.
 */

%%

// PICTURE Grammar Rules

program: %empty												{ $$ = ProgramSemanticAction(NULL, NULL); if (!$$) YYERROR; }
	| newlines												{ $$ = ProgramSemanticAction(NULL, NULL); if (!$$) YYERROR; }
	| declarations											{ $$ = $1; }
	| newlines declarations									{ $$ = $2; }
	;

declarations: variable_section component_list				{ $$ = ProgramSemanticAction($1, $2); if (!$$) YYERROR; }
	| variable_section newlines component_list				{ $$ = ProgramSemanticAction($1, $3); if (!$$) YYERROR; }
	| component_list										{ $$ = ProgramSemanticAction(NULL, $1); if (!$$) YYERROR; }
	| variable_section										{ $$ = ProgramSemanticAction($1, NULL); if (!$$) YYERROR; }
	| variable_section newlines								{ $$ = ProgramSemanticAction($1, NULL); if (!$$) YYERROR; }
	;

variable_section: VARIABLE_DELIMITER newlines variable_list VARIABLE_DELIMITER
														{ $$ = $3; }
	| VARIABLE_DELIMITER newlines VARIABLE_DELIMITER
														{ $$ = CreateVariableListSemanticAction(); }
	;

variable_list: variable newlines							{ $$ = CreateVariableListSemanticAction(); $$ = AddVariableSemanticAction($$, $1); }
	| variable_list variable newlines						{ $$ = AddVariableSemanticAction($1, $2); }
	;

variable: PROPERTY COLON value								{ $$ = VariableSemanticAction($1, $3); }
	;

component_list: component									{ if (!$1) YYERROR; $$ = CreateComponentListSemanticAction(); $$ = AddComponentSemanticAction($$, $1); }
	| component_list component								{ if (!$2) YYERROR; $$ = AddComponentSemanticAction($1, $2); }
	;

component: COMPONENT_ID newlines							{ $$ = ComponentSemanticAction($1, NULL, NULL); if (!$$) YYERROR; }
	| COMPONENT_ID newlines INDENT property_list DEDENT		{ $$ = ComponentSemanticAction($1, $4, NULL); if (!$$) YYERROR; }
	| COMPONENT_ID newlines INDENT property_list component_list DEDENT
														{ $$ = ComponentSemanticAction($1, $4, $5); if (!$$) YYERROR; }
	| COMPONENT_ID newlines INDENT component_list DEDENT	{ $$ = ComponentSemanticAction($1, NULL, $4); if (!$$) YYERROR; }
	;

property_list: property newlines							{ if (!$1) YYERROR; $$ = CreatePropertyListSemanticAction(); $$ = AddPropertySemanticAction($$, $1); }
	| property_list property newlines						{ if (!$2) YYERROR; $$ = AddPropertySemanticAction($1, $2); }
	;

property: PROPERTY COLON value								{ $$ = PropertySemanticAction($1, $3); if (!$$) YYERROR; }
	;

value: STRING												{ $$ = StringValueSemanticAction($1); }
	| NUMBER												{ $$ = NumberValueSemanticAction($1); }
	| IDENTIFIER											{ $$ = IdentifierValueSemanticAction($1); }
	| BUILTIN												{ $$ = BuiltinValueSemanticAction($1); }
	;

newlines: NEWLINE
	| newlines NEWLINE
	;

%%
