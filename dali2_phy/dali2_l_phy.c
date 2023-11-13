/**
 * @copyright
 *
 * @file    dali2_l_phy.c
 * @author  Anton K.
 * @date    19 Jul 2021
 *
 * @brief   DALI-2 Application Controller.
 *          Physical layer source file
 */

#include <stdint.h>
#include <stddef.h>

#include "dali2_l_phy.h"

//! Physical line state
typedef enum {
    DALI2_L_PHY_STATE_STARTUP,
    DALI2_L_PHY_STATE_IDLE,
    DALI2_L_PHY_STATE_FORWARD_START,
    DALI2_L_PHY_STATE_FORWARD_FRAME,
    DALI2_L_PHY_STATE_FORWARD_STOP,
    DALI2_L_PHY_STATE_BACKWARD_START,
    DALI2_L_PHY_STATE_BACKWARD_FRAME,
    DALI2_L_PHY_STATE_BACKWARD_STOP
} DALI2_L_PHY_STATE_T;

//! Internal handle type
typedef struct {
    DALI2_L_PHY_STATE_T phy_state;

    unsigned char half_bits_left;
    DALI2_L_BSP_DPIN_STATE_T expected_dpin_state;

    dali2_l_phy_evt_func_t evt_func;
    dali2_l_phy_evt_param_t ev_param;

    unsigned char is_init:1;
} dali2_l_phy_handle_t;

dali2_l_phy_handle_t __phy_handle;

static inline void __dali2_l_fw_start(void)
{
    DALI2_L_BSP_DPIN_STATE_T dpin_state = dali2_l_bsp_rx_pin_get();

    //! Verify expected PIN state
    if (dpin_state != __phy_handle.expected_dpin_state) {
        __phy_handle.phy_state = DALI2_L_PHY_STATE_IDLE;
        __phy_handle.evt_func(DALI2_L_PHY_EVT_START_ERROR, &__phy_handle.ev_param);
        return;
    }

    //! Last start half-bit
    __phy_handle.expected_dpin_state = DALI2_L_BSP_DPIN_STATE_1;
    dali2_l_bsp_tx_pin_set(DALI2_L_BSP_DPIN_STATE_1);

    //! Going to forward frame
    __phy_handle.phy_state = DALI2_L_PHY_STATE_FORWARD_FRAME;

    //! Start timer for finishing STOP condition
    dali2_l_bsp_phy_timer_start_us(DALI2_L_PHY_HALF_BIT_TIME_US_MIN);
}

static inline void __dali2_l_fw_frame(void)
{
    DALI2_L_BSP_DPIN_STATE_T dpin_state = dali2_l_bsp_rx_pin_get();

    //! Verify expected PIN state
    if (dpin_state != __phy_handle.expected_dpin_state) {
        __phy_handle.phy_state = DALI2_L_PHY_STATE_IDLE;
        __phy_handle.evt_func(DALI2_L_PHY_EVT_FORWARD_ERROR, &__phy_handle.ev_param);
        return;
    }

    //! Forward frame completed
    if (!__phy_handle.half_bits_left) {
        //! Going stop condition
        __phy_handle.phy_state = DALI2_L_PHY_STATE_FORWARD_STOP;

        __phy_handle.expected_dpin_state = DALI2_L_BSP_DPIN_STATE_1;
        dali2_l_bsp_tx_pin_set(DALI2_L_BSP_DPIN_STATE_1);
        dali2_l_bsp_phy_timer_start_us(DALI2_L_PHY_STOP_CONDITION_TIME_US);
        return;
    } else if (__phy_handle.half_bits_left % 2) {  //! Last Half-bits processing

        if (__phy_handle.ev_param.forward.frame & (1 << (__phy_handle.half_bits_left / 2))) {
            //! Logic 1
            __phy_handle.expected_dpin_state = DALI2_L_BSP_DPIN_STATE_1;
            dali2_l_bsp_tx_pin_set(DALI2_L_BSP_DPIN_STATE_1);
        } else {
            //! Logic 0
            __phy_handle.expected_dpin_state = DALI2_L_BSP_DPIN_STATE_0;
            dali2_l_bsp_tx_pin_set(DALI2_L_BSP_DPIN_STATE_0);
        }
    } else {    //! First Half-bits processing

        if (__phy_handle.ev_param.forward.frame & (1 << (__phy_handle.half_bits_left / 2 - 1))) {
            //! Logic 1
            __phy_handle.expected_dpin_state = DALI2_L_BSP_DPIN_STATE_0;
            dali2_l_bsp_tx_pin_set(DALI2_L_BSP_DPIN_STATE_0);
        } else {
            //! Logic 0
            __phy_handle.expected_dpin_state = DALI2_L_BSP_DPIN_STATE_1;
            dali2_l_bsp_tx_pin_set(DALI2_L_BSP_DPIN_STATE_1);
        }
    }

    //! Decrement half-bit left
    if (__phy_handle.half_bits_left) {
        __phy_handle.half_bits_left--;
    }

    //! Start timer again
    dali2_l_bsp_phy_timer_start_us(DALI2_L_PHY_HALF_BIT_TIME_US_TYP);
}

static inline void __dali2_l_fw_stop(void)
{
    DALI2_L_BSP_DPIN_STATE_T dpin_state = dali2_l_bsp_rx_pin_get();

    //! Verify expected PIN state
    if (dpin_state != __phy_handle.expected_dpin_state) {
        __phy_handle.phy_state = DALI2_L_PHY_STATE_IDLE;
        __phy_handle.evt_func(DALI2_L_PHY_EVT_STOP_ERROR, &__phy_handle.ev_param);
        return;
    }

    //! Here is Forward frame DONE
    __phy_handle.phy_state = DALI2_L_PHY_STATE_IDLE;
    __phy_handle.evt_func(DALI2_L_PHY_EVT_FORWARD_DONE, &__phy_handle.ev_param);
}

static inline void __dali2_l_bw_start(void)
{
    //! Timing violation on Backward Start condition
    __phy_handle.phy_state = DALI2_L_PHY_STATE_IDLE;
    __phy_handle.evt_func(DALI2_L_PHY_EVT_TIMING_VIOLATION, &__phy_handle.ev_param);
}

static inline void __dali2_l_bw_frame(void)
{
    DALI2_L_BSP_DPIN_STATE_T dpin_state = dali2_l_bsp_rx_pin_get();

    //! If no bits left
    if (!__phy_handle.half_bits_left) {
        //! Waiting for STOP condition
        __phy_handle.phy_state = DALI2_L_PHY_STATE_BACKWARD_STOP;
        __phy_handle.expected_dpin_state = DALI2_L_BSP_DPIN_STATE_1;
        dali2_l_bsp_phy_timer_start_us(DALI2_L_PHY_STOP_CONDITION_TIME_US);
        return;
    }

    //! Decrement half-bit left
    if (--__phy_handle.half_bits_left) {
        dali2_l_bsp_phy_timer_start_us(DALI2_L_PHY_HALF_BIT_TIME_US_MAX);
    }

    if (__phy_handle.half_bits_left % 2) {  //! First half-bit
        if (dpin_state == DALI2_L_BSP_DPIN_STATE_0) {    //! "1" begins
            __phy_handle.ev_param.backward.frame |= (1 << (__phy_handle.half_bits_left / 2));
            __phy_handle.expected_dpin_state = DALI2_L_BSP_DPIN_STATE_1;
        } else {    //! "0" begins
            __phy_handle.expected_dpin_state = DALI2_L_BSP_DPIN_STATE_0;
        }
    } else {    //! Last half-bit
        //! Timing violation on Backward Frame condition
        __phy_handle.phy_state = DALI2_L_PHY_STATE_IDLE;
        __phy_handle.evt_func(DALI2_L_PHY_EVT_TIMING_VIOLATION, &__phy_handle.ev_param);
        return;
    }
}

static inline void __dali2_l_bw_stop(void)
{
    //! Here is Backward frame DONE
    __phy_handle.phy_state = DALI2_L_PHY_STATE_IDLE;
    __phy_handle.evt_func(DALI2_L_PHY_EVT_BACKWARD_DONE, &__phy_handle.ev_param);
}

dali2_ret_t dali2_l_phy_init(dali2_l_phy_evt_func_t evt_handler)
{
    dali2_ret_t dali2_ret = DALI2_RET_SUCCESS;

    //! Verify initialization done
    if (__phy_handle.is_init) {
        dali2_ret = DALI2_RET_INTERNAL_ERROR;
        goto __ret;
    }

    //! Verify Event handler
    if (!evt_handler) {
        dali2_ret = DALI2_RET_INVALID_PARAMS;
        goto __ret;
    }

    //! Initialize internal structure
    __phy_handle.evt_func = evt_handler;
    __phy_handle.phy_state = DALI2_L_PHY_STATE_STARTUP;

    //! Call BSP layer Initialization
    dali2_l_bsp_init();

    //! Idle Data PIN state
    dali2_l_bsp_tx_pin_set(DALI2_L_BSP_DPIN_STATE_1);

    dali2_l_bsp_phy_timer_start_us(DALI2_L_PHY_STARTUP_TIME_US);

__ret:
    return dali2_ret;
}

dali2_ret_t dali2_l_phy_deinit(void)
{
    dali2_ret_t dali2_ret = DALI2_RET_SUCCESS;

    //! Verify initialization done
    if (!__phy_handle.is_init) {
        dali2_ret = DALI2_RET_INTERNAL_ERROR;
        goto __ret;
    }

    //! Call BSP layer DeInitialization
    dali2_l_bsp_deinit();

    __phy_handle.is_init = 0;

__ret:
    return dali2_ret;
}

//! Bit sequence: MSP first
//! Frame: [Start bit] [N data bits] [Stop condition]
dali2_ret_t dali2_l_phy_exec_frame(DALI2_L_PHY_FRAME_T frame_type, unsigned int frame_data)
{
    dali2_ret_t dali2_ret = DALI2_RET_SUCCESS;

    //! Verify state
    if (__phy_handle.phy_state != DALI2_L_PHY_STATE_IDLE) {
        dali2_ret = DALI2_RET_BUSY; goto __ret;
    }

    //! Verify Bus for Free
    if (dali2_l_bsp_rx_pin_get() != DALI2_L_BSP_DPIN_STATE_1) {
        dali2_ret = DALI2_RET_INTERNAL_ERROR; goto __ret;
    }

    switch (frame_type) {
        case DALI2_L_PHY_FRAME_16BIT_FW:
            __phy_handle.half_bits_left = DALI2_L_PHY_FORWARD_16BIT_SIZE * 2;
            break;

        case DALI2_L_PHY_FRAME_24BIT_FW:
            __phy_handle.half_bits_left = DALI2_L_PHY_FORWARD_24BIT_SIZE * 2;
            break;

        case DALI2_L_PHY_FRAME_PROPRIETARY_FW:
        default:
            dali2_ret = DALI2_RET_NOT_SUPPORTED; goto __ret;
    }

    __phy_handle.ev_param.forward.frame = frame_data;

    //! Going to start bit
    __phy_handle.phy_state = DALI2_L_PHY_STATE_FORWARD_START;

    //! First start half-bit
    __phy_handle.expected_dpin_state = DALI2_L_BSP_DPIN_STATE_0;
    dali2_l_bsp_tx_pin_set(DALI2_L_BSP_DPIN_STATE_0);
    dali2_l_bsp_phy_timer_start_us(DALI2_L_PHY_HALF_BIT_TIME_US_MIN);

__ret:
    return dali2_ret;
}

void dali2_l_phy_timer_cb_handler(void)
{
    switch (__phy_handle.phy_state) {
        case DALI2_L_PHY_STATE_STARTUP:
            //! Initialization done now
            __phy_handle.phy_state = DALI2_L_PHY_STATE_IDLE;
            __phy_handle.is_init = 1;
            break;

        case DALI2_L_PHY_STATE_IDLE:
            break;

        case DALI2_L_PHY_STATE_FORWARD_START:
            __dali2_l_fw_start();
            break;

        case DALI2_L_PHY_STATE_FORWARD_FRAME:
            __dali2_l_fw_frame();
            break;

        case DALI2_L_PHY_STATE_FORWARD_STOP:
            __dali2_l_fw_stop();
            break;

        case DALI2_L_PHY_STATE_BACKWARD_START:
            __dali2_l_bw_start();
            break;

        case DALI2_L_PHY_STATE_BACKWARD_FRAME:
            __dali2_l_bw_frame();
            break;

        case DALI2_L_PHY_STATE_BACKWARD_STOP:
            __dali2_l_bw_stop();
            break;

        default:
            break;
    }
}

void dali2_l_dpin_int_cb_handler(DALI2_L_BSP_DPIN_STATE_T state)
{
    //! Initialization undone!
    if (!__phy_handle.is_init) return;

    switch (__phy_handle.phy_state) {
        case DALI2_L_PHY_STATE_IDLE:
            if (state == DALI2_L_BSP_DPIN_STATE_0) {

                //! First half-bit of Backward frame detected
                __phy_handle.phy_state = DALI2_L_PHY_STATE_BACKWARD_START;

                __phy_handle.expected_dpin_state = DALI2_L_BSP_DPIN_STATE_1;
                dali2_l_bsp_phy_timer_start_us(DALI2_L_PHY_HALF_BIT_TIME_US_MAX);
            } else {
                //! Data violation on Backward Start condition
                __phy_handle.phy_state = DALI2_L_PHY_STATE_IDLE;
                __phy_handle.evt_func(DALI2_L_PHY_EVT_SIZE_VIOLATION, &__phy_handle.ev_param);
            }
            break;

        case DALI2_L_PHY_STATE_BACKWARD_START:
            if (state == __phy_handle.expected_dpin_state) {

                //! Initialize Backward frame reception
                __phy_handle.phy_state = DALI2_L_PHY_STATE_BACKWARD_FRAME;
                __phy_handle.half_bits_left = DALI2_L_PHY_BACKWARD_8BIT_SIZE * 2;
                __phy_handle.ev_param.backward.frame = 0x00;

                //! Waiting for last half-bit of START condition yet
                dali2_l_bsp_phy_timer_start_us(DALI2_L_PHY_HALF_BIT_TIME_US_MAX);
            } else {
                //! Data violation on Backward Start condition
                __phy_handle.phy_state = DALI2_L_PHY_STATE_IDLE;
                __phy_handle.evt_func(DALI2_L_PHY_EVT_DATA_VIOLATION, &__phy_handle.ev_param);
            }
            break;

        case DALI2_L_PHY_STATE_BACKWARD_FRAME:
            //! STOP If no bits left
            if (!__phy_handle.half_bits_left) {
                //! Waiting for STOP condition
                __phy_handle.phy_state = DALI2_L_PHY_STATE_BACKWARD_STOP;
                __phy_handle.expected_dpin_state = DALI2_L_BSP_DPIN_STATE_1;
                dali2_l_bsp_phy_timer_start_us(DALI2_L_PHY_STOP_CONDITION_TIME_US);
                return;
            }

            //! Decrement half-bit left
            if (--__phy_handle.half_bits_left) {
                //! Start timer again
                dali2_l_bsp_phy_timer_start_us(DALI2_L_PHY_HALF_BIT_TIME_US_MAX);
            }

            if (__phy_handle.half_bits_left % 2) {    //! First half-bit
                if (state == DALI2_L_BSP_DPIN_STATE_0) {    //! "1" begins
                    __phy_handle.ev_param.backward.frame |= (1 << (__phy_handle.half_bits_left / 2));
                    __phy_handle.expected_dpin_state = DALI2_L_BSP_DPIN_STATE_1;
                } else {
                    //! "0" begins
                    __phy_handle.expected_dpin_state = DALI2_L_BSP_DPIN_STATE_0;
                }
            } else {  //! Last half-bit
                //! Verify last half-bit
                if (state != __phy_handle.expected_dpin_state) {
                    __phy_handle.phy_state = DALI2_L_PHY_STATE_IDLE;
                    __phy_handle.evt_func(DALI2_L_PHY_EVT_DATA_VIOLATION, &__phy_handle.ev_param);
                }
            }
            break;

        case DALI2_L_PHY_STATE_BACKWARD_STOP:
            //! Data size violation on backward STOP condition
            if (__phy_handle.expected_dpin_state != state) {
                __phy_handle.phy_state = DALI2_L_PHY_STATE_IDLE;
                __phy_handle.evt_func(DALI2_L_PHY_EVT_SIZE_VIOLATION, &__phy_handle.ev_param);
            }
            break;

        case DALI2_L_PHY_STATE_FORWARD_START:
        case DALI2_L_PHY_STATE_FORWARD_FRAME:
        case DALI2_L_PHY_STATE_FORWARD_STOP:
        default:
            break;
    }
}



