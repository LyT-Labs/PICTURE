#include "ui_runtime.h"

const char *textSize = "m";
const char *color = "blue";

int main(void) {
    ui_init();

    UIComponent main = ui_create_root("main");
    ui_set_background(main, "white");

    UIComponent background = ui_create_root("background");
    ui_set_background(background, "color");
    ui_set_width(background, 100);
    ui_set_height(background, 100);

    UIComponent title = ui_create_root("title");
    ui_set_text(title, "Hola Mundo!");
    ui_set_font_size(title, "textSize");
    ui_set_x(title, 20);
    ui_set_y(title, 20);

    UIComponent button1 = ui_create_root("button1");
    ui_set_text(button1, "Cambiar color");
    ui_set_x(button1, 20);
    ui_set_y(button1, 70);
    ui_set_on_press(button1, handle_press_color);
    ui_set_on_focus_gain(button1, handle_release_color);

    ui_run();
    return 0;
}
