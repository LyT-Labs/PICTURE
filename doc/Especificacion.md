# Especificación del Lenguaje - Calculadora

## Descripción General

Este proyecto implementa un analizador léxico y sintáctico para un lenguaje de expresiones aritméticas, similar a una calculadora. Utiliza Flex para el análisis léxico y Bison para el análisis sintáctico. Permite evaluar expresiones aritméticas con soporte para operaciones básicas, paréntesis, comentarios multilínea y bloques de subexpresiones entre llaves.

## Sintaxis Soportada

### Expresiones Aritméticas
- Suma: `1 + 2`
- Resta: `4 - 3`
- Multiplicación: `2 * 5`
- División: `8 / 2`
- Paréntesis para agrupación: `(1 + 2) * 3`
- Constantes enteras: `42`

### Subexpresiones entre llaves

Las llaves `{ ... }` permiten definir bloques de subexpresiones o "importaciones" dentro del código fuente. Todo lo que se encuentre entre llaves es reconocido por el analizador léxico, pero **no es evaluado ni considerado parte de la expresión aritmética principal**. Su función principal es permitir incluir fragmentos de código, datos auxiliares, o anotaciones que pueden ser procesadas por etapas posteriores del compilador (por ejemplo, para importar datos, incluir metadatos, o realizar análisis adicionales), pero no afectan el resultado del cálculo.

**Importante:**
- El contenido entre llaves puede ser cualquier secuencia de caracteres, excepto otras llaves anidadas (no se permite anidación de bloques `{ ... { ... } ... }`).
- Si una llave de apertura `{` no tiene su correspondiente llave de cierre `}`, se reporta un error de sintaxis.
- El contenido de las llaves es ignorado por el evaluador aritmético, pero puede ser registrado o procesado por el compilador según la implementación.

Ejemplo de uso:
```
1 + 2 { esto es un bloque ignorado }
{ metadatos: autor = "Juan" }
(3 * 4) - 5 { datos extra }
```

En todos los casos, el resultado de la expresión aritmética es independiente del contenido entre llaves.

### Comentarios Multilínea
- Se pueden incluir comentarios multilínea usando `/* ... */` en cualquier parte de la expresión. Ejemplo:
  ```
  1 + 2 /* esto es un comentario */ * 3
  ```

### Ejemplos de programas válidos
- `1 + 2 * 3`
- `(4 + 5) / 2`
- `7 - (3 + 2)`
- `{ 1 + 2 } * 3`
- `/* comentario */ 8 / 2`

### Ejemplos de programas inválidos
- `1 + (2 * 3`  _(paréntesis desbalanceados)_
- `4 / /* comentario sin cierre`
- `{ 1 + 2 ` _(llave sin cerrar)_

## Archivos Relevantes

- `src/main/c/frontend/lexical-analysis/FlexPatterns.l`: Define los patrones léxicos (tokens) reconocidos por el analizador léxico (números, operadores, paréntesis, llaves, comentarios, etc).
- `src/main/c/frontend/lexical-analysis/FlexActions.c`: Implementa las acciones asociadas a cada token reconocido por Flex.
- `src/main/c/frontend/syntactic-analysis/BisonGrammar.y`: Define la gramática del lenguaje y las reglas de precedencia para las operaciones aritméticas.
- `src/main/c/frontend/syntactic-analysis/AbstractSyntaxTree.h/c`: Define la estructura del árbol de sintaxis abstracta (AST) y las funciones para manipularlo.

## Resumen de la Gramática

- Un programa es una expresión aritmética.
- Las expresiones pueden ser sumas, restas, multiplicaciones, divisiones, constantes enteras o expresiones agrupadas entre paréntesis.
- Se permite el uso de subexpresiones entre llaves y comentarios multilínea.

## Observaciones
- El parser reporta error si encuentra paréntesis, llaves o comentarios sin cerrar.
- Los comentarios y subexpresiones no afectan el resultado de la expresión principal, pero son reconocidos y pueden ser procesados por el compilador.

---

_Para más detalles, consultar los archivos fuente mencionados._
