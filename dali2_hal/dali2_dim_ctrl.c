/**
 * @copyright
 *
 * @file    dali2_dim_ctrl.c
 * @author  Anton K.
 * @date    15 Oct 2021
 *
 * @brief   DALI-2 Application Controller.
 *          HAL Dimmer Control methods source file
 */

#include "string.h"

#include "dali2_hal.h"
#include "dali2_hal_internal.h"

static dali2_l_app_network_t __dim_node;
static unsigned char __dim_target_level;
static unsigned char __dim_actual_level;
static unsigned char __dim_status;
static unsigned char __dim_failure_status;
static dali2_hal_dim_meta_t __dim_meta_cfg;

void dali2_hal_dim_ctrl_dispatch(DALI2_L_APP_EVT_T evt, dali2_l_app_evt_data_t *evt_data)
{
    dali2_l_app_cmd_data_t instr_data;

    switch (evt_data->cmd) {
        case DALI2_L_APP_CMD_SET_POWER_ON_LEVEL_DTR0:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                //! Query Power On level
                instr_data.std_cmd.net.method = __dim_node.method;
                instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_POWER_ON_LEVEL, &instr_data);
            }
            break;

        case DALI2_L_APP_CMD_QUERY_POWER_ON_LEVEL:
            if ((evt == DALI2_L_APP_EVT_SUCCESS) &&
                (evt_data->cmd_data.std_rsp.data == __dim_target_level)) {
                //! Setting System Failure level
                instr_data.std_cmd.net.method = __dim_node.method;
                instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                instr_data.std_cmd.data = __dim_target_level;
                dali2_hal_queue_push(DALI2_L_APP_CMD_SET_SYSTEM_FAILURE_LEVEL_DTR0, &instr_data);
            } else {
                //! Setting Power On level
                instr_data.std_cmd.net.method = __dim_node.method;
                instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                instr_data.std_cmd.data = __dim_target_level;
                dali2_hal_queue_push(DALI2_L_APP_CMD_SET_POWER_ON_LEVEL_DTR0, &instr_data);
            }
            break;

        case DALI2_L_APP_CMD_SET_SYSTEM_FAILURE_LEVEL_DTR0:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                //! Query Power On level
                instr_data.std_cmd.net.method = __dim_node.method;
                instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_SYSTEM_FAILURE_LEVEL, &instr_data);
            }
            break;

        case DALI2_L_APP_CMD_QUERY_SYSTEM_FAILURE_LEVEL:
            if ((evt == DALI2_L_APP_EVT_SUCCESS) &&
                (evt_data->cmd_data.std_rsp.data == __dim_target_level)) {
                //! Setting Dimmer level with fade
                instr_data.std_cmd.net.method = __dim_node.method;
                instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                instr_data.std_cmd.data = __dim_target_level;
                dali2_hal_queue_push(DALI2_L_APP_CMD_DAPC, &instr_data);
            } else {
                //! REPEAT: Setting System Failure level
                instr_data.std_cmd.net.method = __dim_node.method;
                instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                instr_data.std_cmd.data = __dim_target_level;
                dali2_hal_queue_push(DALI2_L_APP_CMD_SET_SYSTEM_FAILURE_LEVEL_DTR0, &instr_data);
            }
            break;

        case DALI2_L_APP_CMD_DAPC:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                //! Query status
                instr_data.std_cmd.net.method = __dim_node.method;
                instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_STATUS, &instr_data);
            }
            break;

        case DALI2_L_APP_CMD_QUERY_STATUS:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                //! Status is here
                __dim_status |= evt_data->cmd_data.std_rsp.data;
            } else if (evt == DALI2_L_APP_EVT_FAULT) {
                break;
            }

            DALI2_L_BSP_LOG("DALI Status 0x%X", __dim_status);

            //! Query Short Circuit
            instr_data.std_cmd.net.method = __dim_node.method;
            instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
            dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_SHORT_CIRCUIT, &instr_data);
            break;

        case DALI2_L_APP_CMD_QUERY_ACTUAL_LEVEL:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                __dim_actual_level = evt_data->cmd_data.std_rsp.data;

                //! Level thresholds control
                if ((__dim_meta_cfg.phy_min && (__dim_target_level >= __dim_meta_cfg.phy_min)) ||
                    (!__dim_target_level)) {
                    if (evt_data->cmd_data.std_rsp.data == __dim_target_level) {
                        //! Setting level completed!

                        //! Free mutex
                        dali2_hal_mtx_give();
                    }
                }
            }
            break;

        case DALI2_L_APP_CMD_QUERY_CONTROL_GEAR_PRESENT:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                //! Clear failure status
                __dim_status &= ~DALI2_L_APP_CMD_STATUS_CONTROL_GEAR_FAILURE;
            } else if (evt == DALI2_L_APP_EVT_TIMEOUT) {
                DALI2_L_BSP_LOG("DALI Control Gear presence failed!");
                //! Set failure status
                __dim_status |= DALI2_L_APP_CMD_STATUS_CONTROL_GEAR_FAILURE;
            }

            //! Query Lamp Failure status
            instr_data.std_cmd.net.method = __dim_node.method;
            instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
            dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_LAMP_FAILURE, &instr_data);
            break;

        case DALI2_L_APP_CMD_QUERY_LAMP_FAILURE:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                DALI2_L_BSP_LOG("DALI Lamp Failure detected!");
                //! Set lamp failure status
                __dim_status |= DALI2_L_APP_CMD_STATUS_LAMP_FAILURE;
            } else if (evt == DALI2_L_APP_EVT_TIMEOUT) {
                //! Reset lamp failure status
                __dim_status &= ~DALI2_L_APP_CMD_STATUS_LAMP_FAILURE;
            }

            //! Query Lamp Power On status
            instr_data.std_cmd.net.method = __dim_node.method;
            instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
            dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_LAMP_POWER_ON, &instr_data);
            break;

        case DALI2_L_APP_CMD_QUERY_LAMP_POWER_ON:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                //! Set lamp failure status
                __dim_status |= DALI2_L_APP_CMD_STATUS_LAMP_ON;
            } else if (evt == DALI2_L_APP_EVT_TIMEOUT) {
                //! Reset lamp failure status
                __dim_status &= ~DALI2_L_APP_CMD_STATUS_LAMP_ON;
            }

            //! Query Limit Error status
            instr_data.std_cmd.net.method = __dim_node.method;
            instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
            dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_LIMIT_ERROR, &instr_data);
            break;

        case DALI2_L_APP_CMD_QUERY_LIMIT_ERROR:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                //! Set limit error status
                __dim_status |= DALI2_L_APP_CMD_STATUS_LIMIT_ERROR;
            } else if (evt == DALI2_L_APP_EVT_TIMEOUT) {
                //! Reset limit error status
                __dim_status &= ~DALI2_L_APP_CMD_STATUS_LIMIT_ERROR;
            }

            //! Query Reset State status
            instr_data.std_cmd.net.method = __dim_node.method;
            instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
            dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_RESET_STATE, &instr_data);
            break;

        case DALI2_L_APP_CMD_QUERY_RESET_STATE:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                //! Set limit error status
                __dim_status |= DALI2_L_APP_CMD_STATUS_RESET_STATE;
            } else if (evt == DALI2_L_APP_EVT_TIMEOUT) {
                //! Reset limit error status
                __dim_status &= ~DALI2_L_APP_CMD_STATUS_RESET_STATE;
            }

            //! Query Missing Short Address status
            instr_data.std_cmd.net.method = __dim_node.method;
            instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
            dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_MISSING_SHORT_ADDRESS, &instr_data);
            break;

        case DALI2_L_APP_CMD_QUERY_MISSING_SHORT_ADDRESS:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                //! Set Short Address status
                __dim_status |= DALI2_L_APP_CMD_STATUS_SHORT_ADDRESS;
            } else if (evt == DALI2_L_APP_EVT_TIMEOUT) {
                //! Reset Short Address status
                __dim_status &= ~DALI2_L_APP_CMD_STATUS_SHORT_ADDRESS;
            }

            //! Query status
            instr_data.std_cmd.net.method = __dim_node.method;
            instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
            dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_POWER_FAILURE, &instr_data);
            break;

        case DALI2_L_APP_CMD_QUERY_POWER_FAILURE:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                //! Set Power Failure status
                __dim_status |= DALI2_L_APP_CMD_STATUS_POWER_CYCLE_SEEN;
            } else if (evt == DALI2_L_APP_EVT_TIMEOUT) {
                //! Reset Power Failure status
                __dim_status &= ~DALI2_L_APP_CMD_STATUS_POWER_CYCLE_SEEN;
            }

            //! Query status
            instr_data.std_cmd.net.method = __dim_node.method;
            instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
            dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_STATUS, &instr_data);
            break;


        case DALI2_L_APP_CMD_QUERY_SHORT_CIRCUIT:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                __dim_failure_status |= DALI2_L_APP_LED_FAILURE_SHORT_CIRCUIT;
            } else if (evt == DALI2_L_APP_EVT_TIMEOUT) {
                __dim_failure_status &= ~DALI2_L_APP_LED_FAILURE_SHORT_CIRCUIT;
            } else {
                break;
            }

            //! Query Open Circuit
            instr_data.std_cmd.net.method = __dim_node.method;
            instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
            dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_OPEN_CIRCUIT, &instr_data);
            break;

        case DALI2_L_APP_CMD_QUERY_OPEN_CIRCUIT:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                __dim_failure_status |= DALI2_L_APP_LED_FAILURE_OPEN_CIRCUIT;
            } else if (evt == DALI2_L_APP_EVT_TIMEOUT) {
                __dim_failure_status &= ~DALI2_L_APP_LED_FAILURE_OPEN_CIRCUIT;
            } else {
                break;
            }

            //! Query Load Decrease
            instr_data.std_cmd.net.method = __dim_node.method;
            instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
            dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_LOAD_DECREASE, &instr_data);
            break;

        case DALI2_L_APP_CMD_QUERY_LOAD_DECREASE:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                __dim_failure_status |= DALI2_L_APP_LED_FAILURE_LOAD_DECREASE;
            } else if (evt == DALI2_L_APP_EVT_TIMEOUT) {
                __dim_failure_status &= ~DALI2_L_APP_LED_FAILURE_LOAD_DECREASE;
            } else {
                break;
            }

            //! Query Load Increase
            instr_data.std_cmd.net.method = __dim_node.method;
            instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
            dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_LOAD_INCREASE, &instr_data);
            break;

        case DALI2_L_APP_CMD_QUERY_LOAD_INCREASE:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                __dim_failure_status |= DALI2_L_APP_LED_FAILURE_LOAD_INCREASE;
            } else if (evt == DALI2_L_APP_EVT_TIMEOUT) {
                __dim_failure_status &= ~DALI2_L_APP_LED_FAILURE_LOAD_INCREASE;
            } else {
                break;
            }

            //! Query Current Protector Active
            instr_data.std_cmd.net.method = __dim_node.method;
            instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
            dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_CURRENT_PROTECTOR_ACTIVE, &instr_data);
            break;

        case DALI2_L_APP_CMD_QUERY_CURRENT_PROTECTOR_ACTIVE:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                __dim_failure_status |= DALI2_L_APP_LED_FAILURE_CURRENT_PROTECTOR_ACTIVE;
            } else if (evt == DALI2_L_APP_EVT_TIMEOUT) {
                __dim_failure_status &= ~DALI2_L_APP_LED_FAILURE_CURRENT_PROTECTOR_ACTIVE;
            } else {
                break;
            }

            //! Query Thermal Shut Down
            instr_data.std_cmd.net.method = __dim_node.method;
            instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
            dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_THERMAL_SHUT_DOWN, &instr_data);
            break;

        case DALI2_L_APP_CMD_QUERY_THERMAL_SHUT_DOWN:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                __dim_failure_status |= DALI2_L_APP_LED_FAILURE_THERMAL_SHUT_DOWN;
            } else if (evt == DALI2_L_APP_EVT_TIMEOUT) {
                __dim_failure_status &= ~DALI2_L_APP_LED_FAILURE_THERMAL_SHUT_DOWN;
            } else {
                break;
            }

            //! Query Thermal Overload
            instr_data.std_cmd.net.method = __dim_node.method;
            instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
            dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_THERMAL_OVERLOAD, &instr_data);
            break;

        case DALI2_L_APP_CMD_QUERY_THERMAL_OVERLOAD:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                __dim_failure_status |= DALI2_L_APP_LED_FAILURE_THERMAL_OVERLOAD;
            } else if (evt == DALI2_L_APP_EVT_TIMEOUT) {
                __dim_failure_status &= ~DALI2_L_APP_LED_FAILURE_THERMAL_OVERLOAD;
            } else {
                break;
            }

            //! Query Thermal Overload
            instr_data.std_cmd.net.method = __dim_node.method;
            instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
            dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_REFERENCE_MEASUREMENT_FAILED, &instr_data);
            break;

        case DALI2_L_APP_CMD_QUERY_REFERENCE_MEASUREMENT_FAILED:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                __dim_failure_status |= DALI2_L_APP_LED_FAILURE_REFERENCE_FAILED;
            } else if (evt == DALI2_L_APP_EVT_TIMEOUT) {
                __dim_failure_status &= ~DALI2_L_APP_LED_FAILURE_REFERENCE_FAILED;
            } else {
                break;
            }

            //! Query Failure Status
            instr_data.std_cmd.net.method = __dim_node.method;
            instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
            dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_FAILURE_STATUS, &instr_data);
            break;

        case DALI2_L_APP_CMD_QUERY_FAILURE_STATUS:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                //! Failure Status is here
                __dim_failure_status |= evt_data->cmd_data.std_rsp.data;
            } else if (evt == DALI2_L_APP_EVT_FAULT) {
                break;
            }

            DALI2_L_BSP_LOG("DALI Failure Status 0x%X", __dim_failure_status);

            //! Query actual level
            instr_data.std_cmd.net.method = __dim_node.method;
            instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
            dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_ACTUAL_LEVEL, &instr_data);
            break;

        case DALI2_L_APP_CMD_UNKNOWN:
            //! Setting Power On level
            instr_data.std_cmd.net.method = __dim_node.method;
            instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
            instr_data.std_cmd.data = __dim_target_level;
            dali2_hal_queue_push(DALI2_L_APP_CMD_SET_POWER_ON_LEVEL_DTR0, &instr_data);
            break;

        default:
            break;
    }
}

dali2_ret_t dali2_hal_dim_set_level(unsigned char level, dali2_l_app_network_t *node)
{
    dali2_ret_t dali2_ret = DALI2_RET_SUCCESS;
    dali2_l_app_cmd_data_t instr_data;

    //! Retrieve constant dimmer configuration
    dali2_ret = dali2_hal_dim_meta_get(&__dim_meta_cfg, node);
    if (dali2_ret != DALI2_RET_SUCCESS) goto __ret;

    //! Capture mutex
    if (dali2_hal_mtx_check() != DALI2_HAL_EVT_DIM_CTRL) {
        //! Try to take mutex
        if (dali2_hal_mtx_take(DALI2_HAL_EVT_DIM_CTRL) != DALI2_HAL_EVT_DIM_CTRL) {
            dali2_ret = DALI2_RET_BUSY;
            goto __ret;
        }
    }

    //! Copy metadata
    __dim_target_level = DALI2_DIM_LEVEL_LIM(level);
    if (__dim_target_level) {
        __dim_target_level += DALI2_DIM_CFG_MIN_LEVEL;

        //! Fix target level according physical minimum
        if (__dim_meta_cfg.phy_min &&
            __dim_target_level < __dim_meta_cfg.phy_min) {
            //! Set target level as physical minimum
            __dim_target_level = __dim_meta_cfg.phy_min;
        }
    }

    memcpy(&__dim_node, node, sizeof(dali2_l_app_network_t));

    //! Setting Power On level
    instr_data.std_cmd.net.method = __dim_node.method;
    instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
    instr_data.std_cmd.data = __dim_target_level;
    dali2_hal_queue_push(DALI2_L_APP_CMD_SET_POWER_ON_LEVEL_DTR0, &instr_data);

__ret:
    return dali2_ret;
}

static void __dali2_hal_update_dim_state(dali2_l_app_network_t *node)
{
    dali2_l_app_cmd_data_t instr_data;

    //! Copy metadata
    memcpy(&__dim_node, node, sizeof(dali2_l_app_network_t));

    if (dali2_hal_mtx_check() == DALI2_HAL_EVT_FREE) {
        if (dali2_hal_mtx_take(DALI2_HAL_EVT_DIM_CTRL) != DALI2_HAL_EVT_DIM_CTRL) {
            return;
        }

        //! Query driver presence
        instr_data.std_cmd.net.method = __dim_node.method;
        instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
        dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_CONTROL_GEAR_PRESENT, &instr_data);
    }
}

dali2_ret_t dali2_hal_dim_get_level(unsigned char *level, dali2_l_app_network_t *node)
{
    *level = (__dim_actual_level) ? __dim_actual_level - DALI2_DIM_CFG_MIN_LEVEL : __dim_actual_level;

    __dali2_hal_update_dim_state(node);

    return DALI2_RET_SUCCESS;
}

dali2_ret_t dali2_hal_dim_get_status(unsigned char *status, dali2_l_app_network_t *node)
{
    *status = __dim_status;

    __dali2_hal_update_dim_state(node);

    return DALI2_RET_SUCCESS;
}

dali2_ret_t dali2_hal_dim_get_led_failure_status(unsigned char *failure_status, dali2_l_app_network_t *node)
{
    *failure_status = __dim_failure_status;

    __dali2_hal_update_dim_state(node);

    return DALI2_RET_SUCCESS;
}
