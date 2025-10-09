#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *brightness;
    lv_obj_t *brightness_arc_1;
    lv_obj_t *brightness_lbl_1;
    lv_obj_t *brightness_val_lbl_1;
    lv_obj_t *obj0;
    lv_obj_t *icon_bulb;
    lv_obj_t *icon_plug;
    lv_obj_t *refresh_btn;
    lv_obj_t *refresh_btn_lbl;
} objects_t;

extern objects_t objects;

enum IconState {
    STATE_DISABLED = 0,
    STATE_ON = 1,
    STATE_OFF = 2,
};

enum ScreensEnum {
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_BRIGHTNESS = 2,
};

void create_screen_main();
void tick_screen_main();

void create_screen_brightness();
void tick_screen_brightness();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();
void set_icon_state(lv_obj_t *obj, enum IconState state);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/