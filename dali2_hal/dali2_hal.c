/**
 * @copyright
 *
 * @file    dali2_hal.c
 * @author  Anton K.
 * @date    22 Sep 2021
 *
 * @brief   DALI-2 Application Controller.
 *          HAL source file
 *
 * @details This is internal HAL source file.
 *          User should not use this functions
 */

#include "string.h"

#include "dali2_hal.h"
#include "dali2_hal_internal.h"

#define DALI2_HAL_IS_VALID_EVT(EVT)     (EVT < DALI2_HAL_EVT_FREE)

static DALI2_HAL_EVT_T __hal_queue_evt = DALI2_HAL_EVT_FREE;
static DALI2_HAL_EVT_T __hal_freed_by_evt = DALI2_HAL_EVT_FREE;
static DALI2_L_APP_CMD_T __hal_queue_app_cmd;
static dali2_l_app_cmd_data_t __hal_queue_app_cmd_data;

DALI2_HAL_EVT_T dali2_hal_mtx_check(void)
{
    return __hal_queue_evt;
}

DALI2_HAL_EVT_T dali2_hal_mtx_take(DALI2_HAL_EVT_T evt_mtx)
{
    if (evt_mtx < __hal_queue_evt) {
        if (DALI2_HAL_IS_VALID_EVT(evt_mtx)) {
            __hal_queue_evt = evt_mtx;
        }
    }

    return __hal_queue_evt;
}

void dali2_hal_mtx_give(void)
{
    __hal_freed_by_evt = __hal_queue_evt;
    __hal_queue_evt = DALI2_HAL_EVT_FREE;
}

dali2_ret_t dali2_hal_queue_push(DALI2_L_APP_CMD_T cmd, dali2_l_app_cmd_data_t *cmd_data)
{
    //! Parameter verification
    if (!cmd_data || !DALI2_HAL_IS_VALID_EVT(__hal_queue_evt)) {
        DALI2_L_BSP_LOG("DALI2 HAL Internal fail on queue!");

        //! Free mutex
        __hal_queue_evt = DALI2_HAL_EVT_FREE;
        __hal_freed_by_evt = DALI2_HAL_EVT_FREE;
        return DALI2_RET_INVALID_PARAMS;
    }

    //! Copy content
    __hal_queue_app_cmd = cmd;
    memcpy(&__hal_queue_app_cmd_data, cmd_data, sizeof(dali2_l_app_cmd_data_t));
    return DALI2_RET_SUCCESS;
}

dali2_ret_t dali2_hal_process(DALI2_HAL_EVT_T *evt)
{
    dali2_ret_t dali2_ret = DALI2_RET_BUSY;

    if (DALI2_HAL_IS_VALID_EVT(__hal_queue_evt)) {

        //! Set event as active
        *evt = __hal_queue_evt;

        //! Execute command if mutex is active
        if (DALI2_L_APP_CMD_UNKNOWN != __hal_queue_app_cmd) {
            dali2_ret = dali2_l_app_cmd_execute(__hal_queue_app_cmd, &__hal_queue_app_cmd_data);
            switch (dali2_ret) {
                case DALI2_RET_SUCCESS:
                    dali2_ret = DALI2_RET_BUSY;
                    break;

                case DALI2_RET_BUSY:
                    break;

                default:
                    //! Here is error occur!
                    //! Free mutex
                    dali2_hal_mtx_give();
                    break;
            }
        }
    } else if (DALI2_HAL_IS_VALID_EVT(__hal_freed_by_evt)) {
        //! Set event has freed
        *evt = __hal_freed_by_evt;
        __hal_freed_by_evt = DALI2_HAL_EVT_FREE;
        dali2_ret = DALI2_RET_SUCCESS;
    }

    return dali2_ret;
}

void dali2_hal_app_evt_dispatch(DALI2_L_APP_EVT_T evt, dali2_l_app_evt_data_t *evt_data)
{
    //! Skip interrupted command response
    if (evt_data->cmd != __hal_queue_app_cmd) {
        return;
    }

    switch (__hal_queue_evt) {
        case DALI2_HAL_EVT_ADDR_ALLOC:
            dali2_hal_addr_alloc_dispatch(evt, evt_data);
            break;

        case DALI2_HAL_EVT_DIM_CFG:
            dali2_hal_dim_cfg_dispatch(evt, evt_data);
            break;

        case DALI2_HAL_EVT_DIM_CTRL:
            dali2_hal_dim_ctrl_dispatch(evt, evt_data);
            break;

        case DALI2_HAL_EVT_FREE:
        default:
            break;
    }
}

