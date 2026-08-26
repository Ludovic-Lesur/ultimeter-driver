/*
 * ultimeter.c
 *
 *  Created on: 13 may 2025
 *      Author: Ludo
 */

#include "ultimeter.h"

#ifndef ULTIMETER_DRIVER_DISABLE_FLAGS_FILE
#include "ultimeter_driver_flags.h"
#endif
#include "error.h"
#include "maths.h"
#include "ultimeter_hw.h"
#include "types.h"

#ifndef ULTIMETER_DRIVER_DISABLE

/*** ULTIMETER local macros ***/

#define ULTIMETER_WIND_SPEED_1HZ_TO_TENTH_KMH       54
#define ULTIMETER_WIND_SPEED_ERROR_VALUE            0xFFFFFFFF

#define ULTIMETER_WIND_DIRECTION_SCALING_THRESHOLD  1000000000

#define ULTIMETER_WIND_WATCHDOG_SECONDS             12

/*** ULTIMETER local structures ***/

/*******************************************************************/
typedef union {
    uint8_t all;
    struct {
        unsigned wind_direction_irq :1;
        unsigned wind_measurement_enable :1;
    } __attribute__((scalar_storage_order("big-endian"))) __attribute__((packed));
} ULTIMETER_flags_t;

/*******************************************************************/
typedef struct {
    // State machine.
    ULTIMETER_flags_t flags;
    uint8_t wind_watchdog_seconds_count;
    // Wind speed.
    uint32_t wind_speed_counter_start;
    uint32_t wind_speed_counter_stop;
    uint32_t wind_speed_tenth_kmh_sum;
    uint32_t wind_speed_tenth_kmh_peak;
    uint32_t wind_speed_tenth_kmh_last;
    uint32_t wind_speed_data_count;
    // Wind direction.
    uint32_t wind_direction_counter;
    int32_t wind_direction_trend_point_x;
    int32_t wind_direction_trend_point_y;
} ULTIMETER_context_t;

/*** ULTIMETER local global variables ***/

static volatile ULTIMETER_context_t ultimeter_ctx;

/*** ULTIMETER local functions ***/

/*******************************************************************/
static void _ULTIMETER_reset_counters(void) {
    // Watchdog.
    ultimeter_ctx.wind_watchdog_seconds_count = 0;
    // Wind speed.
    ultimeter_ctx.wind_speed_counter_start = 0xFFFFFFFF;
    ultimeter_ctx.wind_speed_counter_stop = 0;
    ultimeter_ctx.wind_speed_tenth_kmh_last = 0;
    // Wind direction.
    ultimeter_ctx.wind_direction_counter = 0;
}

/*******************************************************************/
static void _ULTIMETER_wind_speed_edge_callback(void) {
    // Local variables.
    uint32_t wind_speed_counter_delta = 0;
    uint32_t wind_speed_tenth_kmh = 0;
    uint32_t wind_direction_degrees = 0;
    // Check if measurements are enabled.
    if (ultimeter_ctx.flags.wind_measurement_enable != 0) {
        // Clear watchdog.
        ultimeter_ctx.wind_watchdog_seconds_count = 0;
        // Capture period.
        ultimeter_ctx.wind_speed_counter_stop = ULTIMETER_HW_timer_get_counter();
        // Check counters.
        if (ultimeter_ctx.wind_speed_counter_start < ultimeter_ctx.wind_speed_counter_stop) {
            // Compute delta.
            wind_speed_counter_delta = (ultimeter_ctx.wind_speed_counter_stop - ultimeter_ctx.wind_speed_counter_start);
            // Compute speed.
            wind_speed_tenth_kmh = (ULTIMETER_WIND_SPEED_1HZ_TO_TENTH_KMH * 1000000) / (wind_speed_counter_delta * ULTIMETER_DRIVER_TIMER_STEP_US);
            // Update peak value.
            if (wind_speed_tenth_kmh > ultimeter_ctx.wind_speed_tenth_kmh_peak) {
                ultimeter_ctx.wind_speed_tenth_kmh_peak = wind_speed_tenth_kmh;
            }
            // Update last value.
            ultimeter_ctx.wind_speed_tenth_kmh_last = wind_speed_tenth_kmh;
            // Check if direction IRQ occurred between the two last speed IRQ.
            if ((ultimeter_ctx.flags.wind_direction_irq != 0) &&
                (ultimeter_ctx.wind_direction_counter >= ultimeter_ctx.wind_speed_counter_start) &&
                (ultimeter_ctx.wind_direction_counter <= ultimeter_ctx.wind_speed_counter_stop)) {
                // Compute wind direction.
                wind_direction_degrees = ((((ultimeter_ctx.wind_direction_counter - ultimeter_ctx.wind_speed_counter_start) * MATH_2_PI_DEGREES) / (wind_speed_counter_delta)) % MATH_2_PI_DEGREES);
                // Compute direction only if there is wind.
                if (wind_speed_tenth_kmh > 0) {
                    // Add new wind direction vector weighted by speed.
                    ultimeter_ctx.wind_direction_trend_point_x += (((int32_t) wind_speed_tenth_kmh) * ((int32_t) MATH_COS_TABLE[wind_direction_degrees]));
                    ultimeter_ctx.wind_direction_trend_point_y += (((int32_t) wind_speed_tenth_kmh) * ((int32_t) MATH_SIN_TABLE[wind_direction_degrees]));
                    // Scaling to avoid overflow.
                    if ((ultimeter_ctx.wind_direction_trend_point_x > (+ULTIMETER_WIND_DIRECTION_SCALING_THRESHOLD)) ||
                        (ultimeter_ctx.wind_direction_trend_point_x < (-ULTIMETER_WIND_DIRECTION_SCALING_THRESHOLD)) ||
                        (ultimeter_ctx.wind_direction_trend_point_y > (+ULTIMETER_WIND_DIRECTION_SCALING_THRESHOLD)) ||
                        (ultimeter_ctx.wind_direction_trend_point_y < (-ULTIMETER_WIND_DIRECTION_SCALING_THRESHOLD)))
                    {
                        ultimeter_ctx.wind_direction_trend_point_x = (ultimeter_ctx.wind_direction_trend_point_x / 2);
                        ultimeter_ctx.wind_direction_trend_point_y = (ultimeter_ctx.wind_direction_trend_point_y / 2);
                    }
                }
            }
        }
        // Start new period.
        ultimeter_ctx.flags.wind_direction_irq = 0;
        ultimeter_ctx.wind_speed_counter_start = ultimeter_ctx.wind_speed_counter_stop;
    }
}

/*******************************************************************/
static void _ULTIMETER_wind_direction_edge_callback(void) {
    // Check if measurements are enabled.
    if (ultimeter_ctx.flags.wind_measurement_enable != 0) {
        // Set flag and store counter.
        ultimeter_ctx.flags.wind_direction_irq = 1;
        ultimeter_ctx.wind_direction_counter = ULTIMETER_HW_timer_get_counter();
    }
}

/*******************************************************************/
static void _ULTIMETER_tick_second_callback(void) {
    // Check if measurements are enabled.
    if (ultimeter_ctx.flags.wind_measurement_enable != 0) {
        // Manage watchdog timer.
        ultimeter_ctx.wind_watchdog_seconds_count++;
        // Check period.
        if (ultimeter_ctx.wind_watchdog_seconds_count >= ULTIMETER_WIND_WATCHDOG_SECONDS) {
            // Reset counters.
            _ULTIMETER_reset_counters();
        }
        // Compute average wind speed.
        if (ultimeter_ctx.wind_speed_tenth_kmh_last != ULTIMETER_WIND_SPEED_ERROR_VALUE) {
            // Update average value.
            ultimeter_ctx.wind_speed_tenth_kmh_sum += ultimeter_ctx.wind_speed_tenth_kmh_last;
            ultimeter_ctx.wind_speed_data_count++;
            // Reset last value to error.
            ultimeter_ctx.wind_speed_tenth_kmh_last = ULTIMETER_WIND_SPEED_ERROR_VALUE;
        }
    }
}

/*** ULTIMETER functions ***/

/*******************************************************************/
ULTIMETER_status_t ULTIMETER_init(void) {
    // Local variables.
    ULTIMETER_status_t status = ULTIMETER_SUCCESS;
    ULTIMETER_HW_configuration_t hw_config;
    // Init context.
    ultimeter_ctx.flags.all = 0;
    // Reset data.
    ULTIMETER_reset_measurements();
    // Init hardware interface.
    hw_config.wind_speed_edge_irq_callback = &_ULTIMETER_wind_speed_edge_callback;
    hw_config.wind_direction_edge_irq_callback = &_ULTIMETER_wind_direction_edge_callback;
    hw_config.tick_second_irq_callback = &_ULTIMETER_tick_second_callback;
    status = ULTIMETER_HW_init(&hw_config);
    if (status != ULTIMETER_SUCCESS) goto errors;
errors:
    return status;
}

/*******************************************************************/
ULTIMETER_status_t ULTIMETER_de_init(void) {
    // Local variables.
    ULTIMETER_status_t status = ULTIMETER_SUCCESS;
    // Release hardware interface.
    status = ULTIMETER_HW_de_init();
    if (status != ULTIMETER_SUCCESS) goto errors;
errors:
    return status;
}

/*******************************************************************/
void ULTIMETER_reset_measurements(void) {
    // Reset counters.
    _ULTIMETER_reset_counters();
    // Reset wind speed.
    ultimeter_ctx.wind_speed_data_count = 0;
    ultimeter_ctx.wind_speed_tenth_kmh_sum = 0;
    ultimeter_ctx.wind_speed_tenth_kmh_peak = 0;
    ultimeter_ctx.wind_speed_tenth_kmh_last = 0;
    // Reset wind direction.
    ultimeter_ctx.wind_direction_trend_point_x = 0;
    ultimeter_ctx.wind_direction_trend_point_y = 0;
}

/*******************************************************************/
ULTIMETER_status_t ULTIMETER_set_wind_measurement(uint8_t enable) {
    // Local variables.
    ULTIMETER_status_t status = ULTIMETER_SUCCESS;
    // Check enable bit.
    if ((enable == 0) && (ultimeter_ctx.flags.wind_measurement_enable != 0)) {
        // Stop timer.
        ULTIMETER_HW_timer_stop();
    }
    if ((enable != 0) && (ultimeter_ctx.flags.wind_measurement_enable == 0)) {
        // Reset counters.
        _ULTIMETER_reset_counters();
        // Start timer.
        status = ULTIMETER_HW_timer_start();
        if (status != ULTIMETER_SUCCESS) goto errors;
    }
    // Set interrupt state.
    status = ULTIMETER_HW_set_wind_speed_direction_interrupts(enable);
    if (status != ULTIMETER_SUCCESS) goto errors;
    // Update internal flag.
    ultimeter_ctx.flags.wind_measurement_enable = (enable == 0) ? 0 : 1;
errors:
    return status;
}

/*******************************************************************/
ULTIMETER_status_t ULTIMETER_get_wind_speed(int32_t* average_speed_tenth_kmh, int32_t* peak_speed_tenth_kmh) {
    // Local variables.
    ULTIMETER_status_t status = ULTIMETER_SUCCESS;
    // Check parameters.
    if ((average_speed_tenth_kmh == NULL) || (peak_speed_tenth_kmh == NULL)) {
        status = ULTIMETER_ERROR_NULL_PARAMETER;
        goto errors;
    }
    (*average_speed_tenth_kmh) = (int32_t) (ultimeter_ctx.wind_speed_tenth_kmh_sum / ultimeter_ctx.wind_speed_data_count);
    (*peak_speed_tenth_kmh) = (int32_t) (ultimeter_ctx.wind_speed_tenth_kmh_peak);
errors:
    return status;
}

/*******************************************************************/
ULTIMETER_status_t ULTIMETER_get_wind_direction(int32_t* average_direction_degrees, ULTIMETER_wind_direction_status_t* direction_status) {
    // Local variables.
    ULTIMETER_status_t status = ULTIMETER_SUCCESS;
    MATH_status_t math_status = MATH_SUCCESS;
    // Check parameters.
    if ((average_direction_degrees == NULL) || (direction_status == NULL)) {
        status = ULTIMETER_ERROR_NULL_PARAMETER;
        goto errors;
    }
    // Reset output status.
    (*direction_status) = ULTIMETER_WIND_DIRECTION_STATUS_UNDEFINED;
    // Check trend point coordinates.
    if ((ultimeter_ctx.wind_direction_trend_point_x != 0) || (ultimeter_ctx.wind_direction_trend_point_y != 0)) {
        // Compute trend point angle.
        math_status = MATH_atan2(ultimeter_ctx.wind_direction_trend_point_x, ultimeter_ctx.wind_direction_trend_point_y, average_direction_degrees);
        MATH_exit_error(ULTIMETER_ERROR_BASE_MATH);
        // Update output status.
        (*direction_status) = ULTIMETER_WIND_DIRECTION_STATUS_AVAILABLE;
    }
errors:
    return status;
}

#endif /* ULTIMETER_DRIVER_DISABLE */
