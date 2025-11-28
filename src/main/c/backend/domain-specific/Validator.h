#ifndef VALIDATOR_HEADER
#define VALIDATOR_HEADER

#include <stdbool.h>
#include "../../support/type/ModuleDestructor.h"
#include "../../support/type/CompilerState.h"


typedef struct {
    bool succeeded;
    int value;
} ComputationResult;

ModuleDestructor initializeValidatorModule(void);


ComputationResult executeValidator(CompilerState *compilerState);

#endif
