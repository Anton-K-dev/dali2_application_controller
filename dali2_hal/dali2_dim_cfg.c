/**
 * @copyright
 *
 * @file    dali2_dim_cfg.c
 * @author  Anton K.
 * @date    15 Oct 2021
 *
 * @brief   DALI-2 Application Controller.
 *          HAL Dimmer Configuration method source file
 */

#include "string.h"

#include "dali2_hal.h"
#include "dali2_hal_internal.h"

static dali2_l_app_network_t __dim_node;
static dali2_hal_dim_cfg_t __dim_cfg;
static dali2_hal_dim_meta_t __dim_cfg_meta;
static volatile unsigned char __dim_cfg_meta_retrieved;

static inline unsigned char __dim_cfg_sec_to_ext_fade_time(unsigned int fade_time_s)
{
    if (fade_time_s == 0x00) {
        return DALI2_L_APP_SET_EXTENDED_FADE_TIME_CALCULATION(DALI2_L_APP_SET_EXTENDED_FADE_TIME_BASE_LIM_S_1, DALI2_L_APP_SET_EXTENDED_FADE_TIME_MUL_0MS);
    } else if (fade_time_s <= DALI2_L_APP_SET_EXTENDED_FADE_TIME_BASE_LIM_S_1) {
        return DALI2_L_APP_SET_EXTENDED_FADE_TIME_CALCULATION(fade_time_s, DALI2_L_APP_SET_EXTENDED_FADE_TIME_MUL_1S);
    } else if (fade_time_s <= DALI2_L_APP_SET_EXTENDED_FADE_TIME_BASE_LIM_S_2) {
        return DALI2_L_APP_SET_EXTENDED_FADE_TIME_CALCULATION((unsigned char) fade_time_s / 10, DALI2_L_APP_SET_EXTENDED_FADE_TIME_MUL_10S);
    } else if (fade_time_s <= DALI2_L_APP_SET_EXTENDED_FADE_TIME_BASE_LIM_S_3) {
        return DALI2_L_APP_SET_EXTENDED_FADE_TIME_CALCULATION((unsigned char) fade_time_s / DIM_CFG_SEC_IN_MIN, DALI2_L_APP_SET_EXTENDED_FADE_TIME_MUL_1M);
    } else {
        return DALI2_L_APP_SET_EXTENDED_FADE_TIME_CALCULATION(DALI2_L_APP_SET_EXTENDED_FADE_TIME_BASE_MAX, DALI2_L_APP_SET_EXTENDED_FADE_TIME_MUL_1M);
    }
}

void dali2_hal_dim_cfg_dispatch(DALI2_L_APP_EVT_T evt, dali2_l_app_evt_data_t *evt_data)
{
    dali2_l_app_cmd_data_t instr_data;

    switch (evt_data->cmd) {
        case DALI2_L_APP_CMD_QUERY_DEVICE_TYPE:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                __dim_cfg_meta.device_type = evt_data->cmd_data.std_rsp.data;

                DALI2_L_BSP_LOG("Device type %u", __dim_cfg_meta.device_type);

                //! Verify device is LED module
                if (__dim_cfg_meta.device_type == DALI2_L_APP_CMD_DEVICE_LED) {
                    //! Query Light Source Type
                    instr_data.std_cmd.net.method = __dim_node.method;
                    instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                    dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_LIGHT_SOURCE_TYPE, &instr_data);
                } else {
                    DALI2_L_BSP_LOG("Device type is not supported. Terminated Configuration!");

                    //! Free mutex
                    dali2_hal_mtx_give();
                }
            }
            break;

        case DALI2_L_APP_CMD_QUERY_LIGHT_SOURCE_TYPE:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                __dim_cfg_meta.light_src_type = evt_data->cmd_data.std_rsp.data;

                DALI2_L_BSP_LOG("Light source type %u", __dim_cfg_meta.light_src_type);

                //! Enable LED Module devices
                dali2_hal_queue_push(DALI2_L_APP_CMD_ENABLE_DEVICE_TYPE_6, &instr_data);
            }
            break;

        case DALI2_L_APP_CMD_ENABLE_DEVICE_TYPE_6:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                //! Query LED Operating Mode
                instr_data.std_cmd.net.method = __dim_node.method;
                instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_OPERATING_MODE_LED, &instr_data);
            }
            break;

        case DALI2_L_APP_CMD_QUERY_OPERATING_MODE_LED:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                __dim_cfg_meta.led_operating_mode = evt_data->cmd_data.std_rsp.data;

                DALI2_L_BSP_LOG("LED operating mode 0x%X", __dim_cfg_meta.led_operating_mode);

                //! Query LED featured
                instr_data.std_cmd.net.method = __dim_node.method;
                instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_FEATURES, &instr_data);
            }
            break;

        case DALI2_L_APP_CMD_QUERY_FEATURES:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                __dim_cfg_meta.led_features = evt_data->cmd_data.std_rsp.data;

                DALI2_L_BSP_LOG("LED features byte 0x%X", __dim_cfg_meta.led_features);

                //! Verify Current Protector Support
                if (__dim_cfg_meta.led_features & DALI2_L_APP_LED_FEATURE_CURRENT_PROTECTOR_SUPPORTED) {
                    //! Query Current Protector Enabled
                    instr_data.std_cmd.net.method = __dim_node.method;
                    instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                    dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_CURRENT_PROTECTOR_ENABLED, &instr_data);
                } else {
                    DALI2_L_BSP_LOG("Current protector doesn't supported!");

                    //! Verify does non-logarithmic curve supported
                    if (__dim_cfg_meta.led_operating_mode & DALI2_L_APP_LED_OPERATING_MODE_NON_LOGARITHMIC_ACTIVE) {
                        //! Query Dimming Curve
                        instr_data.std_cmd.net.method = __dim_node.method;
                        instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                        dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_DIMMING_CURVE, &instr_data);
                    } else {
                        DALI2_L_BSP_LOG("Non-logarithmic curve doesn't supported!");

                        //! Query Physical minimum
                        instr_data.std_cmd.net.method = __dim_node.method;
                        instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                        dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_PHYSICAL_MINIMUM, &instr_data);
                    }
                }
            }
            break;

        case DALI2_L_APP_CMD_QUERY_CURRENT_PROTECTOR_ENABLED:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {   //! Enabled
                //! Verify configuration settings
                if (!__dim_cfg.curr_protect_en) {
                    //! Disable Current Protector
                    instr_data.std_cmd.net.method = __dim_node.method;
                    instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                    dali2_hal_queue_push(DALI2_L_APP_CMD_DISABLE_CURRENT_PROTECTOR, &instr_data);
                    break;
                }
            } else if (evt == DALI2_L_APP_EVT_TIMEOUT) {    //! Disabled
                //! Verify configuration settings
                if (__dim_cfg.curr_protect_en) {
                    //! Disable Current Protector
                    instr_data.std_cmd.net.method = __dim_node.method;
                    instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                    dali2_hal_queue_push(DALI2_L_APP_CMD_ENABLE_CURRENT_PROTECTOR, &instr_data);
                    break;
                }
            } else {
                //! On RETRY
                break;
            }

            //! Verify does non-logarithmic curve supported
            if (__dim_cfg_meta.led_operating_mode & DALI2_L_APP_LED_OPERATING_MODE_NON_LOGARITHMIC_ACTIVE) {
                //! Query Dimming Curve
                instr_data.std_cmd.net.method = __dim_node.method;
                instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_DIMMING_CURVE, &instr_data);
            } else {
                DALI2_L_BSP_LOG("Non-logarithmic curve doesn't supported!");

                //! Query Physical minimum
                instr_data.std_cmd.net.method = __dim_node.method;
                instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_PHYSICAL_MINIMUM, &instr_data);
            }
            break;

        case DALI2_L_APP_CMD_DISABLE_CURRENT_PROTECTOR:
        case DALI2_L_APP_CMD_ENABLE_CURRENT_PROTECTOR:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                //! RETRY: Query Current Protector Enabled
                instr_data.std_cmd.net.method = __dim_node.method;
                instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_CURRENT_PROTECTOR_ENABLED, &instr_data);
            }
            break;

        case DALI2_L_APP_CMD_QUERY_DIMMING_CURVE:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                DALI2_L_BSP_LOG("Dimming curve 0x%X", evt_data->cmd_data.std_rsp.data);

                if (evt_data->cmd_data.std_rsp.data == __dim_cfg.dim_curve) {
                    //! Query Physical minimum
                    instr_data.std_cmd.net.method = __dim_node.method;
                    instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                    dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_PHYSICAL_MINIMUM, &instr_data);
                } else {
                    //! Select Dimming Curve
                    instr_data.std_cmd.net.method = __dim_node.method;
                    instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                    instr_data.std_cmd.data = __dim_cfg.dim_curve;
                    dali2_hal_queue_push(DALI2_L_APP_CMD_SELECT_DIMMING_CURVE, &instr_data);
                }
            }
            break;

        case DALI2_L_APP_CMD_SELECT_DIMMING_CURVE:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                //! Query Dimming Curve
                instr_data.std_cmd.net.method = __dim_node.method;
                instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_DIMMING_CURVE, &instr_data);
            }
            break;


        case DALI2_L_APP_CMD_QUERY_PHYSICAL_MINIMUM:
            if (evt == DALI2_L_APP_EVT_SUCCESS &&
                evt_data->cmd_data.std_rsp.data != DALI2_DIM_CFG_WRONG_LEVEL) {
                __dim_cfg_meta.phy_min = evt_data->cmd_data.std_rsp.data;

                DALI2_L_BSP_LOG("Physical minimum: %u", __dim_cfg_meta.phy_min);

                //! Here is the point, where all dimmer configuration metadata retrieved
                __dim_cfg_meta_retrieved = 1;

                //! Setting operating mode
                instr_data.std_cmd.net.method = __dim_node.method;
                instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                instr_data.std_cmd.data = __dim_cfg.mode;
                dali2_hal_queue_push(DALI2_L_APP_CMD_SET_OPERATING_MODE_DTR0, &instr_data);
            }
            break;

        case DALI2_L_APP_CMD_SET_OPERATING_MODE_DTR0:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                //! Query operating mode
                instr_data.std_cmd.net.method = __dim_node.method;
                instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_OPERATING_MODE, &instr_data);
            }
            break;

        case DALI2_L_APP_CMD_QUERY_OPERATING_MODE:
            if ((evt == DALI2_L_APP_EVT_SUCCESS) &&
                (evt_data->cmd_data.std_rsp.data == __dim_cfg.mode)) {
                //! Setting maximum level
                instr_data.std_cmd.net.method = __dim_node.method;
                instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                instr_data.std_cmd.data = __dim_cfg.level_max;
                dali2_hal_queue_push(DALI2_L_APP_CMD_SET_MAX_LEVEL_DTR0, &instr_data);
            } else {
                DALI2_L_BSP_LOG("Dimmer configuration Failed on set mode!");

                //! REPEAT: Setting operating mode
                instr_data.std_cmd.net.method = __dim_node.method;
                instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                instr_data.std_cmd.data = __dim_cfg.mode;
                dali2_hal_queue_push(DALI2_L_APP_CMD_SET_OPERATING_MODE_DTR0, &instr_data);
            }
            break;

        case DALI2_L_APP_CMD_SET_MAX_LEVEL_DTR0:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                //! Query maximum level
                instr_data.std_cmd.net.method = __dim_node.method;
                instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_MAX_LEVEL, &instr_data);
            }
            break;

        case DALI2_L_APP_CMD_QUERY_MAX_LEVEL:
            if (evt == DALI2_L_APP_EVT_SUCCESS &&
                evt_data->cmd_data.std_rsp.data == __dim_cfg.level_max) {
                //! Setting minimum level
                instr_data.std_cmd.net.method = __dim_node.method;
                instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                instr_data.std_cmd.data = __dim_cfg.level_min;
                dali2_hal_queue_push(DALI2_L_APP_CMD_SET_MIN_LEVEL_DTR0, &instr_data);
            } else {
                //! REPEAT: Setting maximum level
                instr_data.std_cmd.net.method = __dim_node.method;
                instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                instr_data.std_cmd.data = __dim_cfg.level_max;
                dali2_hal_queue_push(DALI2_L_APP_CMD_SET_MAX_LEVEL_DTR0, &instr_data);
            }
            break;

        case DALI2_L_APP_CMD_SET_MIN_LEVEL_DTR0:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                //! Setting fade time
                instr_data.std_cmd.net.method = __dim_node.method;
                instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_MIN_LEVEL, &instr_data);
            }
            break;

        case DALI2_L_APP_CMD_QUERY_MIN_LEVEL:
            if ((evt == DALI2_L_APP_EVT_SUCCESS) && (__dim_cfg_meta.phy_min == evt_data->cmd_data.std_rsp.data ||
                                                     evt_data->cmd_data.std_rsp.data == __dim_cfg.level_min)) {
                //! Setting fade time
                instr_data.std_cmd.net.method = __dim_node.method;
                instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                instr_data.std_cmd.data = 0x00;
                dali2_hal_queue_push(DALI2_L_APP_CMD_SET_FADE_TIME_DTR0, &instr_data);
            } else {
                //! REPEAT: Setting minimum level
                instr_data.std_cmd.net.method = __dim_node.method;
                instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                instr_data.std_cmd.data = __dim_cfg.level_min;
                dali2_hal_queue_push(DALI2_L_APP_CMD_SET_MIN_LEVEL_DTR0, &instr_data);
            }
            break;

        case DALI2_L_APP_CMD_SET_FADE_TIME_DTR0:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                //! Query fade time / fade rate
                instr_data.std_cmd.net.method = __dim_node.method;
                instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_FADE_TIME_FADE_RATE, &instr_data);
            } else {
                //! REPEAT: Setting fade time
                instr_data.std_cmd.net.method = __dim_node.method;
                instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                instr_data.std_cmd.data = 0x00;
                dali2_hal_queue_push(DALI2_L_APP_CMD_SET_FADE_TIME_DTR0, &instr_data);
            }
            break;

        case DALI2_L_APP_CMD_QUERY_FADE_TIME_FADE_RATE:
            if ((evt == DALI2_L_APP_EVT_SUCCESS) &&
                (!(evt_data->cmd_data.std_rsp.data & 0xF0))) {
                //! Setting Extended Fade Time
                instr_data.std_cmd.net.method = __dim_node.method;
                instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                instr_data.std_cmd.data = __dim_cfg_sec_to_ext_fade_time(__dim_cfg.fade_time_s);
                dali2_hal_queue_push(DALI2_L_APP_CMD_SET_EXTENDED_FADE_TIME_DTR0, &instr_data);
            } else {
                DALI2_L_BSP_LOG("Dimmer configuration Failed on query fade time rate!");

                //! REPEAT: Setting fade time
                instr_data.std_cmd.net.method = __dim_node.method;
                instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                instr_data.std_cmd.data = 0x00;
                dali2_hal_queue_push(DALI2_L_APP_CMD_SET_FADE_TIME_DTR0, &instr_data);
            }
            break;

        case DALI2_L_APP_CMD_SET_EXTENDED_FADE_TIME_DTR0:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                //! Query Extended Fade Time
                instr_data.std_cmd.net.method = __dim_node.method;
                instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_EXTENDED_FADE_TIME, &instr_data);
            } else {
                //! REPEAT: Setting Extended Fade Time
                instr_data.std_cmd.net.method = __dim_node.method;
                instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                instr_data.std_cmd.data = __dim_cfg_sec_to_ext_fade_time(__dim_cfg.fade_time_s);
                dali2_hal_queue_push(DALI2_L_APP_CMD_SET_EXTENDED_FADE_TIME_DTR0, &instr_data);
            }
            break;

        case DALI2_L_APP_CMD_QUERY_EXTENDED_FADE_TIME:
            if ((evt == DALI2_L_APP_EVT_SUCCESS) &&
                (evt_data->cmd_data.std_rsp.data == __dim_cfg_sec_to_ext_fade_time(__dim_cfg.fade_time_s))) {
                //! Dimmer configuration Done

                //! Free mutex
                dali2_hal_mtx_give();
            } else {
                DALI2_L_BSP_LOG("Dimmer configuration Failed on query extended fade time!");

                //! REPEAT: Setting Extended Fade Time
                instr_data.std_cmd.net.method = __dim_node.method;
                instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
                instr_data.std_cmd.data = __dim_cfg_sec_to_ext_fade_time(__dim_cfg.fade_time_s);
                dali2_hal_queue_push(DALI2_L_APP_CMD_SET_EXTENDED_FADE_TIME_DTR0, &instr_data);
            }
            break;

        case DALI2_L_APP_CMD_UNKNOWN:
            //! REPEAT: Query Device Type
            instr_data.std_cmd.net.method = __dim_node.method;
            instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
            dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_DEVICE_TYPE, &instr_data);
            break;

        default:
            break;
    }
}

dali2_ret_t dali2_hal_dim_cfg(dali2_hal_dim_cfg_t *dim_cfg, dali2_l_app_network_t *node)
{
    dali2_ret_t dali2_ret = DALI2_RET_SUCCESS;
    dali2_l_app_cmd_data_t instr_data;

    //! Verify pointer
    if (!dim_cfg) {
        dali2_ret = DALI2_RET_INVALID_PARAMS;
        goto __ret;
    }

    //! Capture mutex
    if (dali2_hal_mtx_check() != DALI2_HAL_EVT_DIM_CFG) {
        //! Try to take mutex
        if (dali2_hal_mtx_take(DALI2_HAL_EVT_DIM_CFG) != DALI2_HAL_EVT_DIM_CFG) {
            dali2_ret = DALI2_RET_BUSY;
            goto __ret;
        }
    }

    //! Copy metadata
    memcpy(&__dim_node, node, sizeof(dali2_l_app_network_t));
    memcpy(&__dim_cfg, dim_cfg, sizeof(dali2_hal_dim_cfg_t));

    //! Query Device Type
    instr_data.std_cmd.net.method = __dim_node.method;
    instr_data.std_cmd.net.addr_byte = __dim_node.addr_byte;
    dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_DEVICE_TYPE, &instr_data);

__ret:
    return dali2_ret;
}

dali2_ret_t dali2_hal_dim_meta_get(dali2_hal_dim_meta_t *dim_meta, dali2_l_app_network_t *node)
{
    dali2_ret_t dali2_ret = DALI2_RET_SUCCESS;

    //! Verify pointer
    if (!dim_meta) {
        dali2_ret = DALI2_RET_INVALID_PARAMS;
        goto __ret;
    }

    //! Check flag of metadata retrieve
    if (!__dim_cfg_meta_retrieved) {
        dali2_ret = DALI2_RET_BUSY;
        goto __ret;
    }

    //! Copy configuration metadata
    memcpy(dim_meta, &__dim_cfg_meta, sizeof(dali2_hal_dim_meta_t));

__ret:
    return dali2_ret;
}

