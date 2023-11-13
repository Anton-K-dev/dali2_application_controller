/**
 * @copyright
 *
 * @file    dali2_l_bsp.h
 * @author  Anton K.
 * @date    10 Aug 2021
 *
 * @brief   DALI-2 Application Controller.
 *          BSP layer header file
 */

#ifndef DALI2_L_BSP_H_
#define DALI2_L_BSP_H_

#define DALI2_LOG_EN

#ifdef DALI2_LOG_EN
#define DALI2_L_BSP_LOG(...)            dali2_l_bsp_print(__VA_ARGS__)
#else
#define DALI2_L_BSP_LOG(...)
#endif

//! @brief Logical PIN state 0 or 1
typedef enum {
    DALI2_L_BSP_DPIN_STATE_0,
    DALI2_L_BSP_DPIN_STATE_1
} DALI2_L_BSP_DPIN_STATE_T;

//! @brief PIN Interrupt edge type
typedef enum {
    DALI2_L_BSP_DPIN_EDGE_FALLING,
    DALI2_L_BSP_DPIN_EDGE_RAISING
} DALI2_L_BSP_DPIN_EDGE_T;

/**@brief BSP layer initialization
 *
 * @include One-shot timer initialization for @ref dali2_l_bsp_phy_timer_start_us()
 * @include TX PIN initialization for @ref dali2_l_bsp_dpin_set()
 * @include RX PIN initialization for @ref dali2_l_bsp_rx_pin_get()
 * @include Enable interrupt on RX PIN
 */
void dali2_l_bsp_init(void);

/**@brief BSP layer deinitialization
 *
 * @include Timer stop and deinitialization
 * @include Disable interrupt on RX PIN
 * @include Deinitialize RX PIN
 * @include Deinitialize TX PIN
 */
void dali2_l_bsp_deinit(void);

//! @brief Setting pin state
void dali2_l_bsp_tx_pin_set(DALI2_L_BSP_DPIN_STATE_T data);

//! @brief Function pointer for getting pin state
DALI2_L_BSP_DPIN_STATE_T dali2_l_bsp_rx_pin_get(void);

/**@brief Timer One-shot start function
 *
 * @param[IN] us - microseconds for One-shot timer
 * @attention Use dali2_l_phy_timer_handler() inside timer callback
 */
void dali2_l_bsp_phy_timer_start_us(unsigned int us);

/**@brief Timer One-shot start function
 *
 * @param[IN] ms - milliseconds for One-shot timer
 * @attention Use dali2_l_ses_timer_handler() inside timer callback
 */
void dali2_l_bsp_ses_timer_start_ms(unsigned int ms);

/**@brief Timer One-shot start function
 *
 * @param[IN] ms - milliseconds for One-shot timer
 * @attention Use dali2_l_app_timer_handler() inside timer callback
 */
void dali2_l_bsp_app_timer_start_ms(unsigned int ms);

/**@brief Print function for logging
 */
void dali2_l_bsp_print(const char * __restrict _format, ...);

#endif /* DALI2_L_BSP_H_ */
