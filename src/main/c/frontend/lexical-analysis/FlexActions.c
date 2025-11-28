#include "FlexActions.h"
#include "../syntactic-analysis/BisonParser.h"
#include <string.h>

/* MODULE INTERNAL STATE */

static bool _logIgnoredLexemes = true;
static InputBuffer * _inputBuffer = NULL;
static LexicalAnalyzer * _lexicalAnalyzer = NULL;
static Logger * _logger = NULL;

/* Indentation tracking for PICTURE */
static int * _indentStack = NULL;
static int _indentStackSize = 0;
static int _indentStackCapacity = 10;
static int _currentIndentLevel = 0;
static bool _atLineStart = true;

/** Shutdown module's internal state. */
void _shutdownFlexActionsModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: FlexActions...");
		destroyLogger(_logger);
		_logger = NULL;
	}
	if (_inputBuffer != NULL) {
		destroyInputBuffer(_inputBuffer);
		_inputBuffer = NULL;
	}
	if (_indentStack != NULL) {
		free(_indentStack);
		_indentStack = NULL;
	}
	_lexicalAnalyzer = NULL;
}

ModuleDestructor initializeFlexActionsModule(LexicalAnalyzer * lexicalAnalyzer) {
	_inputBuffer = NULL;
	_lexicalAnalyzer = lexicalAnalyzer;
	_logger = createLogger("FlexActions");
	_logIgnoredLexemes = getBooleanOrDefault("LOG_IGNORED_LEXEMES", _logIgnoredLexemes);
	
	_indentStack = (int *)calloc(_indentStackCapacity, sizeof(int));
	_indentStack[0] = 0;
	_indentStackSize = 1;
	_currentIndentLevel = 0;
	_atLineStart = true;
	
	return _shutdownFlexActionsModule;
}

/* PRIVATE FUNCTIONS */

static void _logTokenAction(const char * actionName, Token * token);

/**
 * Logs a lexical-analyzer action over a token in DEBUGGING level.
 */
static void _logTokenAction(const char * actionName, Token * token) {
	char * _lexeme = NULL;
	if (token->lexeme != NULL) {
		_lexeme = escape(token->lexeme);
	}
	logDebugging(_logger, WARNING_COLOR "%s" DEFAULT_COLOR ": Token(context=%d, label=%d, length=%d, lexeme=%s\"%s\"%s, line=%d, semanticValue=%p)",
		actionName,
		token->context,
		token->label,
		token->length,
		INFORMATION_COLOR, _lexeme ? _lexeme : "(null)", DEFAULT_COLOR,
		token->line,
		token->semanticValue);
	if (_lexeme != NULL) {
		free(_lexeme);
		_lexeme = NULL;
	}
}

/* PRIVATE FUNCTIONS FOR INDENTATION */

static CompilationStatus _pushIndent(int level) {
	if (_indentStackSize >= _indentStackCapacity) {
		_indentStackCapacity *= 2;
		_indentStack = (int *)realloc(_indentStack, _indentStackCapacity * sizeof(int));
	}
	_indentStack[_indentStackSize++] = level;
	return IN_PROGRESS;
}

static CompilationStatus _emitDedents(int targetLevel) {
	CompilationStatus status = IN_PROGRESS;
	while (_indentStackSize > 1 && _indentStack[_indentStackSize - 1] > targetLevel) {
		_indentStackSize--;
		Token * dedentToken = createToken(_lexicalAnalyzer, DEDENT);
		_logTokenAction("DEDENT", dedentToken);
		status = pushToken(_lexicalAnalyzer, dedentToken);
		destroyToken(dedentToken);
		if (status != IN_PROGRESS) break;
	}
	return status;
}

/* PUBLIC FUNCTIONS */

CompilationStatus VariableDelimiterLexemeAction() {
	Token * token = createToken(_lexicalAnalyzer, VARIABLE_DELIMITER);
	_logTokenAction(__FUNCTION__, token);
	CompilationStatus status = pushToken(_lexicalAnalyzer, token);
	destroyToken(token);
	_atLineStart = false;
	return status;
}

CompilationStatus SelectionOrderLexemeAction() {
	Token * token = createToken(_lexicalAnalyzer, PROPERTY);
	// Almacenar "selection_order" como el valor del token
	token->semanticValue->string = strdup("selection_order");
	_logTokenAction(__FUNCTION__, token);
	CompilationStatus status = pushToken(_lexicalAnalyzer, token);
	destroyToken(token);
	_atLineStart = false;
	return status;
}

CompilationStatus ComponentIdLexemeAction() {
	if (_atLineStart && _indentStackSize > 1) {
		CompilationStatus status = _emitDedents(0);
		if (status != IN_PROGRESS) {
			return status;
		}
	}
	
	Token * token = createToken(_lexicalAnalyzer, COMPONENT_ID);
	if (token->lexeme != NULL && token->semanticValue != NULL) {
		char * idStart = token->lexeme + 1;
		while (*idStart == ' ' || *idStart == '\t') {
			idStart++;
		}
		token->semanticValue->string = strdup(idStart);
	}
	_logTokenAction(__FUNCTION__, token);
	CompilationStatus status = pushToken(_lexicalAnalyzer, token);
	destroyToken(token);
	_atLineStart = false;
	return status;
}

CompilationStatus PropertyLexemeAction() {
	if (_atLineStart && _indentStackSize > 1) {
		CompilationStatus status = _emitDedents(0);
		if (status != IN_PROGRESS) {
			return status;
		}
	}
	
	Token * token = createToken(_lexicalAnalyzer, PROPERTY);
	if (token->lexeme != NULL && token->semanticValue != NULL) {
		char * propStart = token->lexeme + 1;
		while (*propStart == ' ' || *propStart == '\t') {
			propStart++;
		}
		token->semanticValue->string = strdup(propStart);
	}
	_logTokenAction(__FUNCTION__, token);
	CompilationStatus status = pushToken(_lexicalAnalyzer, token);
	destroyToken(token);
	_atLineStart = false;
	return status;
}

CompilationStatus ColonLexemeAction() {
	Token * token = createToken(_lexicalAnalyzer, COLON);
	_logTokenAction(__FUNCTION__, token);
	CompilationStatus status = pushToken(_lexicalAnalyzer, token);
	destroyToken(token);
	_atLineStart = false;
	return status;
}

CompilationStatus CommaLexemeAction() {
	Token * token = createToken(_lexicalAnalyzer, COMMA);
	_logTokenAction(__FUNCTION__, token);
	CompilationStatus status = pushToken(_lexicalAnalyzer, token);
	destroyToken(token);
	_atLineStart = false;
	return status;
}

CompilationStatus StringLexemeAction() {
	Token * token = createToken(_lexicalAnalyzer, STRING);
	if (token->semanticValue != NULL && token->lexeme != NULL && token->length >= 2) {
		char * unquoted = (char *)calloc(token->length - 1, sizeof(char));
		if (token->length > 2) {
			strncpy(unquoted, token->lexeme + 1, token->length - 2);
		}
		token->semanticValue->string = unquoted;
	}
	_logTokenAction(__FUNCTION__, token);
	CompilationStatus status = pushToken(_lexicalAnalyzer, token);
	destroyToken(token);
	_atLineStart = false;
	return status;
}

CompilationStatus NumberLexemeAction() {
	Token * token = createToken(_lexicalAnalyzer, NUMBER);
	if (token->semanticValue != NULL && token->lexeme != NULL) {
		token->semanticValue->integer = atoi(token->lexeme);
	}
	_logTokenAction(__FUNCTION__, token);
	CompilationStatus status = pushToken(_lexicalAnalyzer, token);
	destroyToken(token);
	_atLineStart = false;
	return status;
}

CompilationStatus IdentifierLexemeAction() {
	Token * token = createToken(_lexicalAnalyzer, IDENTIFIER);
	if (token->semanticValue != NULL && token->lexeme != NULL) {
		token->semanticValue->string = strdup(token->lexeme);
	}
	_logTokenAction(__FUNCTION__, token);
	CompilationStatus status = pushToken(_lexicalAnalyzer, token);
	destroyToken(token);
	_atLineStart = false;
	return status;
}

CompilationStatus BuiltinLexemeAction() {
	Token * token = createToken(_lexicalAnalyzer, BUILTIN);
	if (token->semanticValue != NULL && token->lexeme != NULL) {
		token->semanticValue->string = strdup(token->lexeme);
	}
	_logTokenAction(__FUNCTION__, token);
	CompilationStatus status = pushToken(_lexicalAnalyzer, token);
	destroyToken(token);
	_atLineStart = false;
	return status;
}

CompilationStatus CommentLexemeAction() {
	if (_logIgnoredLexemes) {
		Token * token = createToken(_lexicalAnalyzer, COMMENT);
		_logTokenAction(__FUNCTION__, token);
		destroyToken(token);
	}
	_atLineStart = false;
	return IN_PROGRESS;
}

CompilationStatus NewlineLexemeAction() {
	Token * token = createToken(_lexicalAnalyzer, NEWLINE);
	_logTokenAction(__FUNCTION__, token);
	CompilationStatus status = pushToken(_lexicalAnalyzer, token);
	destroyToken(token);
	_atLineStart = true;
	
	return status;
}

CompilationStatus IndentLexemeAction() {
	if (!_atLineStart) {
		return IgnoredLexemeAction();
	}
	
	Token * token = createToken(_lexicalAnalyzer, IGNORED);
	int indentLevel = 0;
	
	if (token->lexeme != NULL) {
		int i = 0;
		while (i < token->length) {
			if (token->lexeme[i] == '\t') {
				indentLevel++;
				i++;
			} else if (token->lexeme[i] == ' ') {
				if (i + 1 < token->length && token->lexeme[i + 1] == ' ') {
					indentLevel++;
					i += 2;
				} else {
					i++;
				}
			} else {
				i++;
			}
		}
	}
	
	destroyToken(token);
	
	CompilationStatus status = IN_PROGRESS;
	int currentLevel = _indentStack[_indentStackSize - 1];
	
	if (indentLevel > currentLevel) {
		_pushIndent(indentLevel);
		Token * indentToken = createToken(_lexicalAnalyzer, INDENT);
		_logTokenAction("INDENT", indentToken);
		status = pushToken(_lexicalAnalyzer, indentToken);
		destroyToken(indentToken);
	} else if (indentLevel < currentLevel) {
		status = _emitDedents(indentLevel);
		if (status == IN_PROGRESS && _indentStack[_indentStackSize - 1] != indentLevel) {
			logError(_logger, "Indentation error: mismatched indentation level");
			status = FAILED;
		}
	}
	
	_atLineStart = false;
	return status;
}

CompilationStatus DedentLexemeAction() {
	return _emitDedents(0);
}

CompilationStatus EOFLexemeAction() {
	CompilationStatus status = IN_PROGRESS;

	status = _emitDedents(0);
	if (status != IN_PROGRESS) {
		return status;
	}
	
	Token * token = createToken(_lexicalAnalyzer, 0);
	_logTokenAction(__FUNCTION__, token);
	if (!popInputBuffer(_lexicalAnalyzer)) {
		status = pushToken(_lexicalAnalyzer, token);
		FlexContext context = currentLexicalAnalyzerContext(_lexicalAnalyzer);
		if (0 < context) {
			logError(_logger, "The final context is not closed (context=%d).", context);
			status = FAILED;
		}
	}
	destroyToken(token);
	return status;
}

CompilationStatus IgnoredLexemeAction() {
	if (_logIgnoredLexemes) {
		Token * token = createToken(_lexicalAnalyzer, IGNORED);
		_logTokenAction(__FUNCTION__, token);
		destroyToken(token);
	}
	return IN_PROGRESS;
}

CompilationStatus UnknownLexemeAction() {
	Token * token = createToken(_lexicalAnalyzer, UNKNOWN);
	_logTokenAction(__FUNCTION__, token);
	destroyToken(token);
	return FAILED;
}
