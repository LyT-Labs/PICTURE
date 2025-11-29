-> Este documento es un borrador del diseño de un sistema de tipos y propiedades más complejo y expresivo para el framework de PICTURE.
<br>-> Todo muy sujeto a cambios (sugerencias bienvenidas).

# Variables, propiedades y PictureValues
- Las variables y propiedades en Picture son ambos PictureValues.
- Son básicamente los únicos 2 lugares donde se usan PictureValues.

- Las variables tienen scope por componente, con un comportamiento similar al scope en bloques de C.
    -> Por este motivo no pueden ser accedidas directamente y requieren getters y setters, pues el scope puede cambiar en tiempo de ejecución.
    - El getter se encargaría de ver si la variable está definida en el scope local y si no recursivamente buscarla en los scopes padres.
    - El setter se encargaría de definir la variable en el scope local o en el scope externo más cercano según lo que se desee.
    - Ambos getters y setters usan PictureValues para definir/retornar el valor.
    - Los getters y setters son posibles pues las variables son declaradas en tiempo de compilación, 
        - (en ejecución pueden sobrescribirse pero no crearse nuevas).

- Las propiedades siempre son definidas en el scope local del componente y para accederlas se puede usar el puntero directamente pues no hay ambigüedad.
    - Esto es además pues el puntero jamás cambiará y es conocido en tiempo de compilación.
    - Por eso no es recomendable acceder desde un componente a una propiedad de un RuntimeComponent, pues este puede dejar de existir.
- Igual, como las propiedades son PictureValues, para obtener el valor real se debe usar una función de casteo de PictureValue a algún tipo primitivo.
- Y para setear una propiedad se debe usar un setter que reciba un PictureValue.
    - Esto en principio es por si hay optimizaciones posibles como dirty flags

- Los PictureValues son una abstracción que permite manejar los distintos tipos de datos de PICTURE de manera uniforme.
    - Un PictureValue es una unión que puede contener distintos tipos de datos.
    - Puede ser un valor primitivo (color, int, textSize, etc...), ser NULL, o ser un puntero a otro PictureValue o a una función.
    - Si es un puntero a otro PictureValue, se sigue el puntero hasta llegar a un valor primitivo, NULL o función.
    - Si es una función, esta retornará un PictureValue, y se seguirá el mismo proceso con el valor retornado para conseguir el valor final.
    - Esto permite que las variables y propiedades puedan ser definidas como funciones, permitiendo así comportamientos dinámicos.
        - Un caso de uso claro de uso es asociar a una propiedad el getter de una variable
        - (Esto es lo que pasa internamente cuando en el PictureSchema se asocia una variable a una propiedad).
    - Es tarea del programador asegurarse de que las funciones/referencias retornen valores del tipo esperado y evitar llamadas recursivas o circulares.

- Para acceder al valor real de un PictureValue, se deben usar funciones de casteo que retornen el tipo primitivo esperado.
    - Si el PictureValue no es del tipo esperado, la función de casteo retornará un error en runtime.
- Si en un caso más de un tipo es posible, se puede hacer un switch sobre el tipo del PictureValue y en cada caso castear como corresponda.

// Todo: ver bien los tipos de datos soportados y documentarlos acá.
## Tipos de dato soportados en PictureValues:
    - NULL
    - Int (int) un int de C que podés usar en distintos contextos ya que puede ser casteado a varios otros tipos.
    - Float (float) un float de C que podés usar en distintos contextos ya que puede ser casteado a varios otros tipos.
    - Color (uint32_t) en formato 0xAARRGGBB aunque el canal alfa no se usa por ahora
    - TextSize (uint32_t) que indica la escala del texto (1 sería el tamaño base que es 8x8 píxeles por carácter)
    - Ems (float) cantidad de Ems, que es una unidad relativa al tamaño del texto actual del sistema. 
    - Rems (float) cantidad de Rems, que es una unidad relativa al tamaño del texto del padre.
        - Si el padre no tiene definido un tamaño de texto, se usa el tamaño de texto del sistema.
    - RelativePercent (float) vale un valor relativo en porcentaje (0 a 100).

    - Pixels (Pixels) vale un valor en píxeles.
        - La idea es que no se use mucho a mano pero que a este tipo se castee lo demás para el renderizado.
        - Es el tipo de dato que devuelven las funciones para las expresiones en alignment y size.

    - //WIP: Alignment o algo así (enum) un enum que indica cosas de más alto nivel que un valor como "50%", por ejemplo "distribuir uniformemente"
        - No se me ocurre algo más que distribuir uniformemente por ahora, pero quizá algo como un flex para varios hijos puede ser útil
        - Quizá algo que diga "el tamaño de mi texto" o algo así, el tema es que debería poder indicarse un límite máximo...
        - O algo que sea "quiero ocupar el espacio que me queda" pero eso mezcla size con alignment y no sé bien cómo sería posible

- Ambos Em y Rem son tipos que simplemente dicen "esto leelo como el tamaño del texto actual" o "esto leelo como el tamaño del texto del padre".
- Es la función que lo castee a un tipo específico la que debe hacer la conversión a un valor numérico en TextSize o pixeles.
- O sea, no es que Em y Rem sean tipos en sí mismos, no se puede castear algo a un Em o Rem, es como el tipo "center", dice qué hacer.

## Casteos entre tipos de PictureValue
- En el contexto de PictureValues, castear también hace referencia a traer el valor real de un PictureValue, que puede no implicar un cambio de tipo.
    - Por ejemplo, si un PictureValue es de tipo Pixels, castear a Int simplemente trae el valor entero que representa la cantidad de píxeles.

- Hay tipos de dato que no pueden usarse sin castear, por ejemplo, no se puede usar un RelativePercent sin castear a Pixels o Int.

- Para los casteos, la librería ofrece un conjunto de funciones agrupadas dentro del struct picture_value.
    - Ej. picture_value.to_int(pv) castea el PictureValue pv a un int.


## Borrador
 - Los PictureValues guardan un puntero al componente al que hacen referencia.
 - Esto es porque hay tipos de dato, como Em o % que dependen del contexto
 - Otro caso que depende del contexto es el caso de una variable, ya que tienen que saber en que scope buscar.
 
 - Esto agrega algo de complejidad a la hora de leer un PictureValue, ya que hay que seguir el puntero al componente y ver el contexto.
 - El componente que se usará para interpretar el PictureValue es el último encontrado en el recorrido de punteros.
 - Dicho componente lo utilizarían las funciones de casteo para interpretar el valor, y también se le pasará como argumento a la función si el PictureValue es una función.

picture_value.to_int(pv) <- si es primitivo


# Factos:
 - Una expresión (una cuenta básicamente) es un tipo de dato de alto nivel, y debe codificarse como una función que retorne un PictureValue.
 - Esa función sabe no solo el tipo de dato de lo que está calculando, sino también dónde se usará, ya que es un elemento en tiempo de compilación.
 - Esto es porque por ejemplo la expresión "50% + 1rem" si está en height usará el 50% del height, y si está en width usará el 50% del width.
    - Esto significa que no puede haber una función genérica "50% + 1rem", las expresiones siempre están atadas a un contexto y propiedad puntuales.
 - En todos los casos, el tipo de retorno será Pixels y la función tendrá harcodeado si es width o height o x o y.



