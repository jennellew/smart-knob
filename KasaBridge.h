#ifndef KASA_BRIDGE_H
#define KASA_BRIDGE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations for C compatibility
typedef struct KasaDeviceManager KasaDeviceManager;
typedef struct KasaDevice KasaDevice;
typedef struct KasaSmartPlug KasaSmartPlug;
typedef struct KasaSmartBulb KasaSmartBulb;

extern KasaSmartBulb* bedroomLight;
extern KasaSmartPlug* livingRoomLight;
// extern KasaDeviceManager kasaManager;

/**
 * @brief Initialize KasaDeviceManager
 * 
 * @param manager Pointer to bulb device
 */
// void kasaInitBridge(KasaDeviceManager* manager);

/**
 * @brief Get the latest state from all devices
 * 
 * @return recvLength of query or -1 or devices are NULL
 */
// int kasaRefreshAllDevices();

/**
 * @brief Get the latest state from a bulb
 * 
 * @param bulb Pointer to bulb device
 * @return recvLength of query or -1 if bulb is NULL
 */
int kasaRefreshInfoBulb(KasaSmartBulb* bulb);

/**
 * @brief Get the latest state from a plug
 * 
 * @param plug Pointer to plug device
 * @return recvLength of query or -1 if plug is NULL
 */
int kasaRefreshInfoPlug(KasaSmartPlug* plug);

/**
 * @brief Set on/off state for a Kasa bulb
 * 
 * @param bulb Pointer to device
 * @param state 1 = on, 0 = off
 */
void kasaSetOnOffStateBulb(KasaSmartBulb* bulb, uint8_t state);

/**
 * @brief Set on/off state for a plug
 * 
 * @param plug Pointer to plug device
 * @param state 1 = on, 0 = off
 */
void kasaSetOnOffStatePlug(KasaSmartPlug* plug, uint8_t state);

/**
 * @brief Set brightness for a bulb
 * 
 * @param bulb Pointer to bulb device
 * @param value Brightness 0-100
 */
void kasaSetBrightness(KasaSmartBulb* bulb, int value);

/**
 * @brief Get current brightness of a bulb
 * 
 * @param bulb Pointer to bulb device
 * @return Brightness 0-100, or -1 if bulb is NULL
 */
int kasaGetBrightness(KasaSmartBulb* bulb);

/**
 * @brief Get current on/off state of a Kasa bulb
 * 
 * @param bulb Pointer to bulb
 * @return 1 = on, 0 = off, -1 if bulb is NULL
 */
int kasaGetOnOffStateBulb(KasaSmartBulb* bulb);

/**
 * @brief Get current on/off state of a plug
 * 
 * @param plug Pointer to plug device
 * @return 1 = on, 0 = off, -1 if plug is NULL
 */
int kasaGetOnOffStatePlug(KasaSmartPlug* plug);

#ifdef __cplusplus
}
#endif

#endif // KASA_BRIDGE_H