#include "Generator.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../../support/type/ModuleDestructor.h"

/*
 * Generador de código PICTURE para PinkOS
 *
 * Este generador produce código C estático que incluye:
 * - Headers del runtime de PinkOS
 * - Variables estáticas de colores builtin
 * - ComponentRegistry con punteros a cada componente por ID
 * - Array estático de componentes en orden BFS (breadth-first)
 * - Array de selection_order para navegación
 * - Función initialize_component_tree()
 * - Función <identifier>_main() con el main loop
 *
 * Los componentes se organizan en memoria de forma contígua por nivel,
 * permitiendo acceso eficiente a hermanos y padres.
 *
 * Las variables con valores builtin se expanden como macros en cada uso,
 * sin generar declaraciones intermedias.
 */

/* ------------------------------------------------------------------ */
/* Helpers de impresión                                               */
/* ------------------------------------------------------------------ */

// Variable estática para acceder a las variables globales durante generación
static const VariableList *_globalVariables = NULL;

static FILE *_get_output_stream(CompilerState *state)
{
	/*
	 * Por ahora devolvemos stdout.
	 * Si querés escribir a un archivo, podés usar información de CompilerState
	 * (por ejemplo un campo outputFilePath) y abrirlo acá.
	 */
	(void)state;
	return stdout;
}

static void _indent(FILE *out, int level)
{
	for (int i = 0; i < level; ++i)
	{
		fputs("    ", out);
	}
}

static void _emitStringLiteral(FILE *out, const char *s)
{
	fputc('"', out);
	if (s != NULL)
	{
		for (const char *p = s; *p != '\0'; ++p)
		{
			if (*p == '"' || *p == '\\')
			{
				fputc('\\', out);
			}
			fputc(*p, out);
		}
	}
	fputc('"', out);
}

/**
 * Imprime un Value como literal de C string:
 *  - NUMBER       -> "123"
 *  - STRING       -> "Hola"
 *  - IDENTIFIER   -> "textSize"
 *  - BUILTIN      -> "m", "blue", ...
 */
static void _emitValueAsCString(const Value *value, FILE *out)
{
	if (value == NULL)
	{
		fputs("\"\"", out);
		return;
	}

	char buffer[64];

	switch (value->type)
	{
	case VALUE_STRING:
		_emitStringLiteral(out, value->stringValue);
		break;
	case VALUE_NUMBER:
		snprintf(buffer, sizeof(buffer), "%d", value->numberValue);
		_emitStringLiteral(out, buffer);
		break;
	case VALUE_IDENTIFIER:
		_emitStringLiteral(out, value->identifierValue);
		break;
	case VALUE_BUILTIN:
		_emitStringLiteral(out, value->stringValue);
		break;
	default:
		fputs("\"\"", out);
		break;
	}
}

/**
 * Imprime un Value como expresión entera de C:
 *  - NUMBER       -> 123
 *  - IDENTIFIER   -> textSize
 *  Otros casos -> 0
 */
static void _emitValueAsIntExpr(const Value *value, FILE *out)
{
	if (value == NULL)
	{
		fputs("0", out);
		return;
	}

	switch (value->type)
	{
	case VALUE_NUMBER:
		fprintf(out, "%d", value->numberValue);
		break;
	case VALUE_IDENTIFIER:
		fprintf(out, "%s", value->identifierValue);
		break;
	default:
		fputs("0", out);
		break;
	}
}

/* ------------------------------------------------------------------ */
/* Emisión de variables (header ---)                                  */
/* ------------------------------------------------------------------ */

static void _emitVariables(const VariableList *vars, FILE *out)
{
	if (vars == NULL || vars->first == NULL)
	{
		return;
	}

	const Variable *v = vars->first;
	while (v != NULL)
	{
		/*
		 * Si el valor es builtin, NO emitimos la variable.
		 * Se expandirá como macro en cada uso.
		 */
		if (v->value && v->value->type != VALUE_BUILTIN)
		{
			// Solo emitir variables que NO son builtins
			fprintf(out, "const char *%s = ", v->name);
			_emitValueAsCString(v->value, out);
			fputs(";\n", out);
		}

		v = v->next;
	}

	fputc('\n', out);
}

/* ------------------------------------------------------------------ */
/* Emisión de propiedades de un componente                            */
/* ------------------------------------------------------------------ */

static void _emitPropertyForComponent(const Component *comp,
									  const Property *prop,
									  FILE *out,
									  int indentLevel)
{
	if (comp == NULL || prop == NULL)
		return;

	const char *id = comp->id;

	switch (prop->type)
	{
	case PROP_BACKGROUND:
		_indent(out, indentLevel);
		fprintf(out, "ui_set_background(%s, ", id);
		_emitValueAsCString(prop->value, out);
		fputs(");\n", out);
		break;

	case PROP_TEXT:
		_indent(out, indentLevel);
		fprintf(out, "ui_set_text(%s, ", id);
		_emitValueAsCString(prop->value, out);
		fputs(");\n", out);
		break;

	case PROP_FONT_SIZE:
		_indent(out, indentLevel);
		fprintf(out, "ui_set_font_size(%s, ", id);
		_emitValueAsCString(prop->value, out);
		fputs(");\n", out);
		break;

	case PROP_ALIGN:
		_indent(out, indentLevel);
		fprintf(out, "ui_set_align(%s, ", id);
		_emitValueAsCString(prop->value, out);
		fputs(");\n", out);
		break;

	case PROP_WIDTH:
		_indent(out, indentLevel);
		fprintf(out, "ui_set_width(%s, ", id);
		_emitValueAsIntExpr(prop->value, out);
		fputs(");\n", out);
		break;

	case PROP_HEIGHT:
		_indent(out, indentLevel);
		fprintf(out, "ui_set_height(%s, ", id);
		_emitValueAsIntExpr(prop->value, out);
		fputs(");\n", out);
		break;

	case PROP_X:
		_indent(out, indentLevel);
		fprintf(out, "ui_set_x(%s, ", id);
		_emitValueAsIntExpr(prop->value, out);
		fputs(");\n", out);
		break;

	case PROP_Y:
		_indent(out, indentLevel);
		fprintf(out, "ui_set_y(%s, ", id);
		_emitValueAsIntExpr(prop->value, out);
		fputs(");\n", out);
		break;

	case PROP_ON_PRESS:
		_indent(out, indentLevel);
		fprintf(out, "ui_set_on_press(%s, ", id);
		if (prop->value != NULL && prop->value->type == VALUE_IDENTIFIER)
		{
			fprintf(out, "%s", prop->value->identifierValue);
		}
		else
		{
			fputs("NULL", out);
		}
		fputs(");\n", out);
		break;

	case PROP_ON_SELECT:
		_indent(out, indentLevel);
		fprintf(out, "ui_set_on_select(%s, ", id);
		if (prop->value != NULL && prop->value->type == VALUE_IDENTIFIER)
		{
			fprintf(out, "%s", prop->value->identifierValue);
		}
		else
		{
			fputs("NULL", out);
		}
		fputs(");\n", out);
		break;

	case PROP_ON_KEYPRESS:
		_indent(out, indentLevel);
		fprintf(out, "ui_set_on_keypress(%s, ", id);
		if (prop->value != NULL && prop->value->type == VALUE_IDENTIFIER)
		{
			fprintf(out, "%s", prop->value->identifierValue);
		}
		else
		{
			fputs("NULL", out);
		}
		fputs(");\n", out);
		break;

	case PROP_ON_FOCUS_GAIN:
		_indent(out, indentLevel);
		fprintf(out, "ui_set_on_focus_gain(%s, ", id);
		if (prop->value != NULL && prop->value->type == VALUE_IDENTIFIER)
		{
			fprintf(out, "%s", prop->value->identifierValue);
		}
		else
		{
			fputs("NULL", out);
		}
		fputs(");\n", out);
		break;

	case PROP_ON_FOCUS_LOST:
		_indent(out, indentLevel);
		fprintf(out, "ui_set_on_focus_lost(%s, ", id);
		if (prop->value != NULL && prop->value->type == VALUE_IDENTIFIER)
		{
			fprintf(out, "%s", prop->value->identifierValue);
		}
		else
		{
			fputs("NULL", out);
		}
		fputs(");\n", out);
		break;

	default:
		/* Unknown property: skip but leave marker for debugging */
		_indent(out, indentLevel);
		fprintf(out, "/* Unsupported property '%s' on component '%s' */\n",
				prop->key ? prop->key : "(null)",
				id ? id : "(null)");
		break;
	}
}

/* ------------------------------------------------------------------ */
/* Emisión recursiva de componentes                                   */
/* ------------------------------------------------------------------ */

static void _emitComponentRecursive(const Component *comp,
									const char *parentId,
									FILE *out,
									int indentLevel)
{
	if (comp == NULL || comp->id == NULL)
		return;

	const char *id = comp->id;

	/* Crear componente: root o hijo */
	_indent(out, indentLevel);
	if (parentId == NULL)
	{
		fprintf(out,
				"UIComponent %s = ui_create_root(\"%s\");\n",
				id, id);
	}
	else
	{
		fprintf(out,
				"UIComponent %s = ui_create_child(%s, \"%s\");\n",
				id, parentId, id);
	}

	/* Propiedades del componente */
	if (comp->properties != NULL)
	{
		Property *p = comp->properties->first;
		while (p != NULL)
		{
			_emitPropertyForComponent(comp, p, out, indentLevel);
			p = p->next;
		}
	}

	/* Componentes hijos (nested) */
	if (comp->children != NULL)
	{
		Component *child = comp->children->first;
		while (child != NULL)
		{
			_emitComponentRecursive(child, id, out, indentLevel + 1);
			child = child->next;
		}
	}
}

/* ------------------------------------------------------------------ */
/* Emisión de todos los componentes top-level                         */
/* ------------------------------------------------------------------ */

static void _emitComponents(const ComponentList *clist, FILE *out, int indentLevel)
{
	if (clist == NULL)
		return;

	Component *c = clist->first;
	while (c != NULL)
	{
		_emitComponentRecursive(c, NULL, out, indentLevel);
		fputc('\n', out);
		c = c->next;
	}
}

/* ------------------------------------------------------------------ */
/* Funciones auxiliares para el nuevo formato con arrays estáticos   */
/* ------------------------------------------------------------------ */

/**
 * Cuenta la cantidad total de componentes en el árbol
 */
static int _countComponents(const Component *component)
{
	if (!component)
		return 0;

	int count = 1; // Este componente
	
	// Contar hijos recursivamente
	if (component->children && component->children->first)
	{
		Component *child = component->children->first;
		while (child)
		{
			count += _countComponents(child);
			child = child->next;
		}
	}
	
	return count;
}

/**
 * Cuenta cuántos hijos directos tiene un componente
 */
static int _countDirectChildren(const Component *component)
{
	if (!component || !component->children)
		return 0;

	int count = 0;
	Component *child = component->children->first;
	while (child)
	{
		count++;
		child = child->next;
	}
	return count;
}

/**
 * Emite los campos del struct ComponentRegistry de forma recursiva
 */
static void _emitRegistryFields(const Component *component, FILE *out)
{
	if (!component)
		return;

	fprintf(out, "    Component* %s;\n", component->id);

	// Recursión en hijos
	if (component->children && component->children->first)
	{
		Component *child = component->children->first;
		while (child)
		{
			_emitRegistryFields(child, out);
			child = child->next;
		}
	}
}

/**
 * Emite el struct ComponentRegistry con punteros a todos los componentes
 */
static void _emitComponentRegistry(const ComponentList *clist, FILE *out)
{
	fputs("typedef struct {\n", out);

	// Emitir campo para cada componente
	Component *comp = clist->first;
	while (comp)
	{
		_emitRegistryFields(comp, out);
		comp = comp->next;
	}

	fputs("} ComponentRegistry;\n\n", out);
}

/**
 * Asigna índices a componentes recursivamente en pre-order
 */
static int _assignIndices(Component *component, int startIndex)
{
	if (!component)
		return startIndex;
		
	int currentIndex = startIndex;
	
	// Este componente toma el índice actual
	// (guardamos el índice en un campo temporal - pero no tenemos uno,
	// así que vamos a emitir directamente en la segunda pasada)
	currentIndex++;
	
	// Los hijos toman los siguientes índices
	if (component->children && component->children->first)
	{
		Component *child = component->children->first;
		while (child)
		{
			currentIndex = _assignIndices(child, currentIndex);
			child = child->next;
		}
	}
	
	return currentIndex;
}

/**
 * Emite el valor de una propiedad como parámetro del COMPONENT_CONSTRUCTOR
 */
static void _emitPropertyValue(const Value *value, FILE *out)
{
	if (!value)
	{
		fputs("NULL", out);
		return;
	}

	switch (value->type)
	{
	case VALUE_STRING:
		_emitStringLiteral(out, value->stringValue);
		break;
	case VALUE_NUMBER:
		fprintf(out, "%d", value->numberValue);
		break;
	case VALUE_IDENTIFIER:
		// Buscar si el identifier es una variable global con valor builtin
		if (_globalVariables)
		{
			const Variable *var = _globalVariables->first;
			while (var)
			{
				if (strcmp(var->name, value->identifierValue) == 0)
				{
					// Encontrada: si es builtin, expandir directamente
					if (var->value && var->value->type == VALUE_BUILTIN)
					{
						_emitPropertyValue(var->value, out);
						return;
					}
					break;
				}
				var = var->next;
			}
		}
		// Si no es variable builtin, es callback o variable normal
		fprintf(out, "%s", value->identifierValue);
		break;
	case VALUE_BUILTIN:
		// Tamaños de texto: s=1, m=2, l=3
		if (strcmp(value->identifierValue, "s") == 0)
		{
			fprintf(out, "1");
		}
		else if (strcmp(value->identifierValue, "m") == 0)
		{
			fprintf(out, "2");
		}
		else if (strcmp(value->identifierValue, "l") == 0)
		{
			fprintf(out, "3");
		}
		// Colores: punteros a variables estáticas
		else if (strcmp(value->identifierValue, "red") == 0)
		{
			fprintf(out, "&color_red");
		}
		else if (strcmp(value->identifierValue, "blue") == 0)
		{
			fprintf(out, "&color_blue");
		}
		else if (strcmp(value->identifierValue, "green") == 0)
		{
			fprintf(out, "&color_green");
		}
		else if (strcmp(value->identifierValue, "white") == 0)
		{
			fprintf(out, "&color_white");
		}
		else if (strcmp(value->identifierValue, "black") == 0)
		{
			fprintf(out, "&color_black");
		}
		// Alineamientos: enums
		else if (strcmp(value->identifierValue, "left") == 0)
		{
			fprintf(out, "ALIGN_LEFT");
		}
		else if (strcmp(value->identifierValue, "center") == 0)
		{
			fprintf(out, "ALIGN_CENTER");
		}
		else if (strcmp(value->identifierValue, "right") == 0)
		{
			fprintf(out, "ALIGN_RIGHT");
		}
		else
		{
			// Fallback: NULL
			fputs("NULL", out);
		}
		break;
	default:
		fputs("NULL", out);
		break;
	}
}

/**
 * Emite la declaración de un componente en el array
 */
static void _emitSingleComponent(const Component *component, int myIndex, int childrenStartIndex, FILE *out)
{
	if (!component)
		return;

	// Comentario con el ID del componente
	fprintf(out, "    // %s\n", component->id);
	fprintf(out, "    component_array[%d] = COMPONENT_CONSTRUCTOR(\n", myIndex);

	// Emitir propiedades del componente desde el AST
	if (component->properties && component->properties->first)
	{
		Property *prop = component->properties->first;
		while (prop)
		{
			switch (prop->type)
			{
			case PROP_BACKGROUND:
				fputs("        .bg_color = ", out);
				_emitPropertyValue(prop->value, out);
				fputs(",\n", out);
				break;
			case PROP_TEXT:
				fputs("        .text = ", out);
				_emitPropertyValue(prop->value, out);
				fputs(",\n", out);
				break;
			case PROP_FONT_SIZE:
				fputs("        .text_size = ", out);
				_emitPropertyValue(prop->value, out);
				fputs(",\n", out);
				break;
			case PROP_COLOR:
				fputs("        .text_color = ", out);
				_emitPropertyValue(prop->value, out);
				fputs(",\n", out);
				break;
			case PROP_ALIGN:
				fputs("        .alignment = ", out);
				_emitPropertyValue(prop->value, out);
				fputs(",\n", out);
				break;
			case PROP_WIDTH:
				fputs("        .width = ", out);
				_emitPropertyValue(prop->value, out);
				fputs(",\n", out);
				break;
			case PROP_HEIGHT:
				fputs("        .height = ", out);
				_emitPropertyValue(prop->value, out);
				fputs(",\n", out);
				break;
			case PROP_X:
				fputs("        .x_position = ", out);
				_emitPropertyValue(prop->value, out);
				fputs(",\n", out);
				break;
			case PROP_Y:
				fputs("        .y_position = ", out);
				_emitPropertyValue(prop->value, out);
				fputs(",\n", out);
				break;
			case PROP_Y_POSITION:
				fputs("        .y_position = ", out);
				_emitPropertyValue(prop->value, out);
				fputs(",\n", out);
				break;
			case PROP_BORDER_SIZE:
				fputs("        .border_size = ", out);
				_emitPropertyValue(prop->value, out);
				fputs(",\n", out);
				break;
			case PROP_BORDER_COLOR:
				fputs("        .border_color = ", out);
				_emitPropertyValue(prop->value, out);
				fputs(",\n", out);
				break;
			case PROP_ACTIVE:
				fputs("        .active = ", out);
				_emitPropertyValue(prop->value, out);
				fputs(",\n", out);
				break;
			case PROP_ON_PRESS:
				fputs("        .on_press = ", out);
				_emitPropertyValue(prop->value, out);
				fputs(",\n", out);
				break;
			case PROP_ON_SELECT:
				fputs("        .on_select = ", out);
				_emitPropertyValue(prop->value, out);
				fputs(",\n", out);
				break;
			case PROP_ON_KEYPRESS:
				fputs("        .on_key_press = ", out);
				_emitPropertyValue(prop->value, out);
				fputs(",\n", out);
				break;
			case PROP_ON_FOCUS_GAIN:
				fputs("        .on_focus_gain = ", out);
				_emitPropertyValue(prop->value, out);
				fputs(",\n", out);
				break;
			case PROP_ON_FOCUS_LOST:
				fputs("        .on_focus_lost = ", out);
				_emitPropertyValue(prop->value, out);
				fputs(",\n", out);
				break;
			default:
				break;
			}
			prop = prop->next;
		}
	}

	// Indicar cantidad de hijos si los tiene
	int childCount = _countDirectChildren(component);
	if (childCount > 0)
	{
		fprintf(out, "        .children_count = %d,\n", childCount);
		fprintf(out, "        .children = &component_array[%d],\n", childrenStartIndex);
	}

	fputs("    );\n", out);
	fprintf(out, "    components.%s = &component_array[%d];\n\n", component->id, myIndex);
}

/**
 * Cola simple para BFS
 */
typedef struct QueueNode {
	const Component *component;
	int index;
	int childrenStartIndex;
	struct QueueNode *next;
} QueueNode;

typedef struct {
	QueueNode *head;
	QueueNode *tail;
} Queue;

static void _queueInit(Queue *q) {
	q->head = NULL;
	q->tail = NULL;
}

static void _queuePush(Queue *q, const Component *comp, int index, int childrenStartIndex) {
	QueueNode *node = (QueueNode*)malloc(sizeof(QueueNode));
	node->component = comp;
	node->index = index;
	node->childrenStartIndex = childrenStartIndex;
	node->next = NULL;
	
	if (q->tail) {
		q->tail->next = node;
	} else {
		q->head = node;
	}
	q->tail = node;
}

static QueueNode* _queuePop(Queue *q) {
	if (!q->head)
		return NULL;
	
	QueueNode *node = q->head;
	q->head = node->next;
	if (!q->head)
		q->tail = NULL;
	
	return node;
}

static bool _queueIsEmpty(Queue *q) {
	return q->head == NULL;
}

/**
 * Asigna índices a componentes en breadth-first order
 * Retorna el número total de componentes procesados
 */
static int _assignComponentIndices(const Component *root, Queue *emitQueue)
{
	if (!root)
		return 0;
	
	Queue bfsQueue;
	_queueInit(&bfsQueue);
	
	int nextIndex = 0;
	
	// Encolar el root
	_queuePush(&bfsQueue, root, nextIndex++, -1);
	
	// BFS
	while (!_queueIsEmpty(&bfsQueue))
	{
		QueueNode *current = _queuePop(&bfsQueue);
		const Component *comp = current->component;
		int myIndex = current->index;
		int myChildrenStart = nextIndex;
		
		// Encolar todos los hijos del nivel actual
		if (comp->children && comp->children->first)
		{
			Component *child = comp->children->first;
			while (child)
			{
				_queuePush(&bfsQueue, child, nextIndex++, -1);
				child = child->next;
			}
		}
		
		// Guardar en la cola de emisión con el índice correcto de children
		_queuePush(emitQueue, comp, myIndex, myChildrenStart);
		
		free(current);
	}
	
	return nextIndex;
}

/**
 * Emite componentes en el orden determinado por la cola
 */
static void _emitComponentsFromQueue(Queue *emitQueue, FILE *out)
{
	while (!_queueIsEmpty(emitQueue))
	{
		QueueNode *node = _queuePop(emitQueue);
		_emitSingleComponent(node->component, node->index, node->childrenStartIndex, out);
		free(node);
	}
}

/**
 * Verifica si un componente tiene propiedades selectables
 */
static bool _isSelectable(const Component *component)
{
	if (!component || !component->properties)
		return false;
		
	Property *prop = component->properties->first;
	while (prop)
	{
		if (prop->type == PROP_ON_PRESS ||
		    prop->type == PROP_ON_SELECT ||
		    prop->type == PROP_ON_KEYPRESS ||
		    prop->type == PROP_ON_FOCUS_GAIN ||
		    prop->type == PROP_ON_FOCUS_LOST)
		{
			return true;
		}
		prop = prop->next;
	}
	return false;
}

/**
 * Cuenta componentes selectables recursivamente
 */
static int _countSelectableComponents(const Component *component)
{
	if (!component)
		return 0;
		
	int count = _isSelectable(component) ? 1 : 0;
	
	if (component->children && component->children->first)
	{
		Component *child = component->children->first;
		while (child)
		{
			count += _countSelectableComponents(child);
			child = child->next;
		}
	}
	
	return count;
}

/**
 * Emite el array component_selection_order (siempre, incluso si está vacío)
 */
static void _emitSelectionOrderArray(const Program *program, int selectableCount, FILE *out)
{
	// Determinar tamaño: si hay selection_order usar ese count, sino 0
	int arraySize = (program->selectionOrder && program->selectionOrderCount > 0) 
	                ? program->selectionOrderCount 
	                : 0;
	
	// Si el tamaño es 0, declarar array con un elemento para cumplir requisitos de C
	if (arraySize == 0)
	{
		fputs("static Component * component_selection_order[1]; // Empty, required for compilation\n\n", out);
	}
	else
	{
		fprintf(out, "static Component * component_selection_order[%d];\n\n", arraySize);
	}
}

/**
 * Emite la inicialización del selection order y gui_context dentro de initialize_component_tree
 */
static void _emitSelectionOrderInit(const Program *program, const Component *rootComponent, FILE *out)
{
	// Si hay selection_order, emitir la inicialización del array
	if (program->selectionOrder && program->selectionOrderCount > 0)
	{
		fputs("    // Indicar el orden de selección\n", out);
		for (int i = 0; i < program->selectionOrderCount; i++)
		{
			fprintf(out, "    component_selection_order[%d] = components.%s;\n", 
			        i, program->selectionOrder[i]);
		}
		fprintf(out, "    int selectable_count = %d;\n\n", program->selectionOrderCount);
	}
	else
	{
		// Sin selection_order, contar componentes selectables
		fputs("    int selectable_count = 0;\n\n", out);
	}
	
	// SIEMPRE inicializar el contexto de GUI
	fputs("    // Inicializar contexto de GUI\n", out);
	if (rootComponent && rootComponent->id)
	{
		fprintf(out, "    gui_context = initialize_gui_context(components.%s, component_selection_order, selectable_count);\n", 
		        rootComponent->id);
	}
	else
	{
		fputs("    gui_context = initialize_gui_context(&component_array[0], component_selection_order, selectable_count);\n", out);
	}
}

/* ------------------------------------------------------------------ */
/* Entrada principal del generador                                    */
/* ------------------------------------------------------------------ */

void Generator_generate(const Program *program, CompilerState *state)
{
	if (program == NULL)
	{
		return;
	}

	// Inicializar variable estática para expansión de macros builtin
	_globalVariables = program->variables;

	FILE *out = _get_output_stream(state);

	/* Headers */
	fputs("#include <libs/PictureRuntimeLib.h>\n", out);
	fputs("#include <syscalls/syscallCodes.h>\n", out);
	fputs("#include <libs/events.h>\n", out);
	fputs("#include <colors.h>\n", out);
	fputs("#include <themes.h>\n", out);
	
	/* Include específico del identifier si existe */
	if (program->identifier) {
		fprintf(out, "#include <%s.h>\n", program->identifier);
	}
	fputc('\n', out);

	/* Forward declarations */
	fputs("extern uint64_t syscall(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3);\n\n", out);

	/* Variables estáticas para colores literales */
	fputs("// Colores builtin\n", out);
	fputs("static uint32_t color_red = 0xFF0000;\n", out);
	fputs("static uint32_t color_blue = 0x0000FF;\n", out);
	fputs("static uint32_t color_green = 0x00FF00;\n", out);
	fputs("static uint32_t color_white = 0xFFFFFF;\n", out);
	fputs("static uint32_t color_black = 0x000000;\n\n", out);

	/* Emitir struct ComponentRegistry */
	if (program->components && program->components->first)
	{
		_emitComponentRegistry(program->components, out);
	}

	/* Contar componentes totales para dimensionar el array */
	int totalComponents = 0;
	if (program->components)
	{
		Component *comp = program->components->first;
		while (comp)
		{
			totalComponents += _countComponents(comp);
			comp = comp->next;
		}
	}

	/* Arrays estáticos */
	fprintf(out, "static Component component_array[%d];\n", totalComponents);
	fputs("static ComponentRegistry components;\n", out);
	fputs("static GuiContext gui_context;\n", out);
	
	/* Array de selection order (siempre presente, incluso si vacío) */
	int selectableCount = 0;
	if (program->selectionOrder && program->selectionOrderCount > 0)
	{
		selectableCount = program->selectionOrderCount;
	}
	_emitSelectionOrderArray(program, selectableCount, out);
	fputc('\n', out);

	/* Variables globales del header --- */
	_emitVariables(program->variables, out);

	/* Función de inicialización del árbol de componentes */
	fputs("static void initialize_component_tree() {\n", out);
	
	Component *rootComponent = NULL;
	if (program->components && program->components->first)
	{
		rootComponent = program->components->first; // El primer componente es el root
		
		Queue emitQueue;
		_queueInit(&emitQueue);
		
		Component *comp = program->components->first;
		while (comp)
		{
			_assignComponentIndices(comp, &emitQueue);
			comp = comp->next;
		}
		
		_emitComponentsFromQueue(&emitQueue, out);
	}
	
	/* Inicialización del selection order */
	_emitSelectionOrderInit(program, rootComponent, out);
	
	fputs("}\n\n", out);

	/* Emitir función main usando el identifier */
	if (program->identifier) {
		fprintf(out, "static void %s_main() {\n", program->identifier);
		fputs("    enableDoubleBuffering(); // Habilitar double buffering para evitar flickering\n\n", out);
		fputs("    initialize_component_tree();\n", out);
		fputs("  \n", out);
		fprintf(out, "   %s_main_loop(&gui_context, &components, &component_array);\n\n", program->identifier);
		fputs("    disableDoubleBuffering(); // Deshabilitar double buffering al salir porque la shell no lo usa\n", out);
		fputs("}\n", out);
	} else {
		fputs("// No identifier specified, skipping main function generation\n", out);
	}
}

/* ------------------------------------------------------------------ */
/* Inicialización y ejecución del módulo                              */
/* ------------------------------------------------------------------ */

static void _shutdownGeneratorModule(void)
{
	// No cleanup needed for now
}

ModuleDestructor initializeGeneratorModule(void)
{
	return _shutdownGeneratorModule;
}

void executeGenerator(CompilerState *state)
{
	if (state && state->abstractSyntaxtTree)
	{
		Generator_generate(state->abstractSyntaxtTree, state);
	}
}
