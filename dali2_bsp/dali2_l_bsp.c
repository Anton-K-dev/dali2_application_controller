/**
 * @copyright
 *
 * @file    dali2_l_bsp.c
 * @author  Anton K.
 * @date    10 Aug 2021
 *
 * @brief   DALI-2 Application Controller.
 *          BSP layer source file
 */

#include <stdarg.h>

#include "nrf_drv_gpiote.h"
#include "nrf_drv_timer.h"
#include "app_timer.h"
#include "app_error.h"

#include "pwr_management.h"

#include "dali2_l_phy.h"
#include "dali2_l_ses.h"
#include "dali2_l_app.h"
#include "dali2_l_bsp.h"

#define NRF_LOG_MODULE_NAME DALI2
#if APP_DALI2_BSP_MODULE_LOG_ENABLED
    #define NRF_LOG_LEVEL       APP_LOG_LEVEL
    #define NRF_LOG_INFO_COLOR  APP_LOG_INFO_COLOR
    #define NRF_LOG_DEBUG_COLOR APP_LOG_DEBUG_COLOR
#else
    #define NRF_LOG_LEVEL       0
#endif
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#define DALI_L_BSP_TX_PIN           NRF_GPIO_PIN_MAP(0, 27)
#define DALI_L_BSP_RX_PIN           NRF_GPIO_PIN_MAP(0, 7)

APP_TIMER_DEF(__dali2_l_ses_timer_id);
APP_TIMER_DEF(__dali2_l_app_timer_id);
const nrf_drv_timer_t __bsp_phy_timer_us = NRF_DRV_TIMER_INSTANCE(1);

static void __dali2_l_bsp_phy_pin_int_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    if (pin == DALI_L_BSP_RX_PIN) {
        dali2_l_dpin_int_cb_handler((DALI2_L_BSP_DPIN_STATE_T) !nrfx_gpiote_in_is_set(DALI_L_BSP_RX_PIN));
    }
}

static void __dali2_l_bsp_phy_timer_int_handler(nrf_timer_event_t event_type, void *p_context)
{
    if (event_type == NRF_TIMER_EVENT_COMPARE0) {
        //! Call Physical timer callback
        dali2_l_phy_timer_cb_handler();
    }
}

static void __dali2_l_bsp_ses_timer_int_handler(void * p_context)
{
    //! Call Session timer callback
    dali2_l_ses_timer_cb_handler();
}

static void __dali2_l_bsp_app_timer_int_handler(void * p_context)
{
    //! Call Session timer callback
    dali2_l_app_timer_cb_handler();
}

//! @brief BSP layer initialization
void dali2_l_bsp_init(void)
{
    unsigned int err_code;
    nrf_drv_gpiote_in_config_t in_config = { .sense = NRF_GPIOTE_POLARITY_TOGGLE, .pull = NRF_GPIO_PIN_PULLUP,
            .is_watcher = 0, .hi_accuracy = 1, .skip_gpio_setup = 0, };
    nrf_drv_gpiote_out_config_t out_config = {
        .action     = NRF_GPIOTE_POLARITY_LOTOHI,
        .task_pin   = 0,
        .init_state = GPIOTE_CONFIG_OUTINIT_High
    };
    nrf_drv_timer_config_t timer_cfg = {
        .frequency          = NRF_TIMER_FREQ_1MHz,
        .mode               = NRF_TIMER_MODE_TIMER,
        .bit_width          = NRF_TIMER_BIT_WIDTH_16,
        .interrupt_priority = NRFX_TIMER_DEFAULT_CONFIG_IRQ_PRIORITY,
        .p_context          = NULL
    };

    pwr_management_periph_switch(PWR_PERIPH_DALI_PS, PWR_SWITCH_ON);

    //! GPIO for physical layer
    nrf_drv_gpiote_out_init(DALI_L_BSP_TX_PIN, &out_config);

    nrf_drv_gpiote_in_init(DALI_L_BSP_RX_PIN, &in_config, __dali2_l_bsp_phy_pin_int_handler);
    nrf_drv_gpiote_in_event_enable(DALI_L_BSP_RX_PIN, 1);

    //! Timer for Physical layer
    err_code = nrf_drv_timer_init(&__bsp_phy_timer_us, &timer_cfg, __dali2_l_bsp_phy_timer_int_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_timer_pause(&__bsp_phy_timer_us);
    nrf_drv_timer_enable(&__bsp_phy_timer_us);

    //! Timer for Session layer
    err_code = app_timer_create(&__dali2_l_ses_timer_id,
                                APP_TIMER_MODE_SINGLE_SHOT,
                                __dali2_l_bsp_ses_timer_int_handler);
    APP_ERROR_CHECK(err_code);

    //! Timer for Application layer
    err_code = app_timer_create(&__dali2_l_app_timer_id,
                                APP_TIMER_MODE_SINGLE_SHOT,
                                __dali2_l_bsp_app_timer_int_handler);
    APP_ERROR_CHECK(err_code);

    return;
}

//! @brief BSP layer deinitialization
void dali2_l_bsp_deinit(void)
{
    nrf_drv_gpiote_in_event_disable(DALI_L_BSP_RX_PIN);
    nrf_drv_timer_uninit(&__bsp_phy_timer_us);
    return;
}

//! @brief Setting Data PIN state
void dali2_l_bsp_tx_pin_set(DALI2_L_BSP_DPIN_STATE_T data)
{
    if (data == DALI2_L_BSP_DPIN_STATE_0) {
        nrfx_gpiote_out_set(DALI_L_BSP_TX_PIN);
    } else {
        nrfx_gpiote_out_clear(DALI_L_BSP_TX_PIN);
    }
}

//! @brief Getting Feedback PIN state
DALI2_L_BSP_DPIN_STATE_T dali2_l_bsp_rx_pin_get(void)
{
    return (DALI2_L_BSP_DPIN_STATE_T) !nrfx_gpiote_in_is_set(DALI_L_BSP_RX_PIN);
}

/**@brief Timer One-shot start function
 *
 * @param[IN] us - microseconds for One-shot timer
 * @attention Use dali2_l_phy_timer_handler() inside timer callback
 */
void dali2_l_bsp_phy_timer_start_us(unsigned int us)
{
    unsigned int time_ticks;

    nrf_drv_timer_pause(&__bsp_phy_timer_us);
    time_ticks = nrf_drv_timer_us_to_ticks(&__bsp_phy_timer_us, us);
    nrf_drv_timer_extended_compare(
         &__bsp_phy_timer_us, NRF_TIMER_CC_CHANNEL0, time_ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, 1);
    nrf_drv_timer_clear(&__bsp_phy_timer_us);
    nrfx_timer_resume(&__bsp_phy_timer_us);
}

/**@brief Timer One-shot start function
 *
 * @param[IN] ms - milliseconds for One-shot timer
 * @attention Use dali2_l_ses_timer_handler() inside timer callback
 */
void dali2_l_bsp_ses_timer_start_ms(unsigned int ms)
{
    app_timer_stop(__dali2_l_ses_timer_id);
    app_timer_start(__dali2_l_ses_timer_id, APP_TIMER_TICKS(ms), NULL);
}

/**@brief Timer One-shot start function
 *
 * @param[IN] ms - milliseconds for One-shot timer
 * @attention Use dali2_l_app_timer_handler() inside timer callback
 */
void dali2_l_bsp_app_timer_start_ms(unsigned int ms)
{
    app_timer_stop(__dali2_l_app_timer_id);
    app_timer_start(__dali2_l_app_timer_id, APP_TIMER_TICKS(ms), NULL);
}

/**@brief Print function
 */
void dali2_l_bsp_print(const char * _format, ...)
{
    static va_list args;
    va_start(args, _format);
    NRF_LOG_INFO(_format, va_arg(args, va_list*));
    va_end(args);
}

