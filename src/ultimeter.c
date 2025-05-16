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

#define ULTIMETER_WIND_SPEED_1HZ_TO_MH          5400

#define ULTIMETER_WIND_DIRECTION_ERROR_VALUE    0xFFFFFFFF

/*** ULTIMETER local structures ***/

/*******************************************************************/
typedef struct {
    // State machine.
    ULTIMETER_process_cb_t process_callback;
    volatile uint8_t tick_second_flag;
    uint8_t wind_measurement_enable_flag;
    // Wind speed.
    volatile uint32_t wind_speed_counter_start;
    volatile uint32_t wind_speed_counter_stop;
    volatile uint8_t wind_speed_seconds_count;
    volatile uint32_t wind_speed_edge_count;
    uint32_t wind_speed_data_count;
    uint32_t wind_speed_mh_average;
    uint32_t wind_speed_mh_peak;
    // Wind direction.
    volatile uint8_t wind_direction_irq_flag;
    volatile uint32_t wind_direction_counter;
    volatile uint32_t wind_direction_degrees;
    volatile uint32_t wind_direction_seconds_count;
    int32_t wind_direction_trend_point_x;
    int32_t wind_direction_trend_point_y;
} ULTIMETER_context_t;

/*** ULTIMETER local global variables ***/

static ULTIMETER_context_t ultimeter_ctx;

/*** ULTIMETER local functions ***/

/*******************************************************************/
static void _ULTIMETER_wind_speed_edge_callback(void) {
    // Local variables.
    uint32_t wind_direction_period = 0;
    uint32_t wind_direction_duty_cycle = 0;
    // Wind speed.
    ultimeter_ctx.wind_speed_edge_count++;
    // Capture period.
    ultimeter_ctx.wind_speed_counter_stop = ULTIMETER_HW_timer_get_counter();
    // Reset direction.
    ultimeter_ctx.wind_direction_degrees = ULTIMETER_WIND_DIRECTION_ERROR_VALUE;
    // Check counters.
    if ((ultimeter_ctx.wind_direction_irq_flag != 0) &&
        (ultimeter_ctx.wind_speed_counter_start < ultimeter_ctx.wind_speed_counter_stop) &&
        (ultimeter_ctx.wind_direction_counter >= ultimeter_ctx.wind_speed_counter_start) &&
        (ultimeter_ctx.wind_direction_counter <= ultimeter_ctx.wind_speed_counter_stop)) {
        // Compute wind direction.
        wind_direction_period = (ultimeter_ctx.wind_speed_counter_stop - ultimeter_ctx.wind_speed_counter_start);
        wind_direction_duty_cycle = (ultimeter_ctx.wind_direction_counter - ultimeter_ctx.wind_speed_counter_start);
        // Compute wind direction.
        ultimeter_ctx.wind_direction_degrees = (((wind_direction_duty_cycle * MATH_2_PI_DEGREES) / (wind_direction_period)) % MATH_2_PI_DEGREES);
    }
    // Start new period.
    ultimeter_ctx.wind_direction_irq_flag = 0;
    ultimeter_ctx.wind_speed_counter_start = ultimeter_ctx.wind_speed_counter_stop;
}

/*******************************************************************/
static void _ULTIMETER_wind_direction_edge_callback(void) {
    // Set flag and store counter.
    ultimeter_ctx.wind_direction_irq_flag = 1;
    ultimeter_ctx.wind_direction_counter = ULTIMETER_HW_timer_get_counter();
}

/*******************************************************************/
static void _ULTIMETER_tick_second_callback(void) {
    // Check enable flag.
    if (ultimeter_ctx.wind_measurement_enable_flag != 0) {
        // Update local flags.
        ultimeter_ctx.wind_speed_seconds_count++;
        ultimeter_ctx.wind_direction_seconds_count++;
        ultimeter_ctx.tick_second_flag = 1;
        // Ask for processing.
        if (ultimeter_ctx.process_callback != NULL) {
            ultimeter_ctx.process_callback();
        }
    }
}

/*** ULTIMETER functions ***/

/*******************************************************************/
ULTIMETER_status_t ULTIMETER_init(ULTIMETER_process_cb_t process_callback) {
    // Local variables.
    ULTIMETER_status_t status = ULTIMETER_SUCCESS;
    ULTIMETER_HW_configuration_t hw_config;
    // Reset data.
    ULTIMETER_reset_measurements();
    // Check parameter.
    if (process_callback == NULL) {
        status = ULTIMETER_ERROR_NULL_PARAMETER;
        goto errors;
    }
    // Init context.
    ultimeter_ctx.wind_measurement_enable_flag = 0;
    ultimeter_ctx.process_callback = process_callback;
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
ULTIMETER_status_t ULTIMETER_set_wind_measurement(uint8_t enable) {
    // Local variables.
    ULTIMETER_status_t status = ULTIMETER_SUCCESS;
    // Update local enable flag.
    ultimeter_ctx.wind_measurement_enable_flag = enable;
    // Check enable bit.
    if (enable == 0) {
        // Stop timer.
        ULTIMETER_HW_timer_stop();
        // Reset second counters.
        ultimeter_ctx.wind_speed_seconds_count = 0;
        ultimeter_ctx.wind_direction_seconds_count = 0;
        ultimeter_ctx.tick_second_flag = 0;
    }
    else {
        status = ULTIMETER_HW_timer_start();
        if (status != ULTIMETER_SUCCESS) goto errors;
    }
    // Set interrupt state.
    status = ULTIMETER_HW_set_wind_speed_direction_interrupts(enable);
    if (status != ULTIMETER_SUCCESS) goto errors;
errors:
    return status;
}

/*******************************************************************/
ULTIMETER_status_t ULTIMETER_process(void) {
    // Local variables.
    ULTIMETER_status_t status = ULTIMETER_SUCCESS;
    uint32_t wind_speed_mh = 0;
    // Check flag.
    if (ultimeter_ctx.tick_second_flag == 0) goto errors;
    // Clear flag.
    ultimeter_ctx.tick_second_flag = 0;
    // Update wind speed if period is reached.
    if (ultimeter_ctx.wind_speed_seconds_count >= ULTIMETER_DRIVER_WIND_SPEED_SAMPLING_TIME_SECONDS) {
        // Reset seconds counter.
        ultimeter_ctx.wind_speed_seconds_count = 0;
        // Compute new value.
        wind_speed_mh = (ultimeter_ctx.wind_speed_edge_count * ULTIMETER_WIND_SPEED_1HZ_TO_MH) / (ULTIMETER_DRIVER_WIND_SPEED_SAMPLING_TIME_SECONDS);
        ultimeter_ctx.wind_speed_edge_count = 0;
        // Update peak value if required.
        if (wind_speed_mh > ultimeter_ctx.wind_speed_mh_peak) {
            ultimeter_ctx.wind_speed_mh_peak = wind_speed_mh;
        }
        // Update average value.
        MATH_rolling_mean(ultimeter_ctx.wind_speed_mh_average, ultimeter_ctx.wind_speed_data_count, wind_speed_mh, uint32_t);
        // Compute direction only if there is wind.
        if (((wind_speed_mh / 1000) > 0) && (ultimeter_ctx.wind_direction_degrees != ULTIMETER_WIND_DIRECTION_ERROR_VALUE)) {
            // Add new wind direction vector weighted by speed.
            ultimeter_ctx.wind_direction_trend_point_x += ((int32_t) (wind_speed_mh / 1000)) * ((int32_t) MATH_COS_TABLE[ultimeter_ctx.wind_direction_degrees]);
            ultimeter_ctx.wind_direction_trend_point_y += ((int32_t) (wind_speed_mh / 1000)) * ((int32_t) MATH_SIN_TABLE[ultimeter_ctx.wind_direction_degrees]);
        }
    }
    // Update wind direction if period is reached.
    if (ultimeter_ctx.wind_direction_seconds_count >= ULTIMETER_DRIVER_WIND_DIRECTION_SAMPLING_PERIOD_SECONDS) {
        // Reset seconds counter.
        ultimeter_ctx.wind_direction_seconds_count = 0;
        // Compute direction only if there is wind.
        if (((wind_speed_mh / 1000) > 0) && (ultimeter_ctx.wind_direction_degrees != ULTIMETER_WIND_DIRECTION_ERROR_VALUE)) {
            // Add new wind direction vector weighted by speed.
            ultimeter_ctx.wind_direction_trend_point_x += ((int32_t) (wind_speed_mh / 1000)) * ((int32_t) MATH_COS_TABLE[ultimeter_ctx.wind_direction_degrees]);
            ultimeter_ctx.wind_direction_trend_point_y += ((int32_t) (wind_speed_mh / 1000)) * ((int32_t) MATH_SIN_TABLE[ultimeter_ctx.wind_direction_degrees]);
        }
    }
errors:
    return status;
}

/*******************************************************************/
ULTIMETER_status_t ULTIMETER_get_wind_speed(int32_t* average_speed_mh, int32_t* peak_speed_mh) {
    // Local variables.
    ULTIMETER_status_t status = ULTIMETER_SUCCESS;
    // Check parameters.
    if ((average_speed_mh == NULL) || (peak_speed_mh == NULL)) {
        status = ULTIMETER_ERROR_NULL_PARAMETER;
        goto errors;
    }
    (*average_speed_mh) = (int32_t) (ultimeter_ctx.wind_speed_mh_average);
    (*peak_speed_mh) = (int32_t) (ultimeter_ctx.wind_speed_mh_peak);
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

/*******************************************************************/
void ULTIMETER_reset_measurements(void) {
    // Wind speed.
    ultimeter_ctx.wind_speed_counter_start = 0;
    ultimeter_ctx.wind_speed_counter_stop = 0;
    ultimeter_ctx.wind_speed_seconds_count = 0;
    ultimeter_ctx.wind_speed_edge_count = 0;
    ultimeter_ctx.wind_speed_data_count = 0;
    ultimeter_ctx.wind_speed_mh_average = 0;
    ultimeter_ctx.wind_speed_mh_peak = 0;
    // Wind direction.
    ultimeter_ctx.wind_direction_irq_flag = 0;
    ultimeter_ctx.wind_direction_counter = 0;
    ultimeter_ctx.wind_direction_degrees = 0;
    ultimeter_ctx.wind_direction_seconds_count = 0;
    ultimeter_ctx.wind_direction_trend_point_x = 0;
    ultimeter_ctx.wind_direction_trend_point_y = 0;
}

#endif /* ULTIMETER_DRIVER_DISABLE */
