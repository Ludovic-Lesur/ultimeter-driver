/*
 * ultimeter_hw.c
 *
 *  Created on: 13 may 2025
 *      Author: Ludo
 */

#include "ultimeter_hw.h"

#ifndef ULTIMETER_DRIVER_DISABLE_FLAGS_FILE
#include "ultimeter_driver_flags.h"
#endif
#include "ultimeter.h"
#include "types.h"

#ifndef ULTIMETER_DRIVER_DISABLE

/*** ULTIMETER HW functions ***/

/*******************************************************************/
ULTIMETER_status_t __attribute__((weak)) ULTIMETER_HW_init(ULTIMETER_HW_configuration_t* configuration) {
    // Local variables.
    ULTIMETER_status_t status = ULTIMETER_SUCCESS;
    /* To be implemented */
    UNUSED(configuration);
    return status;
}

/*******************************************************************/
ULTIMETER_status_t __attribute__((weak)) ULTIMETER_HW_de_init(void) {
    // Local variables.
    ULTIMETER_status_t status = ULTIMETER_SUCCESS;
    /* To be implemented */
    return status;
}

/*******************************************************************/
ULTIMETER_status_t __attribute__((weak)) ULTIMETER_HW_set_wind_speed_direction_interrupts(uint8_t enable) {
    // Local variables.
    ULTIMETER_status_t status = ULTIMETER_SUCCESS;
    /* To be implemented */
    UNUSED(enable);
    return status;
}

/*******************************************************************/
ULTIMETER_status_t __attribute__((weak)) ULTIMETER_HW_timer_start(void) {
    // Local variables.
    ULTIMETER_status_t status = ULTIMETER_SUCCESS;
    /* To be implemented */
    return status;
}

/*******************************************************************/
ULTIMETER_status_t __attribute__((weak)) ULTIMETER_HW_timer_stop(void) {
    // Local variables.
    ULTIMETER_status_t status = ULTIMETER_SUCCESS;
    /* To be implemented */
    return status;
}
/*******************************************************************/
uint32_t __attribute__((weak)) ULTIMETER_HW_timer_get_counter(void) {
    /* To be implemented */
    return 0;
}

#endif /* ULTIMETER_DRIVER_DISABLE */
