#ifndef CALCULATOR_HEADER
#define CALCULATOR_HEADER

#include <stdbool.h>
#include "../../support/type/ModuleDestructor.h"
#include "../../support/type/CompilerState.h"


typedef struct {
    bool succeeded;
    int value;
} ComputationResult;

ModuleDestructor initializeCalculatorModule(void);


ComputationResult executeCalculator(CompilerState *compilerState);

#endif
