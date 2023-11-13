/**
 * @copyright
 *
 * @file    dali2_l_ses.c
 * @author  Anton K.
 * @date    26 Aug 2021
 *
 * @brief   DALI-2 Application Controller.
 *          Session layer source file
 */

#include "dali2_l_ses.h"

typedef enum {
    DALI2_L_SES_STATE_RDY,
    DALI2_L_SES_STATE_PROGRESS,
    DALI2_L_SES_STATE_SETTLING_TIME
} DALI2_L_SES_STATE_T;

//! Internal handle type
typedef struct {
    DALI2_L_SES_STATE_T ses_state;

    DALI2_L_PHY_FRAME_T phy_frame_type;
    dali2_l_ses_evt_func_t evt_func;
    dali2_l_ses_evt_param_t ev_param;

    unsigned char is_init:1;
    unsigned char send_twice:1;
} dali2_l_ses_handle_t;

static dali2_l_ses_handle_t __ses_handle;

static void __dali2_l_ses_phy_evt_handler(DALI2_L_PHY_EVT_T evt, dali2_l_phy_evt_param_t *param)
{
    switch (evt) {
        case DALI2_L_PHY_EVT_FORWARD_DONE:
            switch (__ses_handle.ev_param.msg) {
                case DALI2_L_SES_SEND_TWICE:
                    if (__ses_handle.send_twice) {
                        //! Settling time before sending forward frame twice
                        dali2_l_bsp_ses_timer_start_ms(DALI2_L_SES_SETTLING_TIME_FW_FW_MS_MAX);
                        break;
                    } //! Else Continue to the next case

                case DALI2_L_SES_SEND:
                    //! Here is DONE forward frame sending
                    //! Waiting Settling time for session ready state again
                    __ses_handle.ses_state = DALI2_L_SES_STATE_SETTLING_TIME;
                    dali2_l_bsp_ses_timer_start_ms(DALI2_L_SES_SETTLING_TIME_FW_FW_MS_MAX);
                    __ses_handle.evt_func(DALI2_L_SES_EVT_DONE, &__ses_handle.ev_param);
                    break;

                case DALI2_L_SES_QUERY:
                    //! Start timeout for response
                    dali2_l_bsp_ses_timer_start_ms(DALI2_L_SES_SETTLING_TIME_FW_BW_MS_MAX);
                    break;

                default:
                    break;
            }
            break;

        case DALI2_L_PHY_EVT_BACKWARD_DONE:
            //! Waiting Settling time for session ready state again
            __ses_handle.ses_state = DALI2_L_SES_STATE_SETTLING_TIME;
            dali2_l_bsp_ses_timer_start_ms(DALI2_L_SES_SETTLING_TIME_FW_FW_MS_MAX);

            if (__ses_handle.ev_param.msg == DALI2_L_SES_QUERY) {
                //! Here is backward Done
                __ses_handle.ev_param.msg_data = param->backward.frame;
                __ses_handle.evt_func(DALI2_L_SES_EVT_DONE, &__ses_handle.ev_param);
            } else {
                //! Received unexpected frame
                __ses_handle.ev_param.msg_data = param->backward.frame;
                __ses_handle.evt_func(DALI2_L_SES_EVT_DONE, &__ses_handle.ev_param);
            }
            break;

        case DALI2_L_PHY_EVT_START_ERROR:
        case DALI2_L_PHY_EVT_STOP_ERROR:
        case DALI2_L_PHY_EVT_FORWARD_ERROR:
            //! Data collision detected
            //! TODO: Here may be Collision Recovery timeout
            __ses_handle.evt_func(DALI2_L_SES_EVT_COLLISION, &__ses_handle.ev_param);
            __ses_handle.ses_state = DALI2_L_SES_STATE_RDY;
            break;

        case DALI2_L_PHY_EVT_BACKWARD_ERROR:
        case DALI2_L_PHY_EVT_DATA_VIOLATION:
        case DALI2_L_PHY_EVT_TIMING_VIOLATION:
        case DALI2_L_PHY_EVT_SIZE_VIOLATION:
            //! Here is undone backward frame or some collision during
            __ses_handle.ev_param.msg_data = param->backward.frame;
            __ses_handle.evt_func(DALI2_L_SES_EVT_UNEXPECTED_FRAME, &__ses_handle.ev_param);
            __ses_handle.ses_state = DALI2_L_SES_STATE_RDY;
            break;

        default:
            break;
    }

}

void dali2_l_ses_timer_cb_handler(void)
{
    dali2_ret_t dali2_ret;

    //! Verify initialization
    if (!__ses_handle.is_init) {
        return;
    }

    //! Verify Session state
    if (__ses_handle.ses_state == DALI2_L_SES_STATE_RDY) {
        return;
    }

    switch (__ses_handle.ev_param.msg) {
        case DALI2_L_SES_SEND_TWICE:
            if (__ses_handle.send_twice) {
                //! Send Forward frame second time
                dali2_ret = dali2_l_phy_exec_frame(__ses_handle.phy_frame_type, __ses_handle.ev_param.msg_data);
                if (dali2_ret != DALI2_RET_SUCCESS) {
                    __ses_handle.evt_func(DALI2_L_SES_EVT_COLLISION, &__ses_handle.ev_param);
                    __ses_handle.ses_state = DALI2_L_SES_STATE_RDY;
                }
                __ses_handle.send_twice = 0;
                break;
            } //! Else Continue to the next case

        case DALI2_L_SES_SEND:
            //! This must be settling time, well just go to Ready state
            __ses_handle.ses_state = DALI2_L_SES_STATE_RDY;
            break;

        case DALI2_L_SES_QUERY:
            if (__ses_handle.ses_state == DALI2_L_SES_STATE_PROGRESS) {
                //! Timeout occur on waiting for Backward frame
                //! Going Ready state immediately
                __ses_handle.evt_func(DALI2_L_SES_EVT_TIMEOUT, &__ses_handle.ev_param);
            }
            __ses_handle.ses_state = DALI2_L_SES_STATE_RDY;
            break;

        default:
            break;
    }
}

dali2_ret_t dali2_l_ses_init(dali2_l_ses_evt_func_t evt_handler)
{
    dali2_ret_t dali2_ret = DALI2_RET_SUCCESS;

    //! Verifying initialization done
    if (__ses_handle.is_init) {
        dali2_ret = DALI2_RET_INTERNAL_ERROR;
        goto __ret;
    }

    //! Function pointer verification
    if (!evt_handler) {
        dali2_ret = DALI2_RET_INVALID_PARAMS;
        goto __ret;
    }

    //! Secure session handler
    __ses_handle.evt_func = evt_handler;

    //! Physical layer initialization
    dali2_ret = dali2_l_phy_init(__dali2_l_ses_phy_evt_handler);
    if (dali2_ret != DALI2_RET_SUCCESS) {
        goto __ret;
    }

    __ses_handle.is_init = 1;

__ret:
    return dali2_ret;
}

dali2_ret_t dali2_l_ses_deinit(void)
{
    dali2_ret_t dali2_ret = DALI2_RET_SUCCESS;

    //! Verifying initialization done
    if (!__ses_handle.is_init) {
        dali2_ret = DALI2_RET_INTERNAL_ERROR;
        goto __ret;
    }

    //! Physical layer deinitialization
    dali2_ret = dali2_l_phy_deinit();
    if (dali2_ret == DALI2_RET_SUCCESS) {
        __ses_handle.is_init = 0;
    }

__ret:
    return dali2_ret;
}

dali2_ret_t dali2_l_ses_exec(DALI2_L_SES_MSG_T msg, DALI2_L_PHY_FRAME_T frame_type, unsigned int frame_data)
{
    dali2_ret_t dali2_ret = DALI2_RET_SUCCESS;

    //! Verify initialization
    if (!__ses_handle.is_init) {
        dali2_ret = DALI2_RET_INTERNAL_ERROR;
        goto __ret;
    }

    //! Verify session state
    if (__ses_handle.ses_state != DALI2_L_SES_STATE_RDY) {
        dali2_ret = DALI2_RET_BUSY;
        goto __ret;
    }

    switch (msg) {
        case DALI2_L_SES_SEND:
            __ses_handle.send_twice = 0;
            break;

        case DALI2_L_SES_SEND_TWICE:
            __ses_handle.send_twice = 1;
            break;

        case DALI2_L_SES_QUERY:
            break;

        default:
            dali2_ret = DALI2_RET_INVALID_PARAMS;
            goto __ret;
    }

    //! Execute backward frame on physical layer
    dali2_ret = dali2_l_phy_exec_frame(frame_type, frame_data);
    if (dali2_ret == DALI2_RET_SUCCESS) {
        __ses_handle.phy_frame_type = frame_type;
        __ses_handle.ev_param.msg = msg;
        __ses_handle.ev_param.msg_data = frame_data;
        __ses_handle.ses_state = DALI2_L_SES_STATE_PROGRESS;
    }

__ret:
    return dali2_ret;
}
