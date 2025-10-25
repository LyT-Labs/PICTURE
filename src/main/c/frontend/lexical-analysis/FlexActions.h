#ifndef FLEX_ACTIONS_HEADER
#define FLEX_ACTIONS_HEADER

#include "../../support/configuration/Environment.h"
#include "../../support/language/String.h"
#include "../../support/logging/Logger.h"
#include "../../support/type/CompilationStatus.h"
#include "../../support/type/FlexContext.h"
#include "../../support/type/LexicalAnalyzer.h"
#include "../../support/type/ModuleDestructor.h"
#include "../../support/type/Token.h"
#include "../../support/type/TokenLabel.h"
#include "../Frontend.h"

/** Initialize module's internal state. */
ModuleDestructor initializeFlexActionsModule();

/**
 * PICTURE lexical action prototypes.
 */
CompilationStatus VariableDelimiterLexemeAction();
CompilationStatus ComponentIdLexemeAction();
CompilationStatus PropertyLexemeAction();
CompilationStatus ColonLexemeAction();
CompilationStatus StringLexemeAction();
CompilationStatus NumberLexemeAction();
CompilationStatus IdentifierLexemeAction();
CompilationStatus BuiltinLexemeAction();
CompilationStatus CommentLexemeAction();
CompilationStatus NewlineLexemeAction();
CompilationStatus IndentLexemeAction();
CompilationStatus DedentLexemeAction();
CompilationStatus EOFLexemeAction();
CompilationStatus IgnoredLexemeAction();
CompilationStatus UnknownLexemeAction();

#endif
