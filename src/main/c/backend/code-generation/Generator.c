#include "Generator.h"

#include <stdio.h>
#include <string.h>
#include "../../support/type/ModuleDestructor.h"

/*
 * La idea de este generador:
 *
 * Genera algo de este estilo:
 *
 *   #include "ui_runtime.h"
 *
 *   const char *textSize = "m";
 *   const char *color = "blue";
 *
 *   int main(void) {
 *       ui_init();
 *
 *       UIComponent main = ui_create_root("main");
 *       ui_set_background(main, "white");
 *
 *       UIComponent background = ui_create_child(main, "background");
 *       ui_set_background(background, color);
 *       ui_set_width(background, 100);
 *       ui_set_height(background, 100);
 *
 *       UIComponent title = ui_create_child(main, "title");
 *       ui_set_text(title, "Hola Mundo!");
 *       ui_set_font_size(title, textSize);
 *       ui_set_x(title, 20);
 *       ui_set_y(title, 20);
 *
 *       UIComponent button1 = ui_create_child(main, "button1");
 *       ui_set_text(button1, "Cambiar color");
 *       ui_set_x(button1, 20);
 *       ui_set_y(button1, 70);
 *       ui_set_on_press(button1, handle_press_color);
 *
 *       ui_run();
 *       return 0;
 *   }
 *
 * Vos después implementás ui_runtime.h / ui_runtime.c para hablar con tu SO.
 */

/* ------------------------------------------------------------------ */
/* Helpers de impresión                                               */
/* ------------------------------------------------------------------ */

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
 *  - IDENTIFIER   -> textSize   (asumimos que es algo que evalúa a int)
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
		 * Decisión simple: todas las variables del header se generan
		 * como const char*.
		 * value -> string con _emitValueAsCString.
		 */
		fprintf(out, "const char *%s = ", v->name);
		_emitValueAsCString(v->value, out);
		fputs(";\n", out);

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
		/* Propiedad desconocida: ignoramos pero dejamos comentario. */
		_indent(out, indentLevel);
		fprintf(out, "/* TODO: unsupported property '%s' on component '%s' */\n",
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
/* Entrada principal del generador                                    */
/* ------------------------------------------------------------------ */

void Generator_generate(const Program *program, CompilerState *state)
{
	if (program == NULL)
	{
		return;
	}

	FILE *out = _get_output_stream(state);

	/* Header */
	fputs("#include \"ui_runtime.h\"\n\n", out);

	/* Variables del header --- */
	_emitVariables(program->variables, out);

	/* main() */
	fputs("int main(void) {\n", out);
	_indent(out, 1);
	fputs("ui_init();\n\n", out);

	_emitComponents(program->components, out, 1);

	_indent(out, 1);
	fputs("ui_run();\n", out);
	_indent(out, 1);
	fputs("return 0;\n", out);
	fputs("}\n", out);
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
