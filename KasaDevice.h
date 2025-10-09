/*
Unified Kasa Device Library for ESP32
Supports both Smart Bulbs and Smart Plugs with shared discovery

Based on KasaSmartPlug by Kris Jearakul
Extended for unified device control

This program is free software: you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation, either version 3 of the License,
or (at your option) any later version.
*/
#ifndef KASA_DEVICE_DOT_HPP
#define KASA_DEVICE_DOT_HPP

#include <Arduino.h>
#include <ArduinoJson.h>

// Use LWIP sockets directly instead of WiFi.h to avoid conflicts
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <lwip/inet.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#define KASA_ENCRYPTED_KEY 171
#define MAX_DEVICE_ALLOW 4

// Color temperature range (Kelvin)
#define MIN_COLOR_TEMP 2500
#define MAX_COLOR_TEMP 9000

// Size optimization flags
// #define KASA_ENABLE_COLOR_TEMP_SUPPORT // Color temperature for bulbs
#define KASA_ENABLE_TRANSITION_SUPPORT // Transition effects
// #define KASA_ENABLE_DEBUG_PRINT     // Debug serial prints

// Device types
enum KasaDeviceType {
    KASA_DEVICE_UNKNOWN = 0,
    KASA_DEVICE_PLUG = 1,
    KASA_DEVICE_BULB = 2
};

// Base device class (shared functionality)
class KasaDevice
{
protected:
    struct sockaddr_in dest_addr;
    static SemaphoreHandle_t mutex;
    StaticJsonDocument<768> doc;
    int sock;
    
    bool openSock()
    {
        int err;
        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

        fd_set fdset;
        struct timeval tv;
        int arg;
        
        if(sock < 0) return false;

        arg = fcntl(sock, F_GETFL, NULL);
        arg |= O_NONBLOCK;
        fcntl(sock, F_SETFL, O_NONBLOCK);

        err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0)
        {
            do
            {
                tv.tv_sec = 1;
                tv.tv_usec = 0;
                FD_ZERO(&fdset);
                FD_SET(sock, &fdset);

                err = select(sock + 1, NULL, &fdset, NULL, &tv);
                if(err < 0 && errno != EINTR) break;
                
                if (err == 1)
                {
                    int so_error = 0;
                    socklen_t len = sizeof so_error;
                    getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);
                    if (so_error == 0)
                    {
                        fcntl(sock, F_SETFL, arg);
                        return true;
                    } else 
                        break;
                }
            } while(1);
        }
        closeSock();
        return false;
    }
    
    void closeSock()
    {
        if (sock != -1)
        {
            shutdown(sock, 0);
            close(sock);
            sock = -1;
        }
    }
    
    void sendCommand(const char *cmd);
    int query(const char *cmd, char *buffer, int bufferLength, long timeout);

public:
    char alias[32];
    char ip_address[32];
    char model[16];
    KasaDeviceType device_type;
    
    // Common state
    uint8_t on_off;
    int8_t err_code;
    
    KasaDevice() : sock(-1), device_type(KASA_DEVICE_UNKNOWN), on_off(0), err_code(0) {}
    virtual ~KasaDevice() {}
    
    virtual int queryInfo() = 0;
    virtual void setOnOff(uint8_t state) = 0;
    
    void updateIPAddress(const char *ip)
    {
        strcpy(ip_address, ip);
        sock = -1;
        dest_addr.sin_addr.s_addr = inet_addr(ip_address);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(9999);
    }
};

// Smart Plug class
class KasaSmartPlug : public KasaDevice
{
public:
    KasaSmartPlug(const char *name, const char *ip)
    {
        strcpy(alias, name);
        updateIPAddress(ip);
        device_type = KASA_DEVICE_PLUG;
        xSemaphoreGive(mutex);
    }
    
    int queryInfo() override;
    void setOnOff(uint8_t state) override;
};

// Smart Bulb class
class KasaSmartBulb : public KasaDevice
{
public:
    uint8_t brightness;
    uint16_t color_temp;
    bool is_dimmable;
    
    #ifdef KASA_ENABLE_COLOR_TEMP_SUPPORT
    bool is_variable_color_temp;
    #endif
    
    KasaSmartBulb(const char *name, const char *ip)
    {
        strcpy(alias, name);
        updateIPAddress(ip);
        device_type = KASA_DEVICE_BULB;
        brightness = 0;
        color_temp = 0;
        is_dimmable = false;
        #ifdef KASA_ENABLE_COLOR_TEMP_SUPPORT
        is_variable_color_temp = false;
        #endif
        xSemaphoreGive(mutex);
    }
    
    int queryInfo() override;
    void setOnOff(uint8_t state) override;
    void setBrightness(uint8_t brightness);
    
    #ifdef KASA_ENABLE_COLOR_TEMP_SUPPORT
    void setColorTemp(uint16_t temp);
    #endif
    
    #ifdef KASA_ENABLE_TRANSITION_SUPPORT
    void setTransition(uint16_t transition_ms);
    #endif
};

// Device manager - handles discovery and management
class KasaDeviceManager
{
private:
    KasaDevice *devices[MAX_DEVICE_ALLOW];
    void closeSock(int sock);
    int isContainDevice(const char *name);
    inline int isStartWith(const char *prefix, const char *model)
    {
        return strncmp(prefix, model, strlen(prefix)) == 0;
    }
    inline bool isPlugModel(const char *model)
    {
        return isStartWith("HS", model) || isStartWith("KP", model) || 
               isStartWith("EP", model);
    }
    inline bool isBulbModel(const char *model)
    {
        return isStartWith("LB", model) || isStartWith("KL", model);
    }
    uint8_t deviceFound;

public:
    static const char *get_device_info;
    static const char *plug_on;
    static const char *plug_off;
    static const char *bulb_on;
    static const char *bulb_off;

    int scanDevices(int timeoutMs = 1000);
    static uint16_t encrypt(const char *data, int length, uint8_t addLengthByte, char *encryped_data);
    static uint16_t decrypt(char *data, int length, char *decryped_data, int startIndex);
    
    KasaDevice* getDevice(const char *alias_name);
    KasaDevice* getDeviceByIndex(int index);
    KasaSmartPlug* getPlug(const char *alias_name);
    KasaSmartBulb* getBulb(const char *alias_name);
    int refreshAllDevices();
    
    int getDeviceCount() { return deviceFound; }
    
    KasaDeviceManager();
    ~KasaDeviceManager();
};

#endif