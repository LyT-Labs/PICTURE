#include "Calculator.h"

#include <string.h>

#include "../../support/logging/Logger.h"
#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../code-generation/Generator.h"


static Logger *_logger = NULL;

static void _shutdownCalculatorModule(void) {
    if (_logger != NULL) {
        logDebugging(_logger, "Destroying module: Calculator...");
        destroyLogger(_logger);
        _logger = NULL;
    }
}

ModuleDestructor initializeCalculatorModule(void) {
    _logger = createLogger("Calculator");
    return _shutdownCalculatorModule;
}


static Variable *_lookupVariable(const VariableList *vars, const char *name) {
    if (vars == NULL || name == NULL) {
        return NULL;
    }

    Variable *v = vars->first;
    while (v != NULL) {
        if (v->name != NULL && strcmp(v->name, name) == 0) {
            return v;
        }
        v = v->next;
    }
    return NULL;
}


static bool _validateValue(const Value *value, const VariableList *vars) {
    (void)vars;

    if (value == NULL) return false;

    switch (value->type) {
        case VALUE_STRING:
            return value->stringValue != NULL;

        case VALUE_NUMBER:
            return true;

        case VALUE_IDENTIFIER:
            return value->identifierValue != NULL;

        case VALUE_BUILTIN:
            // m, l, s, red, blue, center, etc. Bison ya los conoce.
            return true;

        default:
            return false;
    }
}

static bool _validateProperty(const Property *prop, const VariableList *vars) {
    if (prop == NULL) return false;
    if (prop->value == NULL) return false;

    if (!_validateValue(prop->value, vars)) {
        return false;
    }

    switch (prop->type) {
        case PROP_BACKGROUND:
        case PROP_TEXT:
        case PROP_FONT_SIZE:
        case PROP_COLOR:
        case PROP_ON_PRESS:
        case PROP_ALIGN:
        case PROP_ON_SELECT:
        case PROP_ON_KEYPRESS:
        case PROP_ON_FOCUS_GAIN:
        case PROP_ON_FOCUS_LOST:
            return true;

        case PROP_WIDTH:
        case PROP_HEIGHT:
        case PROP_X:
        case PROP_Y:
            return (prop->value->type == VALUE_NUMBER ||
                    prop->value->type == VALUE_IDENTIFIER);

        default:
            return false;
    }
}

static bool _validatePropertyList(const PropertyList *plist, const VariableList *vars) {
    if (plist == NULL) return true; // sin propiedades == OK

    Property *p = plist->first;
    while (p != NULL) {
        if (!_validateProperty(p, vars)) {
            logError(_logger, "Invalid property '%s' in component", p->key ? p->key : "(null)");
            return false;
        }
        p = p->next;
    }

    return true;
}

/**
 * Validar un componente y sus hijos.
 */
static bool _validateComponent(const Component *comp, const VariableList *vars) {
    if (comp == NULL) return true;

    if (comp->id == NULL) {
        logError(_logger, "Component without id detected");
        return false;
    }

    // Validar sus propiedades
    if (!_validatePropertyList(comp->properties, vars)) {
        logError(_logger, "Invalid property list in component '%s'", comp->id);
        return false;
    }

    // Validar hijos
    if (comp->children != NULL) {
        Component *child = comp->children->first;
        while (child != NULL) {
            if (!_validateComponent(child, vars)) {
                return false;
            }
            child = child->next;
        }
    }

    return true;
}


static bool _validateComponentList(const ComponentList *clist, const VariableList *vars) {
    if (clist == NULL) return true;

    Component *c = clist->first;
    while (c != NULL) {
        if (!_validateComponent(c, vars)) {
            return false;
        }
        c = c->next;
    }

    return true;
}

/**
 * Validar el programa completo: variables del header, componentes y propiedades
 */
static bool _validateProgram(const Program *program) {
    if (program == NULL) {
        logError(_logger, "AST Program is NULL");
        return false;
    }

    VariableList *vars = program->variables;

    if (vars != NULL) {
        Variable *outer = vars->first;
        while (outer != NULL) {
            Variable *inner = outer->next;
            while (inner != NULL) {
                if (outer->name != NULL && inner->name != NULL &&
                    strcmp(outer->name, inner->name) == 0) {
                    logError(_logger, "Duplicate variable '%s' in header", outer->name);
                    return false;
                }
                inner = inner->next;
            }
            outer = outer->next;
        }
    }

    if (!_validateComponentList(program->components, vars)) {
        return false;
    }

    return true;
}


ComputationResult executeCalculator(CompilerState *compilerState) {
    ComputationResult result = {
        .succeeded = false,
        .value = 0
    };

    if (compilerState == NULL) {
        logError(_logger, "executeCalculator: compilerState is NULL");
        return result;
    }

    Program *program = compilerState->abstractSyntaxtTree;
    if (program == NULL) {
        logError(_logger, "executeCalculator: AST is NULL");
        return result;
    }

    logDebugging(_logger, "Validating PICTURE AST...");
    if (!_validateProgram(program)) {
        logError(_logger, "executeCalculator: semantic validation failed");
        return result;
    }

    logDebugging(_logger, "Calculator validation completed successfully.");

    result.succeeded = true;
    return result;
}
