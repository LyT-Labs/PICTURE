#include "ui_runtime.h"

int main(void) {
    ui_init();

    UIComponent main = ui_create_root("main");
    ui_set_background(main, "white");

    UIComponent title = ui_create_root("title");
    ui_set_text(title, "Hola Mundo!");
    ui_set_font_size(title, "m");
    ui_set_background(title, "red");

    ui_run();
    return 0;
}
