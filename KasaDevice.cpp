// Unified Kasa Device Library - Size Optimized
// Supports both Smart Bulbs and Smart Plugs

#include "KasaDevice.h"

#ifdef KASA_ENABLE_DEBUG_PRINT
#define TAG "KASA_LIB"
#endif

extern KasaSmartBulb* bedroomLight;

// Command strings
const char *KasaDeviceManager::get_device_info = "{\"system\":{\"get_sysinfo\":null}}";
const char *KasaDeviceManager::plug_on = "{\"system\":{\"set_relay_state\":{\"state\":1}}}";
const char *KasaDeviceManager::plug_off = "{\"system\":{\"set_relay_state\":{\"state\":0}}}";
const char *KasaDeviceManager::bulb_on = "{\"smartlife.iot.smartbulb.lightingservice\":{\"transition_light_state\":{\"on_off\":1}}}";
const char *KasaDeviceManager::bulb_off = "{\"smartlife.iot.smartbulb.lightingservice\":{\"transition_light_state\":{\"on_off\":0}}}";

uint16_t KasaDeviceManager::encrypt(const char *data, int length, uint8_t addLengthByte, char *encryped_data)
{
    uint8_t key = KASA_ENCRYPTED_KEY;
    uint8_t en_b;
    int index = 0;
    
    if (addLengthByte)
    {
        encryped_data[index++] = 0;
        encryped_data[index++] = 0;
        encryped_data[index++] = (char)(length >> 8);
        encryped_data[index++] = (char)(length & 0xFF); 
    }

    for (int i = 0; i < length; i++)
    {
        en_b = data[i] ^ key;
        encryped_data[index++] = en_b;
        key = en_b;
    }

    return index;
}

uint16_t KasaDeviceManager::decrypt(char *data, int length, char *decryped_data, int startIndex)
{
    uint8_t key = KASA_ENCRYPTED_KEY;
    uint8_t dc_b;
    int retLength = 0;
    
    for (int i = startIndex; i < length; i++)
    {
        dc_b = data[i] ^ key;
        key = data[i];
        decryped_data[retLength++] = dc_b;
    }

    return retLength;
}

void KasaDeviceManager::closeSock(int sock)
{
    if (sock != -1)
    {
        shutdown(sock, 0);
        close(sock);
    }
}

KasaDeviceManager::KasaDeviceManager()
{
    deviceFound = 0;
    for(int i = 0; i < MAX_DEVICE_ALLOW; i++)
    {
        devices[i] = NULL;
    }
}

KasaDeviceManager::~KasaDeviceManager()
{
    for(uint8_t i = 0; i < deviceFound; i++)
    {
        if(devices[i] != NULL)
        {
            delete devices[i];
            devices[i] = NULL;
        }
    }
}

int KasaDeviceManager::scanDevices(int timeoutMs)
{
    struct sockaddr_in dest_addr;
    int ret = 0;
    int boardCaseEnable = 1;
    int retValue = 0;
    int sock;
    int err = 1;
    char sendbuf[256];
    int len;
    const char *string_value;
    const char *model;

    StaticJsonDocument<768> doc;

    len = strlen(get_device_info);

    dest_addr.sin_addr.s_addr = inet_addr("255.255.255.255");
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(9999);

    // Clean up previous resources
    for(uint8_t i = 0; i < deviceFound; i++)
    {
        if(devices[i] != NULL)
        {
            delete devices[i];
            devices[i] = NULL;
        }
    }
    deviceFound = 0;

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0)
    {
        return -1;
    }

    ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &boardCaseEnable, sizeof(boardCaseEnable));
    if (ret < 0)
    {
        closeSock(sock);
        return -2;
    }

    len = KasaDeviceManager::encrypt(get_device_info, len, 0, sendbuf);
    if (len > sizeof(sendbuf))
    {
        closeSock(sock);
        return -3;
    }

    err = sendto(sock, sendbuf, len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0)
    {
        closeSock(sock);
        return -4;
    }
    
    uint8_t send_loop = 0;
    long time_out_us = (long)timeoutMs * 1000;
    
    // Set socket to non-blocking mode
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    
    while ((err > 0) && (send_loop < 2))
    {
        struct timeval tv;
        if (time_out_us >= 1000000) {
            tv.tv_sec = time_out_us / 1000000;
            tv.tv_usec = time_out_us % 1000000;
        } else {
            tv.tv_sec = 0;
            tv.tv_usec = time_out_us;
        }
        
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(sock, &rfds);

        int s = select(sock + 1, &rfds, NULL, NULL, &tv);

        if (s < 0)
        {
            if (errno == EINTR) continue;
            err = -1;
            break;
        }
        else if (s > 0)
        {
            if (FD_ISSET(sock, &rfds))
            {
                char recvbuf[1536];
                char raddr_name[32] = {0};

                struct sockaddr_storage raddr;
                socklen_t socklen = sizeof(raddr);
                
                int len = recvfrom(sock, recvbuf, sizeof(recvbuf) - 1, 0,
                                   (struct sockaddr *)&raddr, &socklen);
                if (len < 0)
                {
                    err = -1;
                    break;
                }
                else
                {
                    len = KasaDeviceManager::decrypt(recvbuf, len, recvbuf, 0);
                }

                if (raddr.ss_family == PF_INET)
                {
                    inet_ntoa_r(((struct sockaddr_in *)&raddr)->sin_addr,
                                raddr_name, sizeof(raddr_name) - 1);
                }

                recvbuf[len] = 0;

                if (len > 400)
                {
                    DeserializationError error = deserializeJson(doc, recvbuf, len);

                    if (!error)
                    {
                        JsonObject get_sysinfo = doc["system"]["get_sysinfo"];
                        string_value = get_sysinfo["alias"];
                        model = get_sysinfo["model"];

                        // Check if valid Kasa device
                        bool isPlug = isPlugModel(model);
                        bool isBulb = isBulbModel(model);
                        
                        if (!isPlug && !isBulb) continue;
                        
                        // Check for duplicate device
                        if (isContainDevice(string_value) == -1)
                        {
                            if (deviceFound < MAX_DEVICE_ALLOW)
                            {
                                // Create appropriate device type
                                if (isPlug)
                                {
                                    devices[deviceFound] = new KasaSmartPlug(string_value, raddr_name);
                                }
                                else if (isBulb)
                                {
                                    KasaSmartBulb* bulb = new KasaSmartBulb(string_value, raddr_name);
                                    
                                    // Parse bulb-specific info
                                    JsonObject light_state = doc["smartlife.iot.smartbulb.lightingservice"]["get_light_state"];
                                    if (!light_state.isNull())
                                    {
                                        bulb->brightness = light_state["brightness"];
                                        bulb->color_temp = light_state["color_temp"];
                                        bulb->is_dimmable = light_state["is_dimmable"];
                                        #ifdef KASA_ENABLE_COLOR_TEMP_SUPPORT
                                        bulb->is_variable_color_temp = light_state["is_variable_color_temp"];
                                        #endif
                                    }
                                    
                                    devices[deviceFound] = bulb;
                                }
                                
                                // Common properties
                                strncpy(devices[deviceFound]->model, model, 15);
                                devices[deviceFound]->model[15] = '\0';
                                
                                // Get on/off state
                                if (isPlug)
                                {
                                    devices[deviceFound]->on_off = get_sysinfo["relay_state"];
                                }
                                else if (isBulb)
                                {
                                    JsonObject light_state = doc["smartlife.iot.smartbulb.lightingservice"]["get_light_state"];
                                    if (!light_state.isNull())
                                    {
                                        devices[deviceFound]->on_off = light_state["on_off"];
                                    }
                                    else
                                    {
                                        devices[deviceFound]->on_off = get_sysinfo["light_state"]["on_off"];
                                    }
                                }
                                
                                deviceFound++;
                            }
                        }
                        else
                        {
                            // Update existing device IP
                            KasaDevice *device = getDevice(string_value);
                            if(device != NULL)
                            {
                                device->updateIPAddress(raddr_name);
                            }
                        }
                    }
                }
            }
        }
        else if (s == 0)
        {
            send_loop++;
            if (send_loop >= 2) break;

            err = sendto(sock, sendbuf, len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
            if (err < 0)
            {
                retValue = -1;
                closeSock(sock);
                return retValue;
            }
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    
    closeSock(sock);
    return deviceFound;
}

int KasaDeviceManager::isContainDevice(const char *name)
{
    if (deviceFound == 0) return -1;
        
    for (uint8_t i = 0; i < deviceFound; i++)
    {
        if (devices[i] != NULL && strcmp(name, devices[i]->alias) == 0)
            return i;
    }

    return -1;
}

SemaphoreHandle_t KasaDevice::mutex = xSemaphoreCreateMutex();

KasaDevice* KasaDeviceManager::getDeviceByIndex(int index)
{
    if (index < 0 || index >= deviceFound)
        return NULL;
    return devices[index];
}

KasaDevice* KasaDeviceManager::getDevice(const char *alias_name)
{
    for (uint8_t i = 0; i < deviceFound; i++)
    {
        if (devices[i] != NULL && strcmp(alias_name, devices[i]->alias) == 0)
            return devices[i];
    }
    return NULL;
}

KasaSmartPlug* KasaDeviceManager::getPlug(const char *alias_name)
{
    KasaDevice* device = getDevice(alias_name);
    if (device != NULL && device->device_type == KASA_DEVICE_PLUG)
    {
        return static_cast<KasaSmartPlug*>(device);
    }
    return NULL;
}

KasaSmartBulb* KasaDeviceManager::getBulb(const char *alias_name)
{
    KasaDevice* device = getDevice(alias_name);
    if (device != NULL && device->device_type == KASA_DEVICE_BULB)
    {
        return static_cast<KasaSmartBulb*>(device);
    }
    return NULL;
}

int KasaDeviceManager::refreshAllDevices()
{
    int successCount = 0;
    
    for (uint8_t i = 0; i < deviceFound; i++)
    {
        if (devices[i] != NULL)
        {
            int result = devices[i]->queryInfo();
            if (result > 0)
            {
                successCount++;
            }
        }
    }
    
    return successCount;
}

void KasaDevice::sendCommand(const char *cmd)
{
    int err;
    char sendbuf[512];
    xSemaphoreTake(mutex, portMAX_DELAY);
    openSock();
    int len = KasaDeviceManager::encrypt(cmd, strlen(cmd), 1, sendbuf);
    if (sock > 0)
    {
        err = send(sock, sendbuf, len, 0);
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
    closeSock();
    xSemaphoreGive(mutex);
}

int KasaDevice::query(const char *cmd, char *buffer, int bufferLength, long timeout)
{
    int sendLen;
    int recvLen = 0;
    xSemaphoreTake(mutex, portMAX_DELAY);
    
    openSock();
    sendLen = KasaDeviceManager::encrypt(cmd, strlen(cmd), 1, buffer);
    if (sock > 0)
    {
        send(sock, buffer, sendLen, 0);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    else
    {
        closeSock();
        xSemaphoreGive(mutex);
        return 0;
    }

    struct timeval tv = {
        .tv_sec = 0,
        .tv_usec = timeout,
    };
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(sock, &rfds);

    int s = select(sock + 1, &rfds, NULL, NULL, &tv);
    if (s < 0)
    {
        closeSock();
        xSemaphoreGive(mutex);
        return 0;
    }
    else if (s > 0)
    {
        if (FD_ISSET(sock, &rfds))
        {
            recvLen = recv(sock, buffer, bufferLength, 0);
            if (recvLen > 0)
            {
                recvLen = KasaDeviceManager::decrypt(buffer, recvLen, buffer, 4);
            }
        }
    }

    closeSock();
    xSemaphoreGive(mutex);
    return recvLen;
}

// ===== PLUG IMPLEMENTATION =====

int KasaSmartPlug::queryInfo()
{
    char buffer[1024];
    int recvLen = query(KasaDeviceManager::get_device_info, buffer, 1024, 300000);

    if (recvLen > 200)
    {
        xSemaphoreTake(mutex, portMAX_DELAY);
        DeserializationError error = deserializeJson(doc, buffer, recvLen);

        if (!error)
        {
            JsonObject get_sysinfo = doc["system"]["get_sysinfo"];
            err_code = get_sysinfo["err_code"];
            strcpy(alias, get_sysinfo["alias"]);
            on_off = get_sysinfo["relay_state"];
        }
        xSemaphoreGive(mutex);
    }

    return recvLen;
}

void KasaSmartPlug::setOnOff(uint8_t state)
{
    if (state > 0)
    {
        sendCommand(KasaDeviceManager::plug_on);
        on_off = 1;
    }
    else
    {
        sendCommand(KasaDeviceManager::plug_off);
        on_off = 0;
    }
}

// ===== BULB IMPLEMENTATION =====

int KasaSmartBulb::queryInfo()
{
    char buffer[1536];
    const char *bulb_query = "{\"system\":{\"get_sysinfo\":null},\"smartlife.iot.smartbulb.lightingservice\":{\"get_light_state\":null}}";
    int recvLen = query(bulb_query, buffer, 1536, 300000);

    if (recvLen > 400)
    {
        xSemaphoreTake(mutex, portMAX_DELAY);
        DeserializationError error = deserializeJson(doc, buffer, recvLen);

        if (!error)
        {
            JsonObject get_sysinfo = doc["system"]["get_sysinfo"];
            err_code = get_sysinfo["err_code"];
            strcpy(alias, get_sysinfo["alias"]);
            
            JsonObject light_state = doc["smartlife.iot.smartbulb.lightingservice"]["get_light_state"];
            if (!light_state.isNull())
            {
                on_off = light_state["on_off"];
                brightness = light_state["brightness"];
                color_temp = light_state["color_temp"];
                is_dimmable = light_state["is_dimmable"];
                #ifdef KASA_ENABLE_COLOR_TEMP_SUPPORT
                is_variable_color_temp = light_state["is_variable_color_temp"];
                #endif
            }
        }
        xSemaphoreGive(mutex);
    }

    return recvLen;
}

void KasaSmartBulb::setOnOff(uint8_t state)
{
    if (state > 0)
    {
        sendCommand(KasaDeviceManager::bulb_on);
        on_off = 1;
    }
    else
    {
        sendCommand(KasaDeviceManager::bulb_off);
        on_off = 0;
    }
}

void KasaSmartBulb::setBrightness(uint8_t brightness_val)
{
    if (brightness_val <= 0) {
        setOnOff(0);
        return;
    }
    if (brightness_val > 100) brightness_val = 100;
    
    char cmd[128];
    snprintf(cmd, sizeof(cmd), 
        "{\"smartlife.iot.smartbulb.lightingservice\":{\"transition_light_state\":{\"brightness\":%d}}}", 
        brightness_val);
    sendCommand(cmd);
    brightness = brightness_val;
}

#ifdef KASA_ENABLE_COLOR_TEMP_SUPPORT
void KasaSmartBulb::setColorTemp(uint16_t temp)
{
    if (temp < MIN_COLOR_TEMP) temp = MIN_COLOR_TEMP;
    if (temp > MAX_COLOR_TEMP) temp = MAX_COLOR_TEMP;
    
    char cmd[128];
    snprintf(cmd, sizeof(cmd), 
        "{\"smartlife.iot.smartbulb.lightingservice\":{\"transition_light_state\":{\"color_temp\":%d}}}", 
        temp);
    sendCommand(cmd);
    color_temp = temp;
}
#endif

#ifdef KASA_ENABLE_TRANSITION_SUPPORT
void KasaSmartBulb::setTransition(uint16_t transition_ms)
{
    char cmd[128];
    snprintf(cmd, sizeof(cmd), 
        "{\"smartlife.iot.smartbulb.lightingservice\":{\"transition_light_state\":{\"transition_period\":%d}}}", 
        transition_ms);
    sendCommand(cmd);
}
#endif