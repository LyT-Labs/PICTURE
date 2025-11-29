#include "ui_runtime.h"
#include <stdio.h>

void ui_init(void) {
    printf("[UI] init\n");
}

UIComponent ui_create_root(const char *id) {
    printf("[UI] create root: %s\n", id);
    UIComponent c = { id, NULL };
    return c;
}

UIComponent ui_create_child(UIComponent parent, const char *id) {
    printf("[UI] create child '%s' inside '%s'\n", id, parent.id);
    UIComponent c = { id, NULL };
    return c;
}

void ui_set_background(UIComponent c, const char *value) {
    printf("[UI] %s.background = %s\n", c.id, value);
}

void ui_set_text(UIComponent c, const char *value) {
    printf("[UI] %s.text = %s\n", c.id, value);
}

void ui_set_font_size(UIComponent c, const char *value) {
    printf("[UI] %s.font_size = %s\n", c.id, value);
}

void ui_set_align(UIComponent c, const char *value) {
    printf("[UI] %s.align = %s\n", c.id, value);
}

void ui_set_width(UIComponent c, int value) {
    printf("[UI] %s.width = %d\n", c.id, value);
}

void ui_set_height(UIComponent c, int value) {
    printf("[UI] %s.height = %d\n", c.id, value);
}

void ui_set_x(UIComponent c, int value) {
    printf("[UI] %s.x = %d\n", c.id, value);
}

void ui_set_y(UIComponent c, int value) {
    printf("[UI] %s.y = %d\n", c.id, value);
}

void ui_set_on_press(UIComponent c, void (*fn)(void)) {
    printf("[UI] %s.on_press = <fn>\n", c.id);
}

void ui_set_on_select(UIComponent c, void (*fn)(void)) {
    printf("[UI] %s.on_select = <fn>\n", c.id);
}

void ui_set_on_keypress(UIComponent c, void (*fn)(int)) {
    printf("[UI] %s.on_keypress = <fn>\n", c.id);
}

void ui_set_on_focus_gain(UIComponent c, void (*fn)(void)) {
    printf("[UI] %s.on_focus_gain = <fn>\n", c.id);
}

void ui_set_on_focus_lost(UIComponent c, void (*fn)(int)) {
    printf("[UI] %s.on_focus_lost = <fn>\n", c.id);
}

void ui_run(void) {
    printf("[UI] run loop\n");
}
