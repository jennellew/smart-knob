// Add to your main code or lcd_bsp.c

#include "esp_timer.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "lcd_bl_pwm_bsp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// Global variables
static esp_lcd_panel_handle_t g_panel_handle = NULL;
static TaskHandle_t g_lvgl_task_handle = NULL;
static esp_timer_handle_t dim_timer = NULL;
static esp_timer_handle_t sleep_timer = NULL;
static SemaphoreHandle_t display_state_mutex = NULL;
static bool display_is_on = true;
static bool display_is_dimmed = false;

#define DISPLAY_DIM_TIMEOUT_MS 15000    // 15 seconds - dim backlight
#define DISPLAY_SLEEP_TIMEOUT_MS 30000  // 30 seconds - turn off display

#define BACKLIGHT_NORMAL 40
#define BACKLIGHT_DIM 10

#define LVGL_TASK_PRIORITY_NORMAL 2
#define LVGL_TASK_PRIORITY_LOW 1

#define LOCK_DISPLAY_STATE() xSemaphoreTake(display_state_mutex, portMAX_DELAY)
#define UNLOCK_DISPLAY_STATE() xSemaphoreGive(display_state_mutex)

// Function to dim backlight
void display_dim(void)
{
    LOCK_DISPLAY_STATE();
    if (display_is_on && !display_is_dimmed) {
        // Dim the backlight
        setUpdutySubdivide(BACKLIGHT_DIM);
        display_is_dimmed = true;
        ESP_LOGI("DISPLAY", "Display dimmed");
    }
    UNLOCK_DISPLAY_STATE();
}

// Function to turn off display
void display_sleep(void)
{
    LOCK_DISPLAY_STATE();
    if (display_is_on && g_panel_handle != NULL) {
        // Turn off backlight
        setUpdutySubdivide(0);
        
        // Turn off display panel
        esp_lcd_panel_disp_on_off(g_panel_handle, false);
        
        // Lower LVGL task priority to save CPU cycles
        if (g_lvgl_task_handle != NULL) {
            vTaskPrioritySet(g_lvgl_task_handle, LVGL_TASK_PRIORITY_LOW);
            ESP_LOGI("DISPLAY", "LVGL task priority lowered");
        }
        
        display_is_on = false;
        display_is_dimmed = false;
        ESP_LOGI("DISPLAY", "Display sleeping");
    }
    UNLOCK_DISPLAY_STATE();
}

// Function to wake up display
void display_wake(void)
{
    LOCK_DISPLAY_STATE();
    
    bool was_off = !display_is_on;
    bool was_dimmed = display_is_dimmed;
    
    if (!display_is_on && g_panel_handle != NULL) {
        // Turn on display panel first
        esp_lcd_panel_disp_on_off(g_panel_handle, true);
        
        // Small delay to let panel stabilize
        vTaskDelay(pdMS_TO_TICKS(10));
        
        // Restore LVGL task priority
        if (g_lvgl_task_handle != NULL) {
            vTaskPrioritySet(g_lvgl_task_handle, LVGL_TASK_PRIORITY_NORMAL);
            ESP_LOGI("DISPLAY", "LVGL task priority restored");
        }
        
        // Turn on backlight to normal
        setUpdutySubdivide(BACKLIGHT_NORMAL);
        
        display_is_on = true;
        display_is_dimmed = false;
        ESP_LOGI("DISPLAY", "Display awake");
    } else if (display_is_dimmed) {
        // Just restore brightness if only dimmed
        setUpdutySubdivide(BACKLIGHT_NORMAL);
        display_is_dimmed = false;
        ESP_LOGI("DISPLAY", "Display brightness restored");
    }
    
    UNLOCK_DISPLAY_STATE();
    
    // Reset timers outside of mutex to avoid deadlock
    if (was_off || was_dimmed) {
        if (dim_timer != NULL) {
            esp_timer_stop(dim_timer);
            esp_timer_start_once(dim_timer, DISPLAY_DIM_TIMEOUT_MS * 1000);
        }
        if (sleep_timer != NULL) {
            esp_timer_stop(sleep_timer);
            esp_timer_start_once(sleep_timer, DISPLAY_SLEEP_TIMEOUT_MS * 1000);
        }
    }
}

// Timer callback for dimming
static void dim_timer_callback(void *arg)
{
    display_dim();
}

// Timer callback for sleep
static void sleep_timer_callback(void *arg)
{
    display_sleep();
}

// Initialize sleep timer
void display_sleep_init(esp_lcd_panel_handle_t panel_handle, TaskHandle_t lvgl_task_handle)
{
    // Create mutex for thread safety
    display_state_mutex = xSemaphoreCreateMutex();
    if (display_state_mutex == NULL) {
        ESP_LOGE("DISPLAY", "Failed to create display state mutex");
        return;
    }
    
    g_panel_handle = panel_handle;
    g_lvgl_task_handle = lvgl_task_handle;
    
    // Create dim timer (30 seconds)
    const esp_timer_create_args_t dim_timer_args = {
        .callback = &dim_timer_callback,
        .name = "display_dim"
    };
    ESP_ERROR_CHECK(esp_timer_create(&dim_timer_args, &dim_timer));
    ESP_ERROR_CHECK(esp_timer_start_once(dim_timer, DISPLAY_DIM_TIMEOUT_MS * 1000));
    
    // Create sleep timer (60 seconds)
    const esp_timer_create_args_t sleep_timer_args = {
        .callback = &sleep_timer_callback,
        .name = "display_sleep"
    };
    ESP_ERROR_CHECK(esp_timer_create(&sleep_timer_args, &sleep_timer));
    ESP_ERROR_CHECK(esp_timer_start_once(sleep_timer, DISPLAY_SLEEP_TIMEOUT_MS * 1000));
    
    // ESP_LOGI("DISPLAY", "Sleep timers initialized (dim: 30s, sleep: 60s)");
}

// Call this in your touch callback to reset the timer
void display_activity_detected(void)
{
    // Wake display if sleeping
    if (!display_is_on || display_is_dimmed) {
        display_wake();
    } else {
        // Just reset the timer if already awake
        if (sleep_timer != NULL) {
            esp_timer_restart(sleep_timer, DISPLAY_SLEEP_TIMEOUT_MS * 1000);
        }
    }
}