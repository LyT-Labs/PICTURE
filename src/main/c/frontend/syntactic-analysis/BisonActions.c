#include "BisonActions.h"
#include <string.h>

/* MODULE INTERNAL STATE */

static CompilerState * _compilerState = NULL;
static Logger * _logger = NULL;

/* Semantic validation state */
#define MAX_COMPONENTS 256
static char * _componentIds[MAX_COMPONENTS];
static int _componentCount = 0;
static bool _hasSemanticError = false;

/* Valid property names catalog */
static const char * VALID_PROPERTIES[] = {
	"background", "text", "font_size", "color", "on_press", "on_select", 
	"on_keypress", "on_focus_gain", "on_focus_lost", "align", "width", "height", "x", "y", NULL
};

/* Helper function to check if property is valid */
static bool _isValidProperty(const char * propertyName) {
	for (int i = 0; VALID_PROPERTIES[i] != NULL; i++) {
		if (strcmp(VALID_PROPERTIES[i], propertyName) == 0) {
			return true;
		}
	}
	return false;
}

/* Helper function to check if ID is unique */
static bool _isUniqueId(const char * id) {
	for (int i = 0; i < _componentCount; i++) {
		if (strcmp(_componentIds[i], id) == 0) {
			return false;
		}
	}
	return true;
}

/* Helper function to register a component ID */
static void _registerComponentId(const char * id) {
	if (_componentCount < MAX_COMPONENTS) {
		_componentIds[_componentCount++] = (char *)id;
	}
}

/* Reset validation state */
static void _resetValidationState() {
	_componentCount = 0;
	_hasSemanticError = false;
	for (int i = 0; i < MAX_COMPONENTS; i++) {
		_componentIds[i] = NULL;
	}
}

/** Shutdown module's internal state. */
void _shutdownBisonActionsModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: BisonActions...");
		destroyLogger(_logger);
		_logger = NULL;
	}
	_resetValidationState();
	_compilerState = NULL;
}

ModuleDestructor initializeBisonActionsModule(CompilerState * compilerState) {
	_compilerState = compilerState;
	_logger = createLogger("BisonActions");
	_resetValidationState();
	return _shutdownBisonActionsModule;
}

/* PRIVATE FUNCTIONS */

static void _logSyntacticAnalyzerAction(const char * functionName);

/**
 * Logs a syntactic-analyzer action in DEBUGGING level.
 */
static void _logSyntacticAnalyzerAction(const char * functionName) {
	logDebugging(_logger, "%s", functionName);
}

/* PUBLIC FUNCTIONS - VALUE ACTIONS */

Value * StringValueSemanticAction(const char * stringValue) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Value * value = calloc(1, sizeof(Value));
	value->type = VALUE_STRING;
	value->stringValue = (char *)stringValue;
	return value;
}

Value * NumberValueSemanticAction(const int numberValue) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Value * value = calloc(1, sizeof(Value));
	value->type = VALUE_NUMBER;
	value->numberValue = numberValue;
	return value;
}

Value * IdentifierValueSemanticAction(const char * identifierValue) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Value * value = calloc(1, sizeof(Value));
	value->type = VALUE_IDENTIFIER;
	value->identifierValue = (char *)identifierValue;
	return value;
}

Value * BuiltinValueSemanticAction(const char * builtinValue) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Value * value = calloc(1, sizeof(Value));
	value->type = VALUE_BUILTIN;
	value->identifierValue = (char *)builtinValue;
	return value;
}

Value * AppendIdentifierSemanticAction(Value * list, const char * identifier) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	// list->identifierValue contiene "id1" o "id1,id2"
	// identifier es el nuevo ID a agregar
	// resultado: "id1,id2,id3"
	
	if (!list || !identifier) {
		return list;
	}
	
	size_t oldLen = list->identifierValue ? strlen(list->identifierValue) : 0;
	size_t newLen = strlen(identifier);
	size_t totalLen = oldLen + 1 + newLen + 1; // old + ',' + new + '\0'
	
	char * newList = (char *)malloc(totalLen);
	if (oldLen > 0) {
		strcpy(newList, list->identifierValue);
		strcat(newList, ",");
		strcat(newList, identifier);
		free(list->identifierValue);
	} else {
		strcpy(newList, identifier);
	}
	
	list->identifierValue = newList;
	free((char *)identifier); // Liberar el string del token
	
	return list;
}

/* PROPERTY ACTIONS */

Property * PropertySemanticAction(const char * key, Value * value) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	
	if (!_isValidProperty(key)) {
		logError(_logger, "Invalid property '%s' - not in the catalog of valid properties", key);
		_hasSemanticError = true;
		if (key) free((char *)key);
		destroyValue(value);
		return NULL;
	}
	
	Property * property = calloc(1, sizeof(Property));
	property->key = (char *)key;
	property->value = value;
	property->next = NULL;
	
	if (strcmp(key, "background") == 0) {
		property->type = PROP_BACKGROUND;
	} else if (strcmp(key, "text") == 0) {
		property->type = PROP_TEXT;
	} else if (strcmp(key, "font_size") == 0) {
		property->type = PROP_FONT_SIZE;
	} else if (strcmp(key, "color") == 0) {
		property->type = PROP_COLOR;
	} else if (strcmp(key, "on_press") == 0) {
		property->type = PROP_ON_PRESS;
	} else if (strcmp(key, "align") == 0) {
		property->type = PROP_ALIGN;
	} else if (strcmp(key, "width") == 0) {
		property->type = PROP_WIDTH;
	} else if (strcmp(key, "height") == 0) {
		property->type = PROP_HEIGHT;
	} else if (strcmp(key, "x") == 0) {
		property->type = PROP_X;
	} else if (strcmp(key, "y") == 0) {
		property->type = PROP_Y;
	} else if (strcmp(key, "on_select") == 0) {
		property->type = PROP_ON_SELECT;
	} else if (strcmp(key, "on_keypress") == 0) {
		property->type = PROP_ON_KEYPRESS;
	} else if (strcmp(key, "on_focus_gain") == 0) {
		property->type = PROP_ON_FOCUS_GAIN;
	} else if (strcmp(key, "on_focus_lost") == 0) {
		property->type = PROP_ON_FOCUS_LOST;
	}
	
	return property;
}

PropertyList * CreatePropertyListSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	PropertyList * list = calloc(1, sizeof(PropertyList));
	list->first = NULL;
	list->last = NULL;
	return list;
}

PropertyList * AddPropertySemanticAction(PropertyList * list, Property * property) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (list->first == NULL) {
		list->first = property;
		list->last = property;
	} else {
		list->last->next = property;
		list->last = property;
	}
	return list;
}

/* COMPONENT ACTIONS */

Component * ComponentSemanticAction(const char * id, PropertyList * properties, ComponentList * children) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	
	if (!_isUniqueId(id)) {
		logError(_logger, "Duplicate component ID '%s' - IDs must be unique within the program", id);
		_hasSemanticError = true;
		if (id) free((char *)id);
		destroyPropertyList(properties);
		destroyComponentList(children);
		return NULL;
	}
	
	Component * component = calloc(1, sizeof(Component));
	component->id = (char *)id;
	component->properties = properties;
	component->children = children;
	component->next = NULL;
	
	_registerComponentId(component->id);
	
	return component;
}

ComponentList * CreateComponentListSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ComponentList * list = calloc(1, sizeof(ComponentList));
	list->first = NULL;
	list->last = NULL;
	return list;
}

ComponentList * AddComponentSemanticAction(ComponentList * list, Component * component) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (list->first == NULL) {
		list->first = component;
		list->last = component;
	} else {
		list->last->next = component;
		list->last = component;
	}
	return list;
}

/* VARIABLE ACTIONS */

Variable * VariableSemanticAction(const char * name, Value * value) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Variable * variable = calloc(1, sizeof(Variable));
	variable->name = (char *)name;
	variable->value = value;
	variable->next = NULL;
	return variable;
}

VariableList * CreateVariableListSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	VariableList * list = calloc(1, sizeof(VariableList));
	list->first = NULL;
	list->last = NULL;
	return list;
}

VariableList * AddVariableSemanticAction(VariableList * list, Variable * variable) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (list->first == NULL) {
		list->first = variable;
		list->last = variable;
	} else {
		list->last->next = variable;
		list->last = variable;
	}
	return list;
}

/* PROGRAM ACTION */

Program * ProgramSemanticAction(VariableList * variables, ComponentList * components) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	
	if (_hasSemanticError) {
		logError(_logger, "Program rejected due to semantic errors");
		destroyVariableList(variables);
		destroyComponentList(components);
		_resetValidationState();
		return NULL;
	}
	
	
	Program * program = calloc(1, sizeof(Program));
	program->variables = variables;
	program->components = components;
	program->selectionOrder = NULL;
	program->selectionOrderCount = 0;
	program->identifier = NULL;
	
	// Extraer selection_order de las variables si existe
	if (variables && variables->first) {
		Variable * var = variables->first;
		Variable * prev = NULL;
		while (var) {
			if (var->name && strcmp(var->name, "selection_order") == 0) {
				// Encontramos selection_order, parsear la lista de IDs
				if (var->value && var->value->identifierValue) {
					// Contar cuántos IDs hay (separados por comas)
					int count = 1;
					for (char * p = var->value->identifierValue; *p; p++) {
						if (*p == ',') count++;
					}
					
					// Alocar array de strings
					program->selectionOrder = (char **)calloc(count, sizeof(char *));
					program->selectionOrderCount = count;
					
					// Parsear los IDs
					char * copy = strdup(var->value->identifierValue);
					char * token = strtok(copy, ",");
					int i = 0;
					while (token && i < count) {
						// Trim espacios
						while (*token == ' ') token++;
						program->selectionOrder[i++] = strdup(token);
						token = strtok(NULL, ",");
					}
					free(copy);
				}
				
				// Remover selection_order de la lista de variables
				if (prev) {
					prev->next = var->next;
				} else {
					variables->first = var->next;
				}
				if (variables->last == var) {
					variables->last = prev;
				}
				
				Variable * toDelete = var;
				var = var->next;
				destroyVariable(toDelete);
				continue;
			}
			prev = var;
			var = var->next;
		}
	}
	
	// Extraer identifier de las variables si existe
	if (variables && variables->first) {
		Variable * var = variables->first;
		Variable * prev = NULL;
		while (var) {
			if (var->name && strcmp(var->name, "identifier") == 0) {
				// Encontramos identifier, extraer el valor
				if (var->value && var->value->identifierValue) {
					program->identifier = strdup(var->value->identifierValue);
				}
				
				// Remover identifier de la lista de variables
				if (prev) {
					prev->next = var->next;
				} else {
					variables->first = var->next;
				}
				if (variables->last == var) {
					variables->last = prev;
				}
				
				Variable * toDelete = var;
				var = var->next;
				destroyVariable(toDelete);
				continue;
			}
			prev = var;
			var = var->next;
		}
	}
	
	Constant * dummyConstant = calloc(1, sizeof(Constant));
	dummyConstant->value = 0;
	
	Factor * dummyFactor = calloc(1, sizeof(Factor));
	dummyFactor->constant = dummyConstant;
	dummyFactor->type = CONSTANT;
	
	Expression * dummyExpression = calloc(1, sizeof(Expression));
	dummyExpression->factor = dummyFactor;
	dummyExpression->type = FACTOR;
	
	program->expression = dummyExpression;
	_compilerState->abstractSyntaxtTree = program;
	
	if (program->variables == NULL) {
		program->variables = CreateVariableListSemanticAction();
	}
	if (program->components == NULL) {
		program->components = CreateComponentListSemanticAction();
	}
	
	return program;
}
