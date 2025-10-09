#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"
#include "../../fa_icons.h"

// Icons
#define ICON_LIGHTBULB "\xEF\x83\xAB"
#define ICON_PLUG "\xEF\x87\xA6"

// Colours
lv_color_t palette_white = LV_COLOR_MAKE(255, 255, 255);
lv_color_t palette_grey = LV_COLOR_MAKE(90, 90, 90);
lv_color_t palette_dark_grey = LV_COLOR_MAKE(60, 60, 60);

// Global styles
static lv_style_t fa_icon_style;

objects_t objects;
lv_obj_t *tick_value_change_obj;
uint32_t active_theme_index = 0;

static void event_handler_cb_brightness_brightness_arc_1(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target(e);
        if (tick_value_change_obj != ta) {
            int32_t value = lv_arc_get_value(ta);
            set_var_brightness_val(value);
        }
    }
}

// Styles
void make_styles(void) {
  lv_style_init(&fa_icon_style);
  lv_style_set_text_font(&fa_icon_style, &fa_icons);
  // lv_style_set_bg_color(&fa_icon_style, lv_color_hex(0xff904c4c));
  // lv_style_set_bg_opa(&fa_icon_style, 255);
  lv_style_set_size(&fa_icon_style, 100);
  lv_style_set_radius(&fa_icon_style, 50);
  lv_style_set_pad_top(&fa_icon_style, 10);
  lv_style_set_border_width(&fa_icon_style, 6);
  lv_style_set_text_align(&fa_icon_style, LV_TEXT_ALIGN_CENTER);
}


// Icon "buttons"
void make_icons(lv_obj_t* scr) {
  lv_obj_t *icon_bulb = lv_label_create(scr);
  objects.icon_bulb = icon_bulb;
  lv_label_set_text(icon_bulb, ICON_LIGHTBULB);
  lv_obj_add_style(icon_bulb, &fa_icon_style, 0);
  lv_obj_align(icon_bulb, LV_ALIGN_CENTER, 100, -20);
  lv_obj_add_event_cb(icon_bulb, action_ctrl_brightness, LV_EVENT_PRESSED, (void *)0);
  lv_obj_add_flag(icon_bulb, LV_OBJ_FLAG_CLICKABLE);
  set_icon_state(icon_bulb, STATE_DISABLED);

  lv_obj_t *icon_plug = lv_label_create(scr);
  objects.icon_plug = icon_plug;
  lv_label_set_text(icon_plug, ICON_PLUG);
  lv_obj_add_style(icon_plug, &fa_icon_style, 0);
  lv_obj_align(icon_plug, LV_ALIGN_CENTER, -100, -20);
  lv_obj_add_event_cb(icon_plug, action_lamp_plug_pressed, LV_EVENT_PRESSED, (void *)0);
  lv_obj_add_flag(icon_plug, LV_OBJ_FLAG_CLICKABLE);
  set_icon_state(icon_plug, STATE_DISABLED);
}

void set_icon_state(lv_obj_t *obj, enum IconState state) {
  switch (state) {
    case STATE_DISABLED: 
      lv_obj_set_style_text_color(obj, palette_dark_grey, 0);
      lv_obj_set_style_border_color(obj, palette_dark_grey, LV_PART_MAIN | LV_STATE_DEFAULT);
      break;
    case STATE_ON: 
      if (obj == objects.icon_bulb) {
          lv_obj_set_style_text_color(obj, lv_palette_main(LV_PALETTE_AMBER), 0);
          lv_obj_set_style_border_color(obj, lv_palette_main(LV_PALETTE_AMBER), LV_PART_MAIN | LV_STATE_DEFAULT);
      } else if (obj == objects.icon_plug) {
          lv_obj_set_style_text_color(obj, lv_palette_main(LV_PALETTE_LIGHT_GREEN), 0);
          lv_obj_set_style_border_color(obj, lv_palette_main(LV_PALETTE_LIGHT_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
      }
      break;
    case STATE_OFF: 
      lv_obj_set_style_text_color(obj, palette_grey, 0);
      lv_obj_set_style_border_color(obj, palette_grey, LV_PART_MAIN | LV_STATE_DEFAULT);
      break;
  }
}

void create_screen_main() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.main = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 360, 360);

    make_styles();
    make_icons(obj);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.refresh_btn = obj;
            lv_obj_set_pos(obj, 130, 256);
            lv_obj_set_size(obj, 100, 50);
            lv_obj_set_style_radius(obj, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff5ca5df), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_add_event_cb(obj, action_refresh_devices, LV_EVENT_PRESSED, (void *)0);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.refresh_btn_lbl = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "refresh");
                }
            }
        }
    }
    
    tick_screen_main();
}

void tick_screen_main() {
    
}

void create_screen_brightness() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.brightness = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 360, 360);
    {
        lv_obj_t *parent_obj = obj;
        
        {
            // brightness_lbl_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.brightness_lbl_1 = obj;
            lv_obj_set_pos(obj, 103, 265);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "brightness");
        }
        {
            // brightness_val_lbl_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.brightness_val_lbl_1 = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_fmt(obj, "%d%%", get_var_brightness_val());
            lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        }
        
        {
            lv_obj_t *obj = lv_btn_create(parent_obj);
            lv_obj_set_pos(obj, 16, 0);
            lv_obj_set_size(obj, 48, 48);
            lv_obj_add_event_cb(obj, action_back_to_main, LV_EVENT_PRESSED, (void *)0);
            lv_obj_set_style_align(obj, LV_ALIGN_LEFT_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj0 = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xfffafafa), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "<");
                }
            }
        }
        
        {
            // brightness_arc_1
            lv_obj_t *obj = lv_arc_create(parent_obj);
            objects.brightness_arc_1 = obj;
            lv_obj_set_pos(obj, 80, 80);
            lv_obj_set_size(obj, 200, 200);
            lv_obj_add_event_cb(obj, action_on_arc_change, LV_EVENT_VALUE_CHANGED, objects.brightness_val_lbl_1);
            lv_obj_add_event_cb(obj, action_update_bulb_brightness, LV_EVENT_RELEASED, objects.brightness_val_lbl_1);
            lv_obj_add_event_cb(obj, event_handler_cb_brightness_brightness_arc_1, LV_EVENT_ALL, 0);
            lv_arc_set_value(obj, get_var_brightness_val());
        }

        // Move label to the top, so it can capture clicks
        lv_obj_move_foreground(objects.brightness_val_lbl_1);
        lv_obj_add_event_cb(objects.brightness_val_lbl_1, action_toggle_bulb_onoff, LV_EVENT_PRESSED, objects.brightness_val_lbl_1);
    }
    
    tick_screen_brightness();
}

void tick_screen_brightness() {

}


typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
    tick_screen_brightness,
};
void tick_screen(int screen_index) {
    tick_screen_funcs[screen_index]();
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen_funcs[screenId - 1]();
}

void create_screens() {
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    
    create_screen_main();
    create_screen_brightness();
}
