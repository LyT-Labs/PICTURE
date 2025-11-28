#ifndef UI_RUNTIME_HEADER
#define UI_RUNTIME_HEADER

typedef struct UIComponent {
    const char *id;
    void *internal;   // puntero a lo que quieras dentro del SO (puede ser NULL por ahora)
} UIComponent;

void ui_init(void);

UIComponent ui_create_root(const char *id);
UIComponent ui_create_child(UIComponent parent, const char *id);

void ui_set_background(UIComponent c, const char *value);
void ui_set_text(UIComponent c, const char *value);
void ui_set_font_size(UIComponent c, const char *value);
void ui_set_align(UIComponent c, const char *value);

void ui_set_width(UIComponent c, int value);
void ui_set_height(UIComponent c, int value);
void ui_set_x(UIComponent c, int value);
void ui_set_y(UIComponent c, int value);

void ui_set_on_press(UIComponent c, void (*fn)(void));
void ui_set_on_focus_gain(UIComponent c, void (*fn)(void));
void ui_set_on_focus_lost(UIComponent c, void (*fn)(int));


void ui_run(void);

#endif
