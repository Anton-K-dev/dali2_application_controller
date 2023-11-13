/**
 * @copyright
 *
 * @file    dali2_l_phy.h
 * @author  Anton K.
 * @date    19 Jul 2021
 *
 * @brief   DALI-2 Application Controller.
 *          Physical layer header file
 */

#ifndef DALI2_L_PHY_H_
#define DALI2_L_PHY_H_

#include "dali2_l_bsp.h"
#include "dali2_error.h"

#define DALI2_L_PHY_PWR_STARTUP_TIME_MS     110
#define DALI2_L_PHY_PWR_RETRY_TIME_MS       150

#define DALI2_L_PHY_FORWARD_16BIT_SIZE      16
#define DALI2_L_PHY_FORWARD_24BIT_SIZE      24
#define DALI2_L_PHY_BACKWARD_8BIT_SIZE      8

#define DALI2_L_PHY_ALLOWER_OVERHEAD_US             100     //! This overhead extended for practice case,
                                                            //! "DALI IEC 62386-101-2014" doesn't allowed this
#define DALI2_L_PHY_HALF_BIT_TIME_US_MIN            366
#define DALI2_L_PHY_HALF_BIT_TIME_US_TYP            416
#define DALI2_L_PHY_HALF_BIT_TIME_US_MAX            (466 + DALI2_L_PHY_ALLOWER_OVERHEAD_US)

#define DALI2_L_PHY_DOUBLE_HALF_BIT_TIME_US_MIN     (DALI2_L_PHY_HALF_BIT_TIME_US_MIN * 2)
#define DALI2_L_PHY_DOUBLE_HALF_BIT_TIME_US_TYP     (DALI2_L_PHY_HALF_BIT_TIME_US_TYP * 2)
#define DALI2_L_PHY_DOUBLE_HALF_BIT_TIME_US_MAX     (DALI2_L_PHY_HALF_BIT_TIME_US_MAX * 2)

#define DALI2_L_PHY_STARTUP_TIME_US             13000
#define DALI2_L_PHY_STOP_CONDITION_TIME_US      2450


//! Frame types
typedef enum {
    DALI2_L_PHY_FRAME_16BIT_FW,
    DALI2_L_PHY_FRAME_24BIT_FW,
    DALI2_L_PHY_FRAME_PROPRIETARY_FW
} DALI2_L_PHY_FRAME_T;

//! Event from Physical layer
typedef enum {
    DALI2_L_PHY_EVT_FORWARD_DONE,
    DALI2_L_PHY_EVT_BACKWARD_DONE,
    DALI2_L_PHY_EVT_START_ERROR,
    DALI2_L_PHY_EVT_STOP_ERROR,
    DALI2_L_PHY_EVT_FORWARD_ERROR,
    DALI2_L_PHY_EVT_BACKWARD_ERROR,
    DALI2_L_PHY_EVT_DATA_VIOLATION,
    DALI2_L_PHY_EVT_TIMING_VIOLATION,
    DALI2_L_PHY_EVT_SIZE_VIOLATION
} DALI2_L_PHY_EVT_T;

//! Event parameter for DALI2_L_PHY_EVT_FORWARD_DONE
typedef struct {
    unsigned int frame;
} dali2_l_phy_evt_param_forward_t;

//! Event parameter for DALI2_DALI2_L_PHY_EVT_BACKWARD_DONE
typedef struct {
    unsigned char frame;
} dali2_l_phy_evt_param_backward_t;

//! Event parameter union
typedef union {
    dali2_l_phy_evt_param_forward_t forward;
    dali2_l_phy_evt_param_backward_t backward;
} dali2_l_phy_evt_param_t;

//! @brief One-short Timer callback Handler
//! @note Produced by dali2_l_bsp_phy_timer_start_us(()
//! @attention Must have to be used!
void dali2_l_phy_timer_cb_handler(void);

//! @brief PIN interrupt callback handler
//! @note Produced by Interrupt on the data PIN
//! @attention Must have to be used!
void dali2_l_dpin_int_cb_handler(DALI2_L_BSP_DPIN_STATE_T state);

/**@brief DALI2 Physical layer Event handler function type
 *
 * @param[OUT] evt - DALI2 Event
 * @param[OUT] param - Event parameter
 */
typedef void (* dali2_l_phy_evt_func_t) (DALI2_L_PHY_EVT_T evt, dali2_l_phy_evt_param_t *param);

//! @brief Physical layer initialization
dali2_ret_t dali2_l_phy_init(dali2_l_phy_evt_func_t evt_handler);

//! Physical layer deinitialization
dali2_ret_t dali2_l_phy_deinit(void);

/**@brief DALI2 Physical layer execution frame
 *
 * @param[IN] frame_type - Forward frame type
 * @param[IN] forward_data - Forward frame data
 * @return @see dali2_ret_t
 */
dali2_ret_t dali2_l_phy_exec_frame(DALI2_L_PHY_FRAME_T frame_type, unsigned int frame_data);

#endif /* DALI2_L_PHY_H_ */
