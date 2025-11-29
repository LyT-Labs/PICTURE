# Especificación del Lenguaje PICTURE

## Descripción General

PICTURE (PinkOS Interactive Component Tree Universal Runtime Environment) es un lenguaje declarativo basado en indentación diseñado para construir interfaces de usuario para PinkOS. Este proyecto implementa un compilador que utiliza Flex para el análisis léxico y Bison para el análisis sintáctico, generando código C compatible con el runtime de PinkOS.

## Características Principales

- **Sintaxis declarativa basada en indentación**: Similar a Python o YAML
- **Sistema de componentes jerárquico**: Los componentes se anidan mediante tabs
- **Variables globales y macros builtin**: Soporte para valores reutilizables y expansión automática
- **Orden de selección**: Control explícito del flujo de navegación entre componentes
- **Generación de código estático**: Arrays de componentes contiguos en memoria para eficiencia

## Sintaxis del Lenguaje

### Estructura General

```yaml
---
- variable1: valor
- variable2: builtin
* selection_order: id1, id2, id3
* identifier: nombre_modulo
---

# componenteRaiz
	- propiedad: valor
	
	# componenteHijo
		- propiedad: valor
```

### Sección de Variables Globales

Delimitada por `---`, esta sección define:

1. **Variables**: `- nombre: valor`
   - Variables con valores literales se declaran como `const char *`
   - Variables con valores builtin se expanden como macros (no generan declaración)

2. **Selection Order**: `* selection_order: id1, id2, id3`
   - Define el orden de navegación entre componentes selectables
   - Solo incluye componentes con callbacks (on_press, on_keypress, etc.)

3. **Identifier**: `* identifier: nombre`
   - Define el nombre del módulo
   - Genera función `nombre_main()` y `#include <nombre.h>`

### Componentes

#### Declaración
```yaml
# nombreComponente
	- propiedad1: valor1
	- propiedad2: valor2
```

- El símbolo `#` seguido de un identificador declara un componente
- La indentación (tabs) define la jerarquía padre-hijo
- Los componentes hijos heredan la posición relativa al padre

#### Propiedades Soportadas

| Propiedad | Tipo | Descripción | Ejemplo |
|-----------|------|-------------|---------|
| `background` | Color | Color de fondo | `- background: white` |
| `text` | String | Texto a mostrar | `- text: "Hola"` |
| `font_size` | Builtin/Number | Tamaño de fuente | `- font_size: m` |
| `color` | Color | Color del texto | `- color: blue` |
| `align` | Builtin | Alineación horizontal | `- align: center` |
| `width` | Number | Ancho en píxeles | `- width: 400` |
| `height` | Number | Alto en píxeles | `- height: 300` |
| `x` | Number | Posición X | `- x: 10` |
| `y` | Number | Posición Y | `- y: 20` |
| `on_press` | Callback | Función al presionar | `- on_press: handle_click` |
| `on_keypress` | Callback | Función al escribir | `- on_keypress: handle_input` |
| `on_focus_gain` | Callback | Función al ganar foco | `- on_focus_gain: on_focus` |
| `on_focus_lost` | Callback | Función al perder foco | `- on_focus_lost: on_blur` |

### Valores Builtin

Los valores builtin se expanden automáticamente en tiempo de compilación:

#### Colores
| Builtin | Expansión | Tipo |
|---------|-----------|------|
| `red` | `&color_red` (0xFF0000) | `uint32_t*` |
| `blue` | `&color_blue` (0x0000FF) | `uint32_t*` |
| `green` | `&color_green` (0x00FF00) | `uint32_t*` |
| `white` | `&color_white` (0xFFFFFF) | `uint32_t*` |
| `black` | `&color_black` (0x000000) | `uint32_t*` |

#### Tamaños de Texto
| Builtin | Expansión | Tipo |
|---------|-----------|------|
| `s` | `1` | `int` |
| `m` | `2` | `int` |
| `l` | `3` | `int` |

#### Alineación
| Builtin | Expansión | Tipo |
|---------|-----------|------|
| `left` | `ALIGN_LEFT` | `Alignment` enum |
| `center` | `ALIGN_CENTER` | `Alignment` enum |
| `right` | `ALIGN_RIGHT` | `Alignment` enum |

**Importante**: Las variables con valores builtin NO generan declaraciones. Se expanden inline en cada uso.

### Jerarquía y Memoria

Los componentes se organizan en memoria usando **ordenamiento BFS (Breadth-First Search)**:
- Todos los componentes del mismo nivel están contiguos en memoria
- Cada padre tiene un puntero al primer hijo y un contador de hijos
- Los hijos de un componente están en posiciones consecutivas del array

Ejemplo:
```
mainContainer[0]
├── topPanel[1]
│   └── button1[4]
├── middlePanel[2]
│   └── button2[5]
└── bottomPanel[3]
    └── input1[6]
```

## Ejemplos

### Ejemplo Básico
```yaml
---
- titulo: "Mi App"
* identifier: main_screen
---

# ventana
	- background: white
	- width: 800
	- height: 600
	
	# label
		- text: titulo
		- font_size: l
		- color: blue
		- align: center
```

### Ejemplo con Selection Order
```yaml
---
- mainColor: blue
* selection_order: btn1, btn2, input1
* identifier: form_screen
---

# container
	- background: white
	
	# btn1
		- text: "Aceptar"
		- color: mainColor
		- on_press: handle_accept
		- on_focus_gain: on_focus
	
	# btn2
		- text: "Cancelar"
		- on_press: handle_cancel
		- on_focus_gain: on_focus
	
	# input1
		- text: "Escriba aquí..."
		- on_keypress: handle_input
		- on_focus_gain: on_focus
	
	# label1
		- text: "Etiqueta informativa"
		- color: mainColor
```

## Código Generado

El compilador genera código C con:

1. **Headers**: Includes necesarios + `#include <identifier.h>`
2. **Colores estáticos**: Variables `uint32_t` para colores builtin
3. **ComponentRegistry**: Struct con punteros a cada componente por ID
4. **Arrays estáticos**: 
   - `component_array[]`: Array de componentes en orden BFS
   - `component_selection_order[]`: Array de punteros para navegación
5. **Función de inicialización**: `initialize_component_tree()`
6. **Función main del módulo**: `identifier_main()`

### Estructura de Component

```c
typedef struct Component {
    uint32_t* bg_color;
    char* text;
    uint32_t* text_color;
    int text_size;
    int border_size;
    uint32_t* border_color;
    int width;
    int height;
    Alignment alignment;
    int x_position;
    int y_position;
    int active;
    struct Component* children;
    int children_count;
    struct Component* parent;
    OnPressCallback on_press;
    OnKeyPressCallback on_key_press;
    OnFocusCallback on_focus_gain;
    OnFocusCallback on_focus_lost;
    void* variables;
    void* metadata;
    int needs_full_redraw;
} Component;
```

## Archivos Relevantes

### Frontend (Análisis)
- `src/main/c/frontend/lexical-analysis/FlexPatterns.l`: Patrones léxicos (tokens)
- `src/main/c/frontend/lexical-analysis/FlexActions.c`: Acciones del lexer
- `src/main/c/frontend/syntactic-analysis/BisonGrammar.y`: Gramática del lenguaje
- `src/main/c/frontend/syntactic-analysis/BisonActions.c`: Acciones semánticas
- `src/main/c/frontend/syntactic-analysis/AbstractSyntaxTree.h/c`: Definición del AST

### Backend (Generación)
- `src/main/c/backend/domain-specific/Validator.c`: Validación semántica del AST
- `src/main/c/backend/code-generation/Generator.c`: Generador de código C

### Tests
- `src/test/c/accept/`: Casos de prueba válidos
- `src/test/c/reject/`: Casos de prueba inválidos

## Validación

El compilador valida:

✓ Variables no duplicadas en la sección global  
✓ Componentes tienen ID único  
✓ Propiedades con tipos correctos  
✓ Width, height, x, y son números o variables  
✓ Callbacks son identificadores válidos  
✓ Selection order referencia componentes existentes  

## Limitaciones

- No soporta expresiones aritméticas en propiedades
- No permite anidación de bloques de variables
- Los callbacks deben estar implementados externamente
- La indentación debe ser con tabs, no espacios

---

_Para más detalles de implementación, consultar los archivos fuente en `src/main/c/`._
