#include "AbstractSyntaxTree.h"
#include <string.h>

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

/** Shutdown module's internal state. */
void _shutdownAbstractSyntaxTreeModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: AbstractSyntaxTree...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeAbstractSyntaxTreeModule() {
	_logger = createLogger("AbstractSyntaxTree");
	return _shutdownAbstractSyntaxTreeModule;
}

/* PUBLIC FUNCTIONS */

void destroyValue(Value * value) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (value != NULL) {
		switch (value->type) {
			case VALUE_STRING:
				if (value->stringValue != NULL) {
					free(value->stringValue);
				}
				break;
			case VALUE_IDENTIFIER:
			case VALUE_BUILTIN:
				if (value->identifierValue != NULL) {
					free(value->identifierValue);
				}
				break;
			case VALUE_NUMBER:
				// No additional cleanup needed
				break;
		}
		free(value);
	}
}

void destroyProperty(Property * property) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (property != NULL) {
		if (property->key != NULL) {
			free(property->key);
		}
		destroyValue(property->value);
		free(property);
	}
}

void destroyPropertyList(PropertyList * propertyList) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (propertyList != NULL) {
		Property * current = propertyList->first;
		while (current != NULL) {
			Property * next = current->next;
			destroyProperty(current);
			current = next;
		}
		free(propertyList);
	}
}

void destroyComponent(Component * component) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (component != NULL) {
		if (component->id != NULL) {
			free(component->id);
		}
		destroyPropertyList(component->properties);
		destroyComponentList(component->children);
		free(component);
	}
}

void destroyComponentList(ComponentList * componentList) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (componentList != NULL) {
		Component * current = componentList->first;
		while (current != NULL) {
			Component * next = current->next;
			destroyComponent(current);
			current = next;
		}
		free(componentList);
	}
}

void destroyVariable(Variable * variable) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (variable != NULL) {
		if (variable->name != NULL) {
			free(variable->name);
		}
		destroyValue(variable->value);
		free(variable);
	}
}

void destroyVariableList(VariableList * variableList) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (variableList != NULL) {
		Variable * current = variableList->first;
		while (current != NULL) {
			Variable * next = current->next;
			destroyVariable(current);
			current = next;
		}
		free(variableList);
	}
}

void destroyProgram(Program * program) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (program != NULL) {
		destroyVariableList(program->variables);
		destroyComponentList(program->components);
		
		// Liberar selectionOrder
		if (program->selectionOrder) {
			for (int i = 0; i < program->selectionOrderCount; i++) {
				if (program->selectionOrder[i]) {
					free(program->selectionOrder[i]);
				}
			}
			free(program->selectionOrder);
		}
		
		// Liberar identifier
		if (program->identifier) {
			free(program->identifier);
		}
		
		destroyExpression(program->expression); // Legacy field
		free(program);
	}
}

/**
 * Legacy destructors for backward compatibility.
 */
void destroyConstant(Constant * constant) {
	logDebugging(_logger, "Executing legacy destructor: %s", __FUNCTION__);
	if (constant != NULL) {
		free(constant);
	}
}

void destroyExpression(Expression * expression) {
	logDebugging(_logger, "Executing legacy destructor: %s", __FUNCTION__);
	if (expression != NULL) {
		switch (expression->type) {
			case ADDITION:
			case DIVISION:
			case MULTIPLICATION:
			case SUBTRACTION:
				destroyExpression(expression->leftExpression);
				destroyExpression(expression->rightExpression);
				break;
			case FACTOR:
				destroyFactor(expression->factor);
				break;
		}
		free(expression);
	}
}

void destroyFactor(Factor * factor) {
	logDebugging(_logger, "Executing legacy destructor: %s", __FUNCTION__);
	if (factor != NULL) {
		switch (factor->type) {
			case CONSTANT:
				destroyConstant(factor->constant);
				break;
			case EXPRESSION:
				destroyExpression(factor->expression);
				break;
		}
		free(factor);
	}
}
