[![✗](https://img.shields.io/badge/Release-v0.0.0-ffb600.svg?style=for-the-badge)](https://github.com/LyT-Labs/PICTURE/releases)

[![✗](https://github.com/LyT-Labs/PICTURE/actions/workflows/pipeline.yaml/badge.svg?branch=production)](https://github.com/LyT-Labs/PICTURE/actions/workflows/pipeline.yaml)

# PICTURE Compiler

Compilador del lenguaje **PICTURE** (PinkOS Interface Construction Terminology for UI Representation & Events) - Un lenguaje declarativo basado en indentación para construir interfaces de usuario para el sistema operativo experimental [PinkOS](https://github.com/lukpeluk/PinkOS).

Este proyecto implementa un compilador usando [Flex](https://github.com/westes/flex) y [Bison](https://www.gnu.org/software/bison) que traduce código PICTURE a C para el runtime de PinkOS.

* [Requirements](#requirements)
* [Inicio Rápido](#inicio-rápido)
* [Sintaxis PICTURE](#sintaxis-picture)
* [Configuration](#configuration)
* [Commands](#commands)
* [CI/CD](#cicd)
* [Recommended Extensions](#recommended-extensions)

## Requirements

* [Docker v28.3.2](https://www.docker.com/)

## Inicio Rápido

### 1. Compilar el compilador

```bash
# Usando Docker (recomendado)
docker compose run --rm compiler bash src/main/bash/build.sh

# O localmente (requiere flex, bison, cmake, gcc)
bash src/main/bash/build.sh
```

### 2. Ejecutar el compilador

El compilador lee desde `stdin` y escribe a `stdout`:

```bash
# Procesar un archivo .pic
./.build/Flex-Bison-Compiler < src/test/c/accept/10-nested-with-selection-order

# Guardar el resultado en un archivo
./.build/Flex-Bison-Compiler < mi_archivo.pic > salida.c

# Ver solo errores (redirigir stderr)
./.build/Flex-Bison-Compiler < mi_archivo.pic 2>/dev/null
```

### 3. Ejemplo completo

```bash
# Crear un archivo PICTURE
cat > mi_ui.pic << 'EOF'
---
- titulo: "Mi Aplicación"
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
EOF

# Generar código C
./.build/Flex-Bison-Compiler < mi_ui.pic > mi_ui.c
```

## Sintaxis PICTURE

### Estructura de un archivo

```yaml
---
- miVariable: "valor"
- colorFondo: blue
* selection_order: boton1, boton2, input1
* identifier: mi_pantalla
---

# contenedorPrincipal
	- background: white
	- width: 400
	- height: 300
	
	# boton1
		- text: "Click aquí"
		- color: colorFondo
		- on_press: handle_click
```

### Elementos del lenguaje

- `---` delimita la sección de headers/metadata (donde van las variables globales y otras configuraciones)
- `- nombre: valor` define una variable (que funciona como reemplazo textual, similar a un #define en C)
- `* selection_order: id1, id2, id3` define orden de navegación de los componentes, con tab se iterará el foco en ese orden
- `* identifier: nombre` define el nombre del módulo (genera `nombre_main()` como función principal e importa nombre.h como archivo con la lógica del programa)
- `#id` define un componente
- `- propiedad: valor` (tabulado) define propiedades del componente
- **Indentación con tabs**: define la jerarquía padre-hijo de componentes, el render ser hará jerárquicamente donde componentes hijos se dibujan sobre el padre en orden de arriba a abajo

### Valores builtin (expansión automática)

Los siguientes valores se expanden automáticamente como macros:

| Builtin | Tipo | Se expande a | Uso |
|---------|------|-------------|-----|
| `red`, `blue`, `green`, `white`, `black` | Color | `&color_red`, `&color_blue`, etc. | Propiedades de color |
| `s`, `m`, `l` | Tamaño | `1`, `2`, `3` | Tamaño de texto |
| `left`, `center`, `right` | Alineación | `ALIGN_LEFT`, `ALIGN_CENTER`, `ALIGN_RIGHT` | Alineación horizontal |

### Propiedades disponibles

| Propiedad | Tipo | Descripción |
|-----------|------|-------------|
| `background` | Color | Color de fondo |
| `text` | String | Texto a mostrar |
| `font_size` | Builtin/Number | Tamaño de fuente |
| `color` | Color | Color del texto |
| `border_size` | Number | Tamaño en píxeles del borde del componente |
| `border_color` | Color | Color del borde del componente |
| `alignment` | Builtin | Alineación horizontal |
| `y_position` | Number | Posición vertical absoluta, -1 para auto (distribuido uniformemente) |
| `width`, `height` | Number | Dimensiones interpretadas como porcentaje respecto al padre |
| `active` | Boolean (1/0) | Una especie de display: none, el componente deja de renderizarse, se ignora a la hora de iterar si era seleccionable y deja de considerarse para la distribución de sus componentes hermanos |
| `on_press` | Identifier | Callback al presionar |
| `on_keypress` | Identifier | Callback al escribir |
| `on_focus_gain` | Identifier | Callback al ganar foco |
| `on_focus_lost` | Identifier | Callback al perder foco |

Nota: el alineamiento vertical es siempre una distribución uniforme de los hijos, sin contar los que sobrescriban su posición con y_position

## Configuration

Set the following environment variables to control and configure the behaviour of the application:

| Name                  | Default | Description                                                                                                                                                           |
| :-------------------- | :-----: | :-------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `ENVIRONMENT`         | `Local` | The active environment name. The available environments are: `Local`, `Development` and `Production`.                                                                 |
| `LOG_IGNORED_LEXEMES` | `true`  | When `true`, logs all of the ignored lexemes found with Flex at `DEBUGGING` level. To remove those logs from the console output set it to `false`.                    |
| `LOGGING_LEVEL`       | `ALL`   | The minimum level to log in the console output. From lower to higher, the available levels are: `ALL`, `DEBUGGING`, `INFORMATION`, `WARNING`, `ERROR` and `CRITICAL`. |

_Docker Compose_ can read the variables from an `.env` file too (see `compose.yaml` file).

## Commands

### Start

Rises an ephemeral container, ready to start development:

```bash
docker compose run --rm compiler
```

### Build

Builds or rebuilds the entire compiler:

```bash
src/main/bash/build.sh
```

### Run

Compiles a program:

```bash
src/main/bash/run.sh <program>
```

where `<program>` is the path to the file that represents its entry-point.

### Test

Executes every available unit-test under `src/test/c` folder:

```bash
src/main/bash/test.sh
```

### Stop

Logout, destroy the ephemeral containers and shutdowns the cluster:

```bash
exit
docker compose down
```

### Docker

| Command                                 | Description                                             |
| :-------------------------------------- | :------------------------------------------------------ |
| `docker builder prune --all`            | Removes all builds and complete build cache.            |
| `docker compose --progress=plain build` | Forces a build or rebuild of the images in the cluster. |
| `docker image prune`                    | Removes all of the dangling images from Docker.         |
| `docker network prune`                  | Removes unused networks from Docker.                    |
| `docker volume prune`                   | Removes unused volumes from Docker.                     |

## CI/CD

To trigger an automatic integration on every push or PR (_Pull Request_), you must activate _GitHub Actions_ in the _Settings_ tab. Use the following configuration:

| Key                                                        | Value                                               |
| :--------------------------------------------------------- | :-------------------------------------------------- |
| `Actions permissions`                                      | `Allow all actions and reusable workflows`          |
| `Allow GitHub Actions to create and approve pull requests` | `false`                                             |
| `Artifact and log retention`                               | `30 days`                                           |
| `Fork pull request workflows from outside collaborators`   | `Require approval for all outside collaborators`    |
| `Workflow permissions`                                     | `Read repository contents and packages permissions` |

## Recommended Extensions

* [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
* [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)
* [Yash](https://marketplace.visualstudio.com/items?itemName=daohong-emilio.yash)
