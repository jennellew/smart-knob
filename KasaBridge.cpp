#include "KasaBridge.h"
#include "KasaDevice.h"

extern "C" int kasaRefreshInfoBulb(KasaSmartBulb* bulb)
{
    if (bulb != nullptr) {
        return bulb->queryInfo();
    }
    return -1;
}

extern "C" int kasaRefreshInfoPlug(KasaSmartPlug* plug)
{
    if (plug != nullptr) {
        return plug->queryInfo();
    }
    return -1;
}

// Bulb functions
extern "C" void kasaSetOnOffStateBulb(KasaSmartBulb* bulb, uint8_t state)
{
    if (bulb != nullptr) {
      bulb->setOnOff(state);
    }
}

extern "C" void kasaSetBrightness(KasaSmartBulb* bulb, int value)
{
    if (bulb != nullptr) {
        if (value < 0) value = 0;
        if (value > 100) value = 100;

        if (bulb->on_off == 0) bulb->setOnOff(1);
        bulb->setBrightness(value);
    }
}

extern "C" int kasaGetBrightness(KasaSmartBulb* bulb)
{
    if (bulb != nullptr) {
        return bulb->brightness;
    }
    return -1;
}

extern "C" int kasaGetOnOffStateBulb(KasaSmartBulb* bulb)
{
    if (bulb != nullptr) {
        return bulb->on_off;
    }
    return -1;
}

// Plug functions
extern "C" void kasaSetOnOffStatePlug(KasaSmartPlug* plug, uint8_t state)
{
    if (plug != nullptr) {
        plug->setOnOff(state);
    }
}

extern "C" int kasaGetOnOffStatePlug(KasaSmartPlug* plug)
{
    if (plug != nullptr) {
        return plug->on_off;
    }
    return -1;
}