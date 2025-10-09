#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "bidi_switch_knob.h"

#include "esp_err.h"
#include "Arduino.h"
#include "knob.h"
#include "display_sleep.h"  // Add this include

#include "esp_log.h"
static const char *TAG = "encoder";

#define EXAMPLE_ENCODER_ECA_PIN    8
#define EXAMPLE_ENCODER_ECB_PIN    7

#define SET_BIT(reg,bit) (reg |= ((uint32_t)0x01<<bit))
#define READ_BIT(reg,bit) (((uint32_t)reg>>bit) & 0x01)
#define BIT_EVEN_ALL (0x00ffffff)

static EventGroupHandle_t knob_even_ = NULL;
static knob_handle_t s_knob = 0;
static bool encoder_active = false;
static int8_t encoderValue = 10;
static void (*value_changed_cb)(int8_t) = NULL;

static void _knob_left_cb(void *arg, void *data)
{
  // Always wake/reset display timer on any knob activity
  display_activity_detected();
  
  if (!encoder_active) return;  // Ignore for value changes if inactive
  
  uint8_t eventBits_ = 0;
  SET_BIT(eventBits_,0);
  xEventGroupSetBits(knob_even_,eventBits_);
}

static void _knob_right_cb(void *arg, void *data)
{
  // Always wake/reset display timer on any knob activity
  display_activity_detected();
  
  if (!encoder_active) return;  // Ignore for value changes if inactive
  
  uint8_t eventBits_ = 0;
  SET_BIT(eventBits_,1);
  xEventGroupSetBits(knob_even_,eventBits_);
}

static void user_encoder_loop_task(void *arg)
{
  for(;;)
  {
    EventBits_t even = xEventGroupWaitBits(knob_even_,BIT_EVEN_ALL,pdTRUE,pdFALSE,pdMS_TO_TICKS(5000));
    
    if(READ_BIT(even,0))
    {
      // Notify LVGL that value changed
      if (encoder_active && value_changed_cb) {
        knob_set_value(encoderValue - 5);
        value_changed_cb(encoderValue);
      }
    }
    if(READ_BIT(even,1))
    {
      // Notify LVGL that value changed
      if (encoder_active && value_changed_cb) {
        knob_set_value(encoderValue + 5);
        value_changed_cb(encoderValue);
      }
    }
  }
}

void knob_init(void)
{
  // Create event group
  knob_even_ = xEventGroupCreate();
  
  // Create knob configuration
  knob_config_t cfg = 
  {
    .gpio_encoder_a = EXAMPLE_ENCODER_ECA_PIN,
    .gpio_encoder_b = EXAMPLE_ENCODER_ECB_PIN,
  };
  
  // Create knob instance
  s_knob = iot_knob_create(&cfg);
  if(NULL == s_knob)
  {
    ESP_LOGE(TAG, "knob create failed");
    return;
  }
  
  // Register callbacks
  ESP_ERROR_CHECK(iot_knob_register_cb(s_knob, KNOB_LEFT, _knob_left_cb, NULL));
  ESP_ERROR_CHECK(iot_knob_register_cb(s_knob, KNOB_RIGHT, _knob_right_cb, NULL));
  
  // Create encoder task
  xTaskCreate(user_encoder_loop_task, "user_encoder_loop_task", 3000, NULL, 2, NULL);
  
  // Start inactive
  encoder_active = false;
  
  #ifdef KASA_ENABLE_DEBUG_PRINT
    ESP_LOGI(TAG, "Encoder initialized (inactive)");
  #endif
}

void knob_activate(void)
{
  encoder_active = true;
  
  #ifdef KASA_ENABLE_DEBUG_PRINT
    ESP_LOGI(TAG, "Encoder activated");
  #endif
}

void knob_deactivate(void)
{
  encoder_active = false;

  #ifdef KASA_ENABLE_DEBUG_PRINT
    ESP_LOGI(TAG, "Encoder deactivated");
  #endif
}

bool knob_is_active(void)
{
  return encoder_active;
}

int8_t knob_get_value(void)
{
  return encoderValue;
}

void knob_set_value(int8_t value)
{
  if (value < 0) value = 0;
  if (value > 100) value = 100;
  encoderValue = value;

  #ifdef KASA_ENABLE_DEBUG_PRINT
    ESP_LOGI(TAG, "Encoder Value:%d", encoderValue);
  #endif
  
}

void knob_set_value_changed_callback(void (*callback)(int8_t))
{
  value_changed_cb = callback;
}