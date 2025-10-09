#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations
#define BULB_BRIGHTNESS_OFF 0
#define DEFAULT_BULB_BRIGHTNESS_VAL 25

// Native global variables

extern int8_t get_var_brightness_val();
extern void set_var_brightness_val(int8_t value);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/