#include "actions.h"
#include "ui.h"
#include "vars.h"
#include "../../knob.h"
#include "lvgl.h"
#include "../../KasaBridge.h"
#include "screens.h"

// Callback function that runs when encoder value changes
static void encoder_value_changed(int8_t value) {
	// Update the LVGL arc value
	// This will trigger action_on_arc_change which updates the label
	if (objects.brightness_arc_1) {
		lv_arc_set_value(objects.brightness_arc_1, value);
	}

	if (objects.brightness_val_lbl_1) {
		lv_label_set_text_fmt(objects.brightness_val_lbl_1, "%d%%", value);
	}
	
	// Also update the variable
	set_var_brightness_val(value);

	// Update Kasa bulb
	if (bedroomLight != NULL) {
	    kasaSetBrightness(bedroomLight, value);
	}
}

void action_ctrl_brightness(lv_event_t * e) {
	// Update device states before jumping to next screen

	if (bedroomLight != NULL) {
		kasaRefreshInfoBulb(bedroomLight);

		// Sync encoder with current brightness value before activating
		if (kasaGetOnOffStateBulb(bedroomLight) == 0) {
			set_var_brightness_val(BULB_BRIGHTNESS_OFF);
		}
		knob_set_value(get_var_brightness_val());

		// Update screen elements
		if (objects.brightness_arc_1) {
			lv_arc_set_value(objects.brightness_arc_1, get_var_brightness_val());
		}

		if (objects.brightness_val_lbl_1) {
			lv_label_set_text_fmt(objects.brightness_val_lbl_1, "%d%%", get_var_brightness_val());
		}
	}
	
	// Register the callback to sync encoder changes to LVGL
	knob_set_value_changed_callback(encoder_value_changed);
	
	// Enable encoder when entering brightness screen
	knob_activate();

	update_ui();

	loadScreen(SCREEN_ID_BRIGHTNESS);
}

void action_back_to_main(lv_event_t * e) {
	// Update device states before jumping back to main screen
	kasaRefreshInfoPlug(livingRoomLight);
	kasaRefreshInfoBulb(bedroomLight);

	// Unregister callback
	knob_set_value_changed_callback(NULL);
	
	// Disable encoder when leaving brightness screen
	knob_deactivate();

	update_ui();
	
	loadScreen(SCREEN_ID_MAIN);
}

void action_on_arc_change(lv_event_t * e) {
	lv_obj_t * label = lv_event_get_user_data(e);

	// Update the brightness value label
	lv_label_set_text_fmt(label, "%d%%", get_var_brightness_val());

	// Also update the encoder value and sync with brightness
	
	if (knob_is_active()) {
		knob_set_value(get_var_brightness_val());
	}
}

void action_lamp_plug_pressed(lv_event_t * e) {
	uint8_t plugState = kasaGetOnOffStatePlug(livingRoomLight);
	if (livingRoomLight != NULL) {
		if (plugState == 0) {
			kasaSetOnOffStatePlug(livingRoomLight, 1);
			set_icon_state(objects.icon_plug, STATE_ON);
		} else {
			kasaSetOnOffStatePlug(livingRoomLight, 0);
			set_icon_state(objects.icon_plug, STATE_OFF);
		}
		
	} else {

	}
}

void action_toggle_bulb_onoff(lv_event_t * e) {
	if (bedroomLight != NULL) {
		int tempValue = 0;
		if (kasaGetOnOffStateBulb(bedroomLight) == 0) {
			kasaSetOnOffStateBulb(bedroomLight, 1);
			tempValue = get_var_brightness_val() == 0 ? DEFAULT_BULB_BRIGHTNESS_VAL : get_var_brightness_val();
		} else {
			kasaSetOnOffStateBulb(bedroomLight, 0);
			tempValue = 0;
			

		}
		encoder_value_changed(tempValue);
	}
	
}

void action_update_bulb_brightness(lv_event_t * e) {
	// Update Kasa bulb after user releases arc
	if (bedroomLight != NULL) {
	    kasaSetBrightness(bedroomLight, get_var_brightness_val());
	}
}

void action_refresh_devices() {
	// Update device states
	kasaRefreshInfoPlug(livingRoomLight);
	kasaRefreshInfoBulb(bedroomLight);
}

void update_ui() {
  if (bedroomLight != NULL) {
    if (kasaGetOnOffStateBulb(bedroomLight) == 0) {
      set_var_brightness_val(0);
      set_icon_state(objects.icon_bulb, STATE_OFF);
    } else {
      set_var_brightness_val(kasaGetBrightness(bedroomLight) == 0 ? DEFAULT_BULB_BRIGHTNESS_VAL : kasaGetBrightness(bedroomLight));
      set_icon_state(objects.icon_bulb, STATE_ON);
    }
    // encoder_value_changed(get_var_brightness_val());
    
  } else {
    set_icon_state(objects.icon_bulb, STATE_DISABLED);
  }

  if (livingRoomLight != NULL) {
    set_icon_state(objects.icon_plug, kasaGetOnOffStatePlug(livingRoomLight) == 0 ? STATE_OFF : STATE_ON);
  } else {
    set_icon_state(objects.icon_plug, STATE_DISABLED);
  }
}