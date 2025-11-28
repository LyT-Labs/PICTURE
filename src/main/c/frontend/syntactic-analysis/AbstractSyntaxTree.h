#ifndef ABSTRACT_SYNTAX_TREE_HEADER
#define ABSTRACT_SYNTAX_TREE_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/ModuleDestructor.h"
#include <stdlib.h>

/** Initialize module's internal state. */
ModuleDestructor initializeAbstractSyntaxTreeModule();

/**
 * This type definitions allows self-referencing types (e.g., an expression
 * that is made of another expressions, such as talking about you in 3rd
 * person, but without the madness).
 */


typedef enum ValueType ValueType;
typedef enum PropertyType PropertyType;

typedef enum ExpressionType ExpressionType;
typedef enum FactorType FactorType;

typedef struct Constant Constant;
typedef struct Expression Expression;
typedef struct Factor Factor;

typedef struct Value Value;
typedef struct Property Property;
typedef struct PropertyList PropertyList;
typedef struct Component Component;
typedef struct ComponentList ComponentList;
typedef struct Variable Variable;
typedef struct VariableList VariableList;
typedef struct Program Program;

/**
 * Node types for the PICTURE AST.
 */

enum ValueType {
	VALUE_STRING,    // "text"
	VALUE_NUMBER,    // 100
	VALUE_IDENTIFIER,// textSize, color, func
	VALUE_BUILTIN    // m, l, s, red, blue, center
};

/**
 * Legacy enums for backward compatibility.
 */
enum ExpressionType {
	ADDITION,
	DIVISION,
	FACTOR,
	MULTIPLICATION,
	SUBTRACTION
};

enum FactorType {
	CONSTANT,
	EXPRESSION
};

enum PropertyType {
	PROP_BACKGROUND,
	PROP_TEXT,
	PROP_FONT_SIZE,
	PROP_COLOR,
	PROP_ON_PRESS,
	PROP_ALIGN,
	PROP_WIDTH,
	PROP_HEIGHT,
	PROP_X,
	PROP_Y,
	PROP_ON_SELECT,
	PROP_ON_KEYPRESS,
	PROP_ON_FOCUS_GAIN,
	PROP_ON_FOCUS_LOST
};

struct Value {
	ValueType type;
	union {
		char * stringValue;
		int numberValue;
		char * identifierValue;
	};
};

struct Property {
	PropertyType type;
	char * key;      // "background", "text", etc.
	Value * value;
	Property * next; // linked list
};

struct PropertyList {
	Property * first;
	Property * last;
};

struct Component {
	char * id;                    // component identifier after #
	PropertyList * properties;    // list of properties with -
	ComponentList * children;     // nested components (indented)
	Component * next;             // sibling components
};

struct ComponentList {
	Component * first;
	Component * last;
};

struct Variable {
	char * name;     // variable name
	Value * value;   // variable value
	Variable * next; // linked list
};

struct VariableList {
	Variable * first;
	Variable * last;
};

struct Program {
	VariableList * variables;   // --- section variables
	char ** selectionOrder;     // * selection_order: id1, id2, id3
	int selectionOrderCount;    // cantidad de IDs en selection_order
	char * identifier;          // * identifier: screen_name
	ComponentList * components; // main component tree
	
	// Legacy field for backward compatibility
	Expression * expression;    // Keep this for old backend
};

/**
 * Legacy structs for backward compatibility.
 */
struct Constant {
	int value;
};

struct Factor {
	union {
		Constant * constant;
		Expression * expression;
	};
	FactorType type;
};

struct Expression {
	union {
		Factor * factor;
		struct {
			Expression * leftExpression;
			Expression * rightExpression;
		};
	};
	ExpressionType type;
};

/**
 * Node recursive destructors for PICTURE AST.
 */

void destroyValue(Value * value);
void destroyProperty(Property * property);
void destroyPropertyList(PropertyList * propertyList);
void destroyComponent(Component * component);
void destroyComponentList(ComponentList * componentList);
void destroyVariable(Variable * variable);
void destroyVariableList(VariableList * variableList);
void destroyProgram(Program * program);

/**
 * Legacy destructors for backward compatibility.
 */
void destroyConstant(Constant * constant);
void destroyExpression(Expression * expression);
void destroyFactor(Factor * factor);

#endif
