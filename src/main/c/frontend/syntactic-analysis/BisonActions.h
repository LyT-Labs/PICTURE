#ifndef BISON_ACTIONS_HEADER
#define BISON_ACTIONS_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/CompilerState.h"
#include "../../support/type/ModuleDestructor.h"
#include "../../support/type/TokenLabel.h"
#include "AbstractSyntaxTree.h"
#include "BisonParser.h"
#include <stdlib.h>

/** Initialize module's internal state. */
ModuleDestructor initializeBisonActionsModule();

/**
 * PICTURE semantic actions.
 */

// Value actions
Value * StringValueSemanticAction(const char * stringValue);
Value * NumberValueSemanticAction(const int numberValue);
Value * IdentifierValueSemanticAction(const char * identifierValue);
Value * BuiltinValueSemanticAction(const char * builtinValue);
Value * AppendIdentifierSemanticAction(Value * list, const char * identifier);

// Property actions
Property * PropertySemanticAction(const char * key, Value * value);
PropertyList * CreatePropertyListSemanticAction();
PropertyList * AddPropertySemanticAction(PropertyList * list, Property * property);

// Component actions
Component * ComponentSemanticAction(const char * id, PropertyList * properties, ComponentList * children);
ComponentList * CreateComponentListSemanticAction();
ComponentList * AddComponentSemanticAction(ComponentList * list, Component * component);

// Variable actions
Variable * VariableSemanticAction(const char * name, Value * value);
VariableList * CreateVariableListSemanticAction();
VariableList * AddVariableSemanticAction(VariableList * list, Variable * variable);

// Program action
Program * ProgramSemanticAction(VariableList * variables, ComponentList * components);

#endif
