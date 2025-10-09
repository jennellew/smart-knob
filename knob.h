#ifndef ENCODER_APP_H
#define ENCODER_APP_H

#include <stdint.h>
#include <stdbool.h>

// #define KASA_ENABLE_DEBUG_PRINT

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the rotary encoder application
 * 
 * This function sets up the encoder hardware, creates event groups,
 * registers callbacks, and starts the encoder processing task.
 * The encoder starts in an inactive state.
 */
void knob_init(void);

/**
 * @brief Activate the encoder for processing input
 * 
 * When active, encoder input will update the volume/brightness value
 */
void knob_activate(void);

/**
 * @brief Deactivate the encoder to ignore input
 * 
 * When inactive, encoder input is ignored but hardware continues polling
 */
void knob_deactivate(void);

/**
 * @brief Get the current encoder state
 * 
 * @return true if encoder is active, false if inactive
 */
bool knob_is_active(void);

/**
 * @brief Get the current volume/brightness value
 * 
 * @return Current value (0-100)
 */
int8_t knob_get_value(void);

/**
 * @brief Set the volume/brightness value
 * 
 * @param value New value (0-100)
 */
void knob_set_value(int8_t value);

/**
 * @brief Register a callback for when the encoder value changes
 * 
 * @param callback Function to call when value changes, receives new value (0-100)
 */
void knob_set_value_changed_callback(void (*callback)(int8_t));

#ifdef __cplusplus
}
#endif

#endif // ENCODER_APP_H