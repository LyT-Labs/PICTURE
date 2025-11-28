#ifndef GENERATOR_HEADER
#define GENERATOR_HEADER

#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../../support/type/CompilerState.h"
#include "../../support/type/ModuleDestructor.h"

/**
 * Genera código C a partir del AST del programa PICTURE.
 *
 * Por ahora escribe el código a stdout
 */
void Generator_generate(const Program *program, CompilerState *state);

// Inicializa el módulo del generador.
ModuleDestructor initializeGeneratorModule(void);

//Ejecuta el generador con el estado del compilador.
void executeGenerator(CompilerState *state);

#endif
