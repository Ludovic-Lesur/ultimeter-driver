/*
 * ultimeter_hw.h
 *
 *  Created on: 13 may 2025
 *      Author: Ludo
 */

#ifndef __ULTIMETER_HW_H__
#define __ULTIMETER_HW_H__

#ifndef ULTIMETER_DRIVER_DISABLE_FLAGS_FILE
#include "ultimeter_driver_flags.h"
#endif
#include "ultimeter.h"
#include "types.h"

#ifndef ULTIMETER_DRIVER_DISABLE

/*** ULTIMETER HW structures ***/

/*!******************************************************************
 * \fn ULTIMETER_HW_gpio_edge_irq_cb_t
 * \brief GPIO edge interrupt callback.
 *******************************************************************/
typedef void (*ULTIMETER_HW_gpio_edge_irq_cb_t)(void);

/*!******************************************************************
 * \fn ULTIMETER_HW_tick_second_irq_cb_t
 * \brief 1 second timer interrupt callback.
 *******************************************************************/
typedef void (*ULTIMETER_HW_tick_second_irq_cb_t)(void);

/*!******************************************************************
 * \struct ULTIMETER_HW_configuration_t
 * \brief ULTIMETER hardware interface parameters.
 *******************************************************************/
typedef struct {
    ULTIMETER_HW_gpio_edge_irq_cb_t wind_speed_edge_irq_callback;
    ULTIMETER_HW_gpio_edge_irq_cb_t wind_direction_edge_irq_callback;
    ULTIMETER_HW_tick_second_irq_cb_t tick_second_irq_callback;
} ULTIMETER_HW_configuration_t;

/*** ULTIMETER HW functions ***/

/*!******************************************************************
 * \fn ULTIMETER_status_t ULTIMETER_HW_init(ULTIMETER_HW_configuration_t* configuration)
 * \brief Init ULTIMETER hardware interface.
 * \param[in]   configuration: Pointer to the hardware interface parameters structure.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
ULTIMETER_status_t ULTIMETER_HW_init(ULTIMETER_HW_configuration_t* configuration);

/*!******************************************************************
 * \fn ULTIMETER_status_t ULTIMETER_HW_de_init(void)
 * \brief Release ULTIMETER hardware interface.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
ULTIMETER_status_t ULTIMETER_HW_de_init(void);

/*!******************************************************************
 * \fn ULTIMETER_status_t ULTIMETER_HW_set_wind_speed_direction_interrupts(uint8_t enable)
 * \brief Set wind speed and direction interrupts state.
 * \param[in]   enable: Disable (0) or enable (otherwise) the wind speed and direction GPIOs interrupts.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
ULTIMETER_status_t ULTIMETER_HW_set_wind_speed_direction_interrupts(uint8_t enable);

/*!******************************************************************
 * \fn ULTIMETER_status_t ULTIMETER_HW_timer_start(void)
 * \brief Start timer.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
ULTIMETER_status_t ULTIMETER_HW_timer_start(void);

/*!******************************************************************
 * \fn ULTIMETER_status_t ULTIMETER_HW_timer_stop(void)
 * \brief Stop timer.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
ULTIMETER_status_t ULTIMETER_HW_timer_stop(void);

/*!******************************************************************
 * \fn uint32_t ULTIMETER_HW_timer_get_counter(void)
 * \brief Get timer counter.
 * \param[in]   none
 * \param[out]  none
 * \retval      Current counter value.
 *******************************************************************/
uint32_t ULTIMETER_HW_timer_get_counter(void);

#endif /* ULTIMETER_DRIVER_DISABLE */

#endif /* __ULTIMETER_HW_H__ */
