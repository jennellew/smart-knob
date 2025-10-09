#ifndef DISPLAY_SLEEP_H
#define DISPLAY_SLEEP_H

#include "esp_lcd_panel_ops.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize display sleep/dim functionality
 * 
 * @param panel_handle LCD panel handle
 * @param lvgl_task_handle LVGL task handle for priority control
 */
void display_sleep_init(esp_lcd_panel_handle_t panel_handle, TaskHandle_t lvgl_task_handle);

/**
 * @brief Dim the display backlight
 */
void display_dim(void);

/**
 * @brief Put display to sleep (turn off panel and lower LVGL priority)
 */
void display_sleep(void);

/**
 * @brief Wake up the display and restore normal operation
 */
void display_wake(void);

/**
 * @brief Call this when any user activity is detected (touch, button, etc.)
 * This will wake the display if sleeping and reset all timers
 */
void display_activity_detected(void);

#ifdef __cplusplus
}
#endif

#endif // DISPLAY_SLEEP_H