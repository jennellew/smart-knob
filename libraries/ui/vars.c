#include "vars.h"
#include "../../KasaBridge.h"
#include <stdbool.h>
#include <stdint.h>

int8_t brightness_val = DEFAULT_BULB_BRIGHTNESS_VAL; 

int8_t get_var_brightness_val() {
    return brightness_val;
}

void set_var_brightness_val(int8_t value) {
    brightness_val = value;
}
