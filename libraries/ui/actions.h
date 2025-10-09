#ifndef EEZ_LVGL_UI_EVENTS_H
#define EEZ_LVGL_UI_EVENTS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

static void encoder_value_changed(int8_t value);
extern void action_ctrl_brightness(lv_event_t * e);
extern void action_back_to_main(lv_event_t * e);
extern void action_on_arc_change(lv_event_t * e);
extern void action_lamp_plug_pressed(lv_event_t * e);
extern void action_toggle_bulb_onoff(lv_event_t * e);
extern void action_update_bulb_brightness(lv_event_t * e);
extern void action_refresh_devices();
extern void update_ui();


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_EVENTS_H*/