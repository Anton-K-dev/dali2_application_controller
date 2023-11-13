/**
 * @copyright
 *
 * @file    dali2_hal_internal.h
 * @author  Anton K.
 * @date    22 Sep 2021
 *
 * @brief   DALI-2 Application Controller.
 *          HAL Internal header file. Do not use this functions.
 *          This is dedicated internal header for HAL library.
 */
#ifndef DALI2_HAL_INTERNAL_H_
#define DALI2_HAL_INTERNAL_H_

#include "dali2_l_app.h"
#include "dali2_error.h"


/**@brief Check mutex
 *
 * @return DALI2_HAL_EVT_FREE - mutex is free
 *         DALI2_RET_BUSY - mutex is captured by @retval
 */
DALI2_HAL_EVT_T dali2_hal_mtx_check(void);

/**@brief Take mutex
 *
 * @param[IN] evt_mtx Event for capturing mutex
 * @return If (@retval == evt_mtx) - mutex has taken by @param evt_mtx
 *         If (@retval != evt_mtx) - mutex is captured by @retval
 */
DALI2_HAL_EVT_T dali2_hal_mtx_take(DALI2_HAL_EVT_T evt_mtx);

/**@brief Give mutex
 */
void dali2_hal_mtx_give(void);

/**@brief Push single command to queue
 */
dali2_ret_t dali2_hal_queue_push(DALI2_L_APP_CMD_T cmd, dali2_l_app_cmd_data_t *cmd_data);

/**@brief Dispatch this function inside dali2_l_app_evt_func_t()
 *
 * @param[IN] evt - @see dali2_l_app_evt_func_t()
 * @param[IN] evt_data - @see dali2_l_app_evt_func_t()
 */
void dali2_hal_addr_alloc_dispatch(DALI2_L_APP_EVT_T evt, dali2_l_app_evt_data_t *evt_data);

/**@brief Dispatch this function inside dali2_l_app_evt_func_t()
 *
 * @param[IN] evt - @see dali2_l_app_evt_func_t()
 * @param[IN] evt_data - @see dali2_l_app_evt_func_t()
 */
void dali2_hal_dim_cfg_dispatch(DALI2_L_APP_EVT_T evt, dali2_l_app_evt_data_t *evt_data);

/**@brief Dispatch this function inside dali2_l_app_evt_func_t()
 *
 * @param[IN] evt - @see dali2_l_app_evt_func_t()
 * @param[IN] evt_data - @see dali2_l_app_evt_func_t()
 */
void dali2_hal_dim_ctrl_dispatch(DALI2_L_APP_EVT_T evt, dali2_l_app_evt_data_t *evt_data);

#endif /* DALI2_HAL_INTERNAL_H_ */
