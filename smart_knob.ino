#include "lcd_bsp.h"
#include "cst816.h"
#include "lcd_bl_pwm_bsp.h"
#include "lcd_config.h"
#include "knob.h"
#include <ui.h>
#include <screens.h>
#include <lvgl.h>

#include <vars.h>
#include <actions.h>

#include <WiFi.h>
#include "KasaBridge.h"
#include "KasaDevice.h"

#ifdef KASA_ENABLE_DEBUG_PRINT
#include "esp_log.h"
static const char *TAG = "APP";
#endif


const char* ssid = "International House of Potatoes";          
const char* password = "bravecow298"; 

KasaDeviceManager kasaManager;
KasaSmartBulb* bedroomLight = nullptr;
KasaSmartPlug* livingRoomLight = nullptr;

void setup()
{
  Serial.begin(115200);
  knob_init();
  Touch_Init();
  lcd_lvgl_Init();
  lcd_bl_pwm_bsp_init(LCD_PWM_MODE_50);

  wifi_init();
  kasa_device_init();
  update_ui();  
}
void loop()
{
  lv_timer_handler();
  ui_tick();
}

void wifi_init() {
  // Set up Wifi
  WiFi.mode(WIFI_STA); // Station Mode
  WiFi.begin(ssid, password);

  #ifdef KASA_ENABLE_DEBUG_PRINT
    ESP_LOGI(TAG, "Connecting to WiFi");
  #endif
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
  }

  #ifdef KASA_ENABLE_DEBUG_PRINT
    ESP_LOGI(TAG, "IP Address: %s\n",WiFi.localIP().toString().c_str());
  #endif
}

void kasa_device_init() {
  // Scan for Kasa Devices
  int found = kasaManager.scanDevices(1000);
  #ifdef KASA_ENABLE_DEBUG_PRINT
    ESP_LOGI(TAG, "Found %d devices\n", found);
  #endif

  // Bind devices to UI
  bedroomLight = kasaManager.getBulb("Wake Up Light");
  livingRoomLight = kasaManager.getPlug("Living Room Lamp");

  #ifdef KASA_ENABLE_DEBUG_PRINT
  if (bedroomLight != NULL) {
            ESP_LOGI(TAG,"  Name: %s\n", bedroomLight->alias);
            ESP_LOGI(TAG,"  Model: %s\n", bedroomLight->model);
            ESP_LOGI(TAG,"  IP: %s\n", bedroomLight->ip_address);
            ESP_LOGI(TAG,"  State: %s\n", bedroomLight->on_off ? "ON" : "OFF");
            ESP_LOGI(TAG,"  Brightness: %d%%\n", bedroomLight->brightness);
            ESP_LOGI(TAG,"  Dimmable: %s\n", bedroomLight->is_dimmable ? "Yes" : "No");
  }
  #endif
}
