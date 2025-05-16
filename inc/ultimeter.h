/*
 * ultimeter.h
 *
 *  Created on: 13 may 2025
 *      Author: Ludo
 */

#ifndef __ULTIMETER_H__
#define __ULTIMETER_H__

#ifndef ULTIMETER_DRIVER_DISABLE_FLAGS_FILE
#include "ultimeter_driver_flags.h"
#endif
#include "error.h"
#include "maths.h"
#include "types.h"

/*** ULTIMETER structures ***/

/*!******************************************************************
 * \enum ULTIMETER_status_t
 * \brief ULTIMETER driver error codes.
 *******************************************************************/
typedef enum {
    // Driver errors.
    ULTIMETER_SUCCESS = 0,
    ULTIMETER_ERROR_NULL_PARAMETER,
    ULTIMETER_ERROR_RESISTOR_DIVIDER_RATIO,
    // Low level drivers errors.
    ULTIMETER_ERROR_BASE_TIMER = ERROR_BASE_STEP,
    ULTIMETER_ERROR_BASE_MATH = (ULTIMETER_ERROR_BASE_TIMER + ULTIMETER_DRIVER_TIMER_ERROR_BASE_LAST),
    // Last base value.
    ULTIMETER_ERROR_BASE_LAST = (ULTIMETER_ERROR_BASE_MATH + MATH_ERROR_BASE_LAST),
} ULTIMETER_status_t;

#ifndef ULTIMETER_DRIVER_DISABLE

/*!******************************************************************
 * \enum ULTIMETER_wind_direction_status_t
 * \brief ULTIMETER driver wind direction data status.
 *******************************************************************/
typedef enum {
    ULTIMETER_WIND_DIRECTION_STATUS_AVAILABLE = 0,
    ULTIMETER_WIND_DIRECTION_STATUS_UNDEFINED,
    ULTIMETER_WIND_DIRECTION_STATUS_LAST
} ULTIMETER_wind_direction_status_t;

/*!******************************************************************
 * \fn ULTIMETER_process_cb_t
 * \brief ULTIMETER driver process callback.
 *******************************************************************/
typedef void (*ULTIMETER_process_cb_t)(void);

/*** ULTIMETER functions ***/

/*!******************************************************************
 * \fn ULTIMETER_status_t ULTIMETER_init(ULTIMETER_process_cb_t process_callback)
 * \brief Init ULTIMETER driver.
 * \param[in]   process_callback: Function which will be called when the ULTIMETER driver has to be processed.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
ULTIMETER_status_t ULTIMETER_init(ULTIMETER_process_cb_t process_callback);

/*!******************************************************************
 * \fn ULTIMETER_status_t ULTIMETER_de_init(void)
 * \brief Release ULTIMETER driver.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
ULTIMETER_status_t ULTIMETER_de_init(void);

/*!******************************************************************
 * \fn ULTIMETER_status_t ULTIMETER_set_wind_measurement(uint8_t enable)
 * \brief Control wind speed and direction measurements.
 * \param[in]   enable: Disable (0) or enable (otherwise) wind speed and direction measurements.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
ULTIMETER_status_t ULTIMETER_set_wind_measurement(uint8_t enable);

/*!******************************************************************
 * \fn ULTIMETER_status_t ULTIMETER_process(void)
 * \brief ULTIMETER driver process function.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
ULTIMETER_status_t ULTIMETER_process(void);

/*!******************************************************************
 * \fn ULTIMETER_status_t ULTIMETER_get_wind_speed(int32_t* average_speed_mh, int32_t* peak_speed_mh)
 * \brief Read wind speeds.
 * \param[in]   none
 * \param[out]  average_speed_mh: Pointer to integer that will contain the average wind speed since last reset in m/h.
 * \param[out]  peak_speed_mh: Pointer to integer that will contain the peak wind speed since last reset in m/h.
 * \retval      Function execution status.
 *******************************************************************/
ULTIMETER_status_t ULTIMETER_get_wind_speed(int32_t* average_speed_mh, int32_t* peak_speed_mh);

/*!******************************************************************
 * \fn ULTIMETER_status_t ULTIMETER_get_wind_direction(int32_t* average_direction_degrees, ULTIMETER_wind_direction_status_t* direction_status)
 * \brief Read wind average direction.
 * \param[in]   none
 * \param[out]  average_direction_degrees: Pointer to integer that will contain the average wind direction since last reset in degrees.
 * \param[out]  direction_status: Status of the output data.
 * \retval      Function execution status.
 *******************************************************************/
ULTIMETER_status_t ULTIMETER_get_wind_direction(int32_t* average_direction_degrees, ULTIMETER_wind_direction_status_t* direction_status);

/*!******************************************************************
 * \fn void ULTIMETER_reset_measurements(void);
 * \brief Reset wind and rainfall measurements.
 * \param[in]   none
 * \param[out]  none
 * \retval      none
 *******************************************************************/
void ULTIMETER_reset_measurements(void);

/*******************************************************************/
#define ULTIMETER_exit_error(base) { ERROR_check_exit(ultimeter_status, ULTIMETER_SUCCESS, base) }

/*******************************************************************/
#define ULTIMETER_stack_error(base) { ERROR_check_stack(ultimeter_status, ULTIMETER_SUCCESS, base) }

/*******************************************************************/
#define ULTIMETER_stack_exit_error(base, code) { ERROR_check_stack_exit(ultimeter_status, ULTIMETER_SUCCESS, base, code) }

#endif /* ULTIMETER_DRIVER_DISABLE */

#endif /* __ULTIMETER_H__ */
