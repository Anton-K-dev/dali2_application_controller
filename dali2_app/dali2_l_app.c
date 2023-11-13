/**
 * @copyright
 *
 * @file    dali2_l_app.c
 * @author  Anton K.
 * @date    02 Sep 2021
 *
 * @brief   DALI-2 Application Controller.
 *          Application layer source file
 */

#include <string.h>

#include "dali2_l_app.h"

#include "dali2_spec_cmd_list.h"
#include "dali2_std_cmd_list.h"
#include "dali2_led_cmd_list.h"


typedef enum {
    DALI2_L_APP_STATE_IDLE,
    DALI2_L_APP_STATE_BUSY
} DALI2_L_APP_STATE_T;

typedef struct {
    DALI2_L_APP_STATE_T state;
    dali2_l_app_evt_data_t evt_data;
    dali2_l_app_evt_func_t evt_cb;

    unsigned char is_dtr:1;
    unsigned char is_init:1;
} __app_handle_t;

static __app_handle_t __app_handle;

void dali2_l_app_timer_cb_handler(void)
{
    dali2_ret_t dali2_ret;
    //! Check DTR0 flag
    if (__app_handle.is_dtr) {
        __app_handle.state = DALI2_L_APP_STATE_IDLE;
        if (DALI2_RET_SUCCESS !=
            (dali2_ret = dali2_l_app_cmd_execute(__app_handle.evt_data.cmd, &__app_handle.evt_data.cmd_data))) {
            __app_handle.evt_cb(DALI2_L_APP_EVT_FAULT, &__app_handle.evt_data);
        }
    }

    switch (__app_handle.evt_data.cmd) {
        case DALI2_L_APP_CMD_RESET:
        case DALI2_L_APP_CMD_SET_OPERATING_MODE_DTR0:
        case DALI2_L_APP_CMD_SET_MAX_LEVEL_DTR0:
        case DALI2_L_APP_CMD_SET_MIN_LEVEL_DTR0:
        case DALI2_L_APP_CMD_SET_FADE_TIME_DTR0:
        case DALI2_L_APP_CMD_SET_EXTENDED_FADE_TIME_DTR0:
            __app_handle.state = DALI2_L_APP_STATE_IDLE;
            __app_handle.evt_cb(DALI2_L_APP_EVT_SUCCESS, &__app_handle.evt_data);
            break;

        default:
            break;
    }
}

static inline void __dali2_l_app_ses_done_query_evt_handler(dali2_l_ses_evt_param_t *params)
{
    switch (__app_handle.evt_data.cmd) {
        //! Standard commands
        case DALI2_L_APP_CMD_QUERY_CONTROL_GEAR_PRESENT:
        case DALI2_L_APP_CMD_QUERY_LAMP_FAILURE:
        case DALI2_L_APP_CMD_QUERY_LAMP_POWER_ON:
        case DALI2_L_APP_CMD_QUERY_LIMIT_ERROR:
        case DALI2_L_APP_CMD_QUERY_RESET_STATE:
        case DALI2_L_APP_CMD_QUERY_MISSING_SHORT_ADDRESS:
        case DALI2_L_APP_CMD_QUERY_VERSION_NUMBER:
        case DALI2_L_APP_CMD_QUERY_CONTENT_DTR0:
        case DALI2_L_APP_CMD_QUERY_DEVICE_TYPE:
        case DALI2_L_APP_CMD_QUERY_PHYSICAL_MINIMUM:
        case DALI2_L_APP_CMD_QUERY_POWER_FAILURE:
        case DALI2_L_APP_CMD_QUERY_CONTENT_DTR1:
        case DALI2_L_APP_CMD_QUERY_CONTENT_DTR2:
        case DALI2_L_APP_CMD_QUERY_OPERATING_MODE:
        case DALI2_L_APP_CMD_QUERY_LIGHT_SOURCE_TYPE:

        case DALI2_L_APP_CMD_QUERY_ACTUAL_LEVEL:
        case DALI2_L_APP_CMD_QUERY_MAX_LEVEL:
        case DALI2_L_APP_CMD_QUERY_MIN_LEVEL:
        case DALI2_L_APP_CMD_QUERY_POWER_ON_LEVEL:
        case DALI2_L_APP_CMD_QUERY_SYSTEM_FAILURE_LEVEL:
        case DALI2_L_APP_CMD_QUERY_FADE_TIME_FADE_RATE:
        case DALI2_L_APP_CMD_QUERY_MANUFACTURER_SPECIFIC_MODE:
        case DALI2_L_APP_CMD_QUERY_NEXT_DEVICE_TYPE:
        case DALI2_L_APP_CMD_QUERY_EXTENDED_FADE_TIME:
        case DALI2_L_APP_CMD_QUERY_CONTROL_GEAR_FAILURE:

        //! LED commands
        case DALI2_L_APP_CMD_QUERY_GEAR_TYPE:
        case DALI2_L_APP_CMD_QUERY_DIMMING_CURVE:
        case DALI2_L_APP_CMD_QUERY_POSSIBLE_OPERATING_MODES:
        case DALI2_L_APP_CMD_QUERY_FEATURES:
        case DALI2_L_APP_CMD_QUERY_FAILURE_STATUS:
        case DALI2_L_APP_CMD_QUERY_SHORT_CIRCUIT:
        case DALI2_L_APP_CMD_QUERY_OPEN_CIRCUIT:
        case DALI2_L_APP_CMD_QUERY_LOAD_DECREASE:
        case DALI2_L_APP_CMD_QUERY_LOAD_INCREASE:
        case DALI2_L_APP_CMD_QUERY_CURRENT_PROTECTOR_ACTIVE:
        case DALI2_L_APP_CMD_QUERY_THERMAL_SHUT_DOWN:
        case DALI2_L_APP_CMD_QUERY_THERMAL_OVERLOAD:
        case DALI2_L_APP_CMD_QUERY_REFERENCE_RUNNING:
        case DALI2_L_APP_CMD_QUERY_REFERENCE_MEASUREMENT_FAILED:
        case DALI2_L_APP_CMD_QUERY_CURRENT_PROTECTOR_ENABLED:
        case DALI2_L_APP_CMD_QUERY_OPERATING_MODE_LED:
        case DALI2_L_APP_CMD_QUERY_FAST_FADE_TIME:
        case DALI2_L_APP_CMD_QUERY_MIN_FAST_FADE_TIME:
        case DALI2_L_APP_CMD_QUERY_EXTENDED_VERSION_NUMBER:
            __app_handle.evt_data.cmd_data.std_rsp.data = (unsigned char) params->msg_data;
            break;

            //! Special commands
        case DALI2_L_APP_CMD_COMPARE:
        case DALI2_L_APP_CMD_VERIFY_SHORT_ADDRESS:
        case DALI2_L_APP_CMD_QUERY_SHORT_ADDRESS:
            __app_handle.evt_data.cmd_data.spec_rsp = (unsigned char) params->msg_data;
            break;

        default:
            break;
    }

    //! Here is Command done
    __app_handle.state = DALI2_L_APP_STATE_IDLE;
    __app_handle.evt_cb(DALI2_L_APP_EVT_SUCCESS, &__app_handle.evt_data);
}

static void __dali2_l_app_ses_evt_handler(DALI2_L_SES_EVT_T evt, dali2_l_ses_evt_param_t *params)
{
    switch (evt) {
        case DALI2_L_SES_EVT_DONE:
            switch (params->msg) {
                case DALI2_L_SES_SEND:
                    if (__app_handle.is_dtr) {
                        dali2_l_bsp_app_timer_start_ms(DALI2_L_SES_SETTLING_TIME_FW_FW_MS_MAX);
                        break;
                    }

                    //! Continue
                case DALI2_L_SES_SEND_TWICE:

                    //! Delay for some commands
                    switch (__app_handle.evt_data.cmd) {
                        case DALI2_L_APP_CMD_RESET:
                            dali2_l_bsp_app_timer_start_ms(DALI2_L_APP_CMD_RESET_SETTLING_TIME_MS);
                            return;

                        case DALI2_L_APP_CMD_SET_OPERATING_MODE_DTR0:
                        case DALI2_L_APP_CMD_SET_MAX_LEVEL_DTR0:
                        case DALI2_L_APP_CMD_SET_MIN_LEVEL_DTR0:
                        case DALI2_L_APP_CMD_SET_FADE_TIME_DTR0:
                        case DALI2_L_APP_CMD_SET_EXTENDED_FADE_TIME_DTR0:
                            dali2_l_bsp_app_timer_start_ms(DALI2_L_APP_CMD_DEFAULT_SETTLING_TIME_MS);
                            return;

                        default:
                            break;
                    }

                    __app_handle.state = DALI2_L_APP_STATE_IDLE;
                    __app_handle.evt_cb(DALI2_L_APP_EVT_SUCCESS, &__app_handle.evt_data);
                    break;

                case DALI2_L_SES_QUERY:
                    //! Here is Query done
                    __dali2_l_app_ses_done_query_evt_handler(params);
                    break;

                default:
                    break;
            }
            break;

        case DALI2_L_SES_EVT_COLLISION:
            __app_handle.state = DALI2_L_APP_STATE_IDLE;
            __app_handle.evt_cb(DALI2_L_APP_EVT_FAULT, &__app_handle.evt_data);
            break;

        case DALI2_L_SES_EVT_TIMEOUT:
            __app_handle.state = DALI2_L_APP_STATE_IDLE;
            __app_handle.evt_cb(DALI2_L_APP_EVT_TIMEOUT, &__app_handle.evt_data);
            break;

        case DALI2_L_SES_EVT_UNEXPECTED_FRAME:
            __app_handle.state = DALI2_L_APP_STATE_IDLE;
            __app_handle.evt_data.cmd = DALI2_L_APP_CMD_UNKNOWN;
            __app_handle.evt_data.cmd_data.unexpected_rsp = params->msg_data;
            __app_handle.evt_cb(DALI2_L_APP_EVT_SUCCESS, &__app_handle.evt_data);
            break;

        default:
            //! Do nothing. Shouldn't be here.
            break;
    }
}

dali2_ret_t dali2_l_app_init(dali2_l_app_evt_func_t cb)
{
    dali2_ret_t dali2_ret = DALI2_RET_SUCCESS;

    //! Verify state
    if (__app_handle.is_init) {
        dali2_ret = DALI2_RET_INTERNAL_ERROR;
        goto __ret;
    }

    //! Verify parameters
    if (!cb) {
        dali2_ret = DALI2_RET_INVALID_PARAMS;
        goto __ret;
    }

    //! Call into Session layer
    dali2_ret = dali2_l_ses_init(__dali2_l_app_ses_evt_handler);
    if (dali2_ret != DALI2_RET_SUCCESS) goto __ret;

    __app_handle.state = DALI2_L_APP_STATE_IDLE;
    __app_handle.evt_cb = cb;
    __app_handle.is_dtr = 0;
    __app_handle.is_init = 1;

__ret:
    return dali2_ret;
}

dali2_ret_t dali2_l_app_deinit(void)
{
    dali2_ret_t dali2_ret = DALI2_RET_SUCCESS;

    //! Verify state
    if (!__app_handle.is_init) {
        dali2_ret = DALI2_RET_INTERNAL_ERROR;
        goto __ret;
    }

    //! Call into Session layer
    dali2_ret = dali2_l_ses_deinit();
    if (dali2_ret != DALI2_RET_SUCCESS) goto __ret;

    __app_handle.evt_cb = 0;
    __app_handle.is_init = 0;

__ret:
    return dali2_ret;
}

static inline dali2_ret_t __terminate_execute(unsigned long int *frame, dali2_l_app_cmd_data_t *cmd_data)
{
    dali2_ret_t dali2_ret = DALI2_RET_SUCCESS;

    dali2_ret = dali2_l_pres_16bit_encode(frame, DALI2_L_APP_SPEC_CMD_TERMINATE, 0x00);
    if (dali2_ret != DALI2_RET_SUCCESS) goto __ret;

    dali2_ret = dali2_l_ses_exec(DALI2_L_SES_SEND, DALI2_L_PHY_FRAME_16BIT_FW, *frame);
__ret:
    return dali2_ret;
}

static inline dali2_ret_t __dtr0_execute(unsigned long int *frame, dali2_l_app_cmd_data_t *cmd_data)
{
    dali2_ret_t dali2_ret = DALI2_RET_SUCCESS;

    dali2_ret = dali2_l_pres_16bit_encode(frame, DALI2_L_APP_SPEC_CMD_DTR0, cmd_data->spec_cmd.dtr0);
    if (dali2_ret != DALI2_RET_SUCCESS) goto __ret;

    dali2_ret = dali2_l_ses_exec(DALI2_L_SES_SEND, DALI2_L_PHY_FRAME_16BIT_FW, *frame);
__ret:
    return dali2_ret;
}

static inline dali2_ret_t __initialise_execute(unsigned long int *frame, dali2_l_app_cmd_data_t *cmd_data)
{
    dali2_ret_t dali2_ret = DALI2_RET_SUCCESS;
    unsigned char initialise_device;

    switch (cmd_data->spec_cmd.initialise.addressing) {
        case DALI2_APP_CMD_INITIALISE_ADDRESSING_SHORT_ADDRESS:
            //! Verify short address
            if (cmd_data->spec_cmd.initialise.addr >= DALI2_L_NET_ADDR_SHORT_MAX) {
                dali2_ret = DALI2_RET_INVALID_PARAMS;
                goto __ret;
            }
            initialise_device = ((cmd_data->spec_cmd.initialise.addr << 1) | 0x01) & 0x7F;
            break;

        case DALI2_APP_CMD_INITIALISE_ADDRESSING_NO_ADDRESS:
            initialise_device = 0xFF;
            break;

        case DALI2_APP_CMD_INITIALISE_ADDRESSING_ALL:
            initialise_device = 0x00;
            break;

        default:
            dali2_ret = DALI2_RET_INVALID_PARAMS;
            goto __ret;
    }

    dali2_ret = dali2_l_pres_16bit_encode(frame, DALI2_L_APP_SPEC_CMD_INITIALISE, initialise_device);
    if (dali2_ret != DALI2_RET_SUCCESS) goto __ret;

    dali2_ret = dali2_l_ses_exec(DALI2_L_SES_SEND_TWICE, DALI2_L_PHY_FRAME_16BIT_FW, *frame);
__ret:
    return dali2_ret;
}

static inline dali2_ret_t __randomise_execute(unsigned long int *frame, dali2_l_app_cmd_data_t *cmd_data)
{
    dali2_ret_t dali2_ret = DALI2_RET_SUCCESS;

    dali2_ret = dali2_l_pres_16bit_encode(frame, DALI2_L_APP_SPEC_CMD_RANDOMISE, 0x00);
    if (dali2_ret != DALI2_RET_SUCCESS) goto __ret;

    dali2_ret = dali2_l_ses_exec(DALI2_L_SES_SEND_TWICE, DALI2_L_PHY_FRAME_16BIT_FW, *frame);
__ret:
    return dali2_ret;
}

static inline dali2_ret_t __compare_execute(unsigned long int *frame, dali2_l_app_cmd_data_t *cmd_data)
{
    dali2_ret_t dali2_ret = DALI2_RET_SUCCESS;

    dali2_ret = dali2_l_pres_16bit_encode(frame, DALI2_L_APP_SPEC_CMD_COMPARE, 0x00);
    if (dali2_ret != DALI2_RET_SUCCESS) goto __ret;

    dali2_ret = dali2_l_ses_exec(DALI2_L_SES_QUERY, DALI2_L_PHY_FRAME_16BIT_FW, *frame);
__ret:
    return dali2_ret;
}

static inline dali2_ret_t __withdraw_execute(unsigned long int *frame, dali2_l_app_cmd_data_t *cmd_data)
{
    dali2_ret_t dali2_ret = DALI2_RET_SUCCESS;

    dali2_ret = dali2_l_pres_16bit_encode(frame, DALI2_L_APP_SPEC_CMD_WITHDRAW, 0x00);
    if (dali2_ret != DALI2_RET_SUCCESS) goto __ret;

    dali2_ret = dali2_l_ses_exec(DALI2_L_SES_SEND, DALI2_L_PHY_FRAME_16BIT_FW, *frame);
__ret:
    return dali2_ret;
}

static inline dali2_ret_t __ping_execute(unsigned long int *frame, dali2_l_app_cmd_data_t *cmd_data)
{
    dali2_ret_t dali2_ret = DALI2_RET_SUCCESS;

    dali2_ret = dali2_l_pres_16bit_encode(frame, DALI2_L_APP_SPEC_CMD_PING, 0x00);
    if (dali2_ret != DALI2_RET_SUCCESS) goto __ret;

    dali2_ret = dali2_l_ses_exec(DALI2_L_SES_SEND, DALI2_L_PHY_FRAME_16BIT_FW, *frame);
__ret:
    return dali2_ret;
}

static inline dali2_ret_t __searchaddrh_execute(unsigned long int *frame, dali2_l_app_cmd_data_t *cmd_data)
{
    dali2_ret_t dali2_ret = DALI2_RET_SUCCESS;

    dali2_ret = dali2_l_pres_16bit_encode(frame, DALI2_L_APP_SPEC_CMD_SEARCHADDRH, cmd_data->spec_cmd.searchaddress_hml);
    if (dali2_ret != DALI2_RET_SUCCESS) goto __ret;

    dali2_ret = dali2_l_ses_exec(DALI2_L_SES_SEND, DALI2_L_PHY_FRAME_16BIT_FW, *frame);
__ret:
    return dali2_ret;
}

static inline dali2_ret_t __searchaddrm_execute(unsigned long int *frame, dali2_l_app_cmd_data_t *cmd_data)
{
    dali2_ret_t dali2_ret = DALI2_RET_SUCCESS;

    dali2_ret = dali2_l_pres_16bit_encode(frame, DALI2_L_APP_SPEC_CMD_SEARCHADDRM, cmd_data->spec_cmd.searchaddress_hml);
    if (dali2_ret != DALI2_RET_SUCCESS) goto __ret;

    dali2_ret = dali2_l_ses_exec(DALI2_L_SES_SEND, DALI2_L_PHY_FRAME_16BIT_FW, *frame);
__ret:
    return dali2_ret;
}

static inline dali2_ret_t __searchaddrl_execute(unsigned long int *frame, dali2_l_app_cmd_data_t *cmd_data)
{
    dali2_ret_t dali2_ret = DALI2_RET_SUCCESS;

    dali2_ret = dali2_l_pres_16bit_encode(frame, DALI2_L_APP_SPEC_CMD_SEARCHADDRL, cmd_data->spec_cmd.searchaddress_hml);
    if (dali2_ret != DALI2_RET_SUCCESS) goto __ret;

    dali2_ret = dali2_l_ses_exec(DALI2_L_SES_SEND, DALI2_L_PHY_FRAME_16BIT_FW, *frame);
__ret:
    return dali2_ret;
}

static inline dali2_ret_t __program_short_address_execute(unsigned long int *frame, dali2_l_app_cmd_data_t *cmd_data)
{
    dali2_ret_t dali2_ret = DALI2_RET_SUCCESS;
    unsigned char address;

    address = ((cmd_data->spec_cmd.program_short_address << 1) | 0x01) & 0x7F;
    dali2_ret = dali2_l_pres_16bit_encode(frame, DALI2_L_APP_SPEC_CMD_PROGRAM_SHORT_ADDRESS, address);
    if (dali2_ret != DALI2_RET_SUCCESS) goto __ret;

    dali2_ret = dali2_l_ses_exec(DALI2_L_SES_SEND, DALI2_L_PHY_FRAME_16BIT_FW, *frame);
__ret:
    return dali2_ret;
}

static inline dali2_ret_t __verify_short_address_execute(unsigned long int *frame, dali2_l_app_cmd_data_t *cmd_data)
{
    dali2_ret_t dali2_ret = DALI2_RET_SUCCESS;
    unsigned char address;

    address = ((cmd_data->spec_cmd.verify_short_address << 1) | 0x01) & 0x7F;
    dali2_ret = dali2_l_pres_16bit_encode(frame, DALI2_L_APP_SPEC_CMD_VERIFY_SHORT_ADDRESS, address);
    if (dali2_ret != DALI2_RET_SUCCESS) goto __ret;

    dali2_ret = dali2_l_ses_exec(DALI2_L_SES_QUERY, DALI2_L_PHY_FRAME_16BIT_FW, *frame);
__ret:
    return dali2_ret;
}

static inline dali2_ret_t __query_short_address_execute(unsigned long int *frame, dali2_l_app_cmd_data_t *cmd_data)
{
    dali2_ret_t dali2_ret = DALI2_RET_SUCCESS;

    dali2_ret = dali2_l_pres_16bit_encode(frame, DALI2_L_APP_SPEC_CMD_QUERY_SHORT_ADDRESS, 0x00);
    if (dali2_ret != DALI2_RET_SUCCESS) goto __ret;

    dali2_ret = dali2_l_ses_exec(DALI2_L_SES_QUERY, DALI2_L_PHY_FRAME_16BIT_FW, *frame);
__ret:
    return dali2_ret;
}

static inline dali2_ret_t __dapc_execute(unsigned long int *frame, dali2_l_app_cmd_data_t *cmd_data)
{
    dali2_ret_t dali2_ret = DALI2_RET_SUCCESS;
    unsigned char net_byte;

    dali2_ret = dali2_l_net_encode(DALI2_L_NET_SELECTOR_DAPC, cmd_data->std_cmd.net.method,
            cmd_data->std_cmd.net.addr_byte, &net_byte);
    if (dali2_ret != DALI2_RET_SUCCESS) goto __ret;

    dali2_ret = dali2_l_pres_16bit_encode(frame, net_byte, cmd_data->std_cmd.data);
    if (dali2_ret != DALI2_RET_SUCCESS) goto __ret;

    dali2_ret = dali2_l_ses_exec(DALI2_L_SES_SEND, DALI2_L_PHY_FRAME_16BIT_FW, *frame);
__ret:
    return dali2_ret;
}

static inline dali2_ret_t __query_enable_device_type_6_execute(unsigned long int *frame, dali2_l_app_cmd_data_t *cmd_data)
{
    dali2_ret_t dali2_ret = DALI2_RET_SUCCESS;

    dali2_ret = dali2_l_ses_exec(DALI2_L_SES_SEND_TWICE, DALI2_L_PHY_FRAME_16BIT_FW, DALI2_L_APP_LED_CMD_ENABLE_DEVICE_TYPE_6);
    return dali2_ret;
}

dali2_ret_t dali2_l_app_cmd_execute(DALI2_L_APP_CMD_T cmd, dali2_l_app_cmd_data_t *cmd_data)
{
    dali2_ret_t dali2_ret = DALI2_RET_SUCCESS;
    DALI2_L_SES_MSG_T session_method;
    DALI2_L_APP_STD_CMD_T std_cmd;
    unsigned char net_byte;
    unsigned long int frame;
    dali2_l_app_cmd_data_t cmd_data_dtr0;

    //! Verify instruction data pointer
    if (!cmd_data) {
        dali2_ret = DALI2_RET_INVALID_PARAMS;
        goto __ret;
    }

    //! Verify Application layer state
    if (__app_handle.state != DALI2_L_APP_STATE_IDLE) {
        dali2_ret = DALI2_RET_BUSY;
        goto __ret;
    }

    //! Copy Inputs to internal structure
    __app_handle.evt_data.cmd = cmd;
    memcpy(&__app_handle.evt_data.cmd_data, cmd_data, sizeof(dali2_l_app_cmd_data_t));

    switch (cmd) {
        //! ***** Special commands here *****
        case DALI2_L_APP_CMD_TERMINATE:
            dali2_ret = __terminate_execute(&frame, cmd_data);
            goto __ret;
        case DALI2_L_APP_CMD_DTR0:
            dali2_ret = __dtr0_execute(&frame, cmd_data);
            goto __ret;
        case DALI2_L_APP_CMD_INITIALISE:
            dali2_ret = __initialise_execute(&frame, cmd_data);
            goto __ret;
        case DALI2_L_APP_CMD_RANDOMISE:
            dali2_ret = __randomise_execute(&frame, cmd_data);
            goto __ret;
        case DALI2_L_APP_CMD_COMPARE:
            dali2_ret = __compare_execute(&frame, cmd_data);
            goto __ret;
        case DALI2_L_APP_CMD_WITHDRAW:
            dali2_ret = __withdraw_execute(&frame, cmd_data);
            goto __ret;
        case DALI2_L_APP_CMD_PING:
            dali2_ret = __ping_execute(&frame, cmd_data);
            goto __ret;
        case DALI2_L_APP_CMD_SEARCHADDRH:
            dali2_ret = __searchaddrh_execute(&frame, cmd_data);
            goto __ret;
        case DALI2_L_APP_CMD_SEARCHADDRM:
            dali2_ret = __searchaddrm_execute(&frame, cmd_data);
            goto __ret;
        case DALI2_L_APP_CMD_SEARCHADDRL:
            dali2_ret = __searchaddrl_execute(&frame, cmd_data);
            goto __ret;
        case DALI2_L_APP_CMD_PROGRAM_SHORT_ADDRESS:
            dali2_ret = __program_short_address_execute(&frame, cmd_data);
            goto __ret;
        case DALI2_L_APP_CMD_VERIFY_SHORT_ADDRESS:
            dali2_ret = __verify_short_address_execute(&frame, cmd_data);
            goto __ret;
        case DALI2_L_APP_CMD_QUERY_SHORT_ADDRESS:
            dali2_ret = __query_short_address_execute(&frame, cmd_data);
            goto __ret;

        //! ***** Standard commands here *****
        case DALI2_L_APP_CMD_DAPC:  //! Deprecated
            dali2_ret = __dapc_execute(&frame, cmd_data);
            goto __ret;

        case DALI2_L_APP_CMD_OFF:
            std_cmd = DALI2_L_APP_STD_CMD_OFF;
            session_method = DALI2_L_SES_SEND;
            break;

        case DALI2_L_APP_CMD_UP:
            std_cmd = DALI2_L_APP_STD_CMD_UP;
            session_method = DALI2_L_SES_SEND;
            break;

        case DALI2_L_APP_CMD_DOWN:
            std_cmd = DALI2_L_APP_STD_CMD_DOWN;
            session_method = DALI2_L_SES_SEND;
            break;

        case DALI2_L_APP_CMD_STEP_UP:
            std_cmd = DALI2_L_APP_STD_CMD_STEP_UP;
            session_method = DALI2_L_SES_SEND;
            break;

        case DALI2_L_APP_CMD_STEP_DOWN:
            std_cmd = DALI2_L_APP_STD_CMD_STEP_DOWN;
            session_method = DALI2_L_SES_SEND;
            break;

        case DALI2_L_APP_CMD_RECALL_MAX_LEVEL:
            std_cmd = DALI2_L_APP_STD_CMD_RECALL_MAX_LEVEL;
            session_method = DALI2_L_SES_SEND;
            break;

        case DALI2_L_APP_CMD_RECALL_MIN_LEVEL:
            std_cmd = DALI2_L_APP_STD_CMD_RECALL_MIN_LEVEL;
            session_method = DALI2_L_SES_SEND;
            break;

        case DALI2_L_APP_CMD_STEP_DOWN_AND_OFF:
            std_cmd = DALI2_L_APP_STD_CMD_STEP_DOWN_AND_OFF;
            session_method = DALI2_L_SES_SEND;
            break;

        case DALI2_L_APP_CMD_ON_AND_STEP_UP:
            std_cmd = DALI2_L_APP_STD_CMD_ON_AND_STEP_UP;
            session_method = DALI2_L_SES_SEND;
            break;

        case DALI2_L_APP_CMD_ENABLE_DAPC_SEQUENCE:
            std_cmd = DALI2_L_APP_STD_CMD_ENABLE_DAPC_SEQUENCE;
            session_method = DALI2_L_SES_SEND;
            break;

        case DALI2_L_APP_CMD_GO_TO_LAST_ACTIVE_LEVEL:
            std_cmd = DALI2_L_APP_STD_CMD_GO_TO_LAST_ACTIVE_LEVEL;
            session_method = DALI2_L_SES_SEND;
            break;

        case DALI2_L_APP_CMD_GO_TO_SCENE:
            if (cmd_data->std_cmd.data >= DALI2_L_APP_CMD_SCENE_COUNT) {
                dali2_ret = DALI2_RET_INVALID_PARAMS;
                goto __ret;
            }
            std_cmd = DALI2_L_APP_STD_CMD_GO_TO_SCENE + cmd_data->std_cmd.data;
            session_method = DALI2_L_SES_SEND;
            break;

        case DALI2_L_APP_CMD_RESET:
            std_cmd = DALI2_L_APP_STD_CMD_RESET;
            session_method = DALI2_L_SES_SEND_TWICE;
            break;

        case DALI2_L_APP_CMD_SET_OPERATING_MODE_DTR0:
            __app_handle.is_dtr ^= 0x01;
            if (__app_handle.is_dtr) {
                goto __ret;
            }

            std_cmd = DALI2_L_APP_STD_CMD_SET_OPERATING_MODE;
            session_method = DALI2_L_SES_SEND_TWICE;
            break;

        case DALI2_L_APP_CMD_IDENTIFY_DEVICE:
            std_cmd = DALI2_L_APP_STD_CMD_IDENTIFY_DEVICE;
            session_method = DALI2_L_SES_SEND_TWICE;
            break;

        case DALI2_L_APP_CMD_SET_MAX_LEVEL_DTR0:
            __app_handle.is_dtr ^= 0x01;
            if (__app_handle.is_dtr) {
                goto __ret;
            }

            std_cmd = DALI2_L_APP_STD_CMD_SET_MAX_LEVEL_DTR0;
            session_method = DALI2_L_SES_SEND_TWICE;
            break;

        case DALI2_L_APP_CMD_SET_MIN_LEVEL_DTR0:
            __app_handle.is_dtr ^= 0x01;
            if (__app_handle.is_dtr) {
                goto __ret;
            }

            std_cmd = DALI2_L_APP_STD_CMD_SET_MIN_LEVEL_DTR0;
            session_method = DALI2_L_SES_SEND_TWICE;
            break;

        case DALI2_L_APP_CMD_SET_SYSTEM_FAILURE_LEVEL_DTR0:
            __app_handle.is_dtr ^= 0x01;
            if (__app_handle.is_dtr) {
                goto __ret;
            }

            std_cmd = DALI2_L_APP_STD_CMD_SET_SYSTEM_FAILURE_LEVEL_DTR0;
            session_method = DALI2_L_SES_SEND_TWICE;
            break;

        case DALI2_L_APP_CMD_SET_POWER_ON_LEVEL_DTR0:
            __app_handle.is_dtr ^= 0x01;
            if (__app_handle.is_dtr) {
                goto __ret;
            }

            std_cmd = DALI2_L_APP_STD_CMD_SET_POWER_ON_LEVEL_DTR0;
            session_method = DALI2_L_SES_SEND_TWICE;
            break;

        case DALI2_L_APP_CMD_SET_FADE_TIME_DTR0:
            __app_handle.is_dtr ^= 0x01;
            if (__app_handle.is_dtr) {
                goto __ret;
            }

            std_cmd = DALI2_L_APP_STD_CMD_SET_FADE_TIME_DTR0;
            session_method = DALI2_L_SES_SEND_TWICE;
            break;

        case DALI2_L_APP_CMD_SET_FADE_RATE_DTR0:
            __app_handle.is_dtr ^= 0x01;
            if (__app_handle.is_dtr) {
                goto __ret;
            }

            std_cmd = DALI2_L_APP_STD_CMD_SET_FADE_RATE_DTR0;
            session_method = DALI2_L_SES_SEND_TWICE;
            break;

        case DALI2_L_APP_CMD_SET_EXTENDED_FADE_TIME_DTR0:
            __app_handle.is_dtr ^= 0x01;
            if (__app_handle.is_dtr) {
                goto __ret;
            }

            std_cmd = DALI2_L_APP_STD_CMD_SET_EXTENDED_FADE_TIME_DTR0;
            session_method = DALI2_L_SES_SEND_TWICE;
            break;

        case DALI2_L_APP_CMD_SET_SHORT_ADDRESS:
            __app_handle.is_dtr ^= 0x01;
            if (__app_handle.is_dtr) {
                goto __ret;
            }

            std_cmd = DALI2_L_APP_STD_CMD_SET_SHORT_ADDRESS_DTR0;
            session_method = DALI2_L_SES_SEND_TWICE;
            break;

        case DALI2_L_APP_CMD_QUERY_STATUS:
            std_cmd = DALI2_L_APP_STD_CMD_QUERY_STATUS;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_CONTROL_GEAR_PRESENT:
            std_cmd = DALI2_L_APP_STD_CMD_QUERY_CONTROL_GEAR_PRESENT;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_LAMP_FAILURE:
            std_cmd = DALI2_L_APP_STD_CMD_QUERY_LAMP_FAILURE;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_LAMP_POWER_ON:
            std_cmd = DALI2_L_APP_STD_CMD_QUERY_LAMP_POWER_ON;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_LIMIT_ERROR:
            std_cmd = DALI2_L_APP_STD_CMD_QUERY_LIMIT_ERROR;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_RESET_STATE:
            std_cmd = DALI2_L_APP_STD_CMD_QUERY_RESET_STATE;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_MISSING_SHORT_ADDRESS:
            std_cmd = DALI2_L_APP_STD_CMD_QUERY_MISSING_SHORT_ADDRESS;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_VERSION_NUMBER:
            std_cmd = DALI2_L_APP_STD_CMD_QUERY_VERSION_NUMBER;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_CONTENT_DTR0:
            std_cmd = DALI2_L_APP_STD_CMD_QUERY_CONTENT_DTR0;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_DEVICE_TYPE:
            std_cmd = DALI2_L_APP_STD_CMD_QUERY_DEVICE_TYPE;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_PHYSICAL_MINIMUM:
            std_cmd = DALI2_L_APP_STD_CMD_QUERY_PHYSICAL_MINIMUM;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_POWER_FAILURE:
            std_cmd = DALI2_L_APP_STD_CMD_QUERY_POWER_FAILURE;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_CONTENT_DTR1:
            std_cmd = DALI2_L_APP_STD_CMD_QUERY_CONTENT_DTR1;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_CONTENT_DTR2:
            std_cmd = DALI2_L_APP_STD_CMD_QUERY_CONTENT_DTR2;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_OPERATING_MODE:
            std_cmd = DALI2_L_APP_STD_CMD_QUERY_OPERATING_MODE;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_LIGHT_SOURCE_TYPE:
            std_cmd = DALI2_L_APP_STD_CMD_QUERY_LIGHT_SOURCE_TYPE;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_ACTUAL_LEVEL:
            std_cmd = DALI2_L_APP_STD_CMD_QUERY_ACTUAL_LEVEL;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_MAX_LEVEL:
            std_cmd = DALI2_L_APP_STD_CMD_QUERY_MAX_LEVEL;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_MIN_LEVEL:
            std_cmd = DALI2_L_APP_STD_CMD_QUERY_MIN_LEVEL;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_POWER_ON_LEVEL:
            std_cmd = DALI2_L_APP_STD_CMD_QUERY_POWER_ON_LEVEL;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_SYSTEM_FAILURE_LEVEL:
            std_cmd = DALI2_L_APP_STD_CMD_QUERY_SYSTEM_FAILURE_LEVEL;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_FADE_TIME_FADE_RATE:
            std_cmd = DALI2_L_APP_STD_CMD_QUERY_FADE_TIME_FADE_RATE;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_MANUFACTURER_SPECIFIC_MODE:
            std_cmd = DALI2_L_APP_STD_CMD_QUERY_MANUFACTURER_SPECIFIC_MODE;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_NEXT_DEVICE_TYPE:
            std_cmd = DALI2_L_APP_STD_CMD_QUERY_NEXT_DEVICE_TYPE;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_EXTENDED_FADE_TIME:
            std_cmd = DALI2_L_APP_STD_CMD_QUERY_EXTENDED_FADE_TIME;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_CONTROL_GEAR_FAILURE:
            std_cmd = DALI2_L_APP_STD_CMD_QUERY_CONTROL_GEAR_FAILURE;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_REFERENCE_SYSTEM_POWER:
            std_cmd = DALI2_L_APP_LED_CMD_REFERENCE_SYSTEM_POWER;
            session_method = DALI2_L_SES_SEND_TWICE;
            break;

        case DALI2_L_APP_CMD_ENABLE_CURRENT_PROTECTOR:
            std_cmd = DALI2_L_APP_LED_CMD_ENABLE_CURRENT_PROTECTOR;
            session_method = DALI2_L_SES_SEND_TWICE;
            break;

        case DALI2_L_APP_CMD_DISABLE_CURRENT_PROTECTOR:
            std_cmd = DALI2_L_APP_LED_CMD_DISABLE_CURRENT_PROTECTOR;
            session_method = DALI2_L_SES_SEND_TWICE;
            break;

        case DALI2_L_APP_CMD_SELECT_DIMMING_CURVE:
            __app_handle.is_dtr ^= 0x01;
            if (__app_handle.is_dtr) {
                goto __ret;
            }

            std_cmd = DALI2_L_APP_LED_CMD_SELECT_DIMMING_CURVE;
            session_method = DALI2_L_SES_SEND_TWICE;
            break;

        case DALI2_L_APP_CMD_STORE_DTR_AS_FAST_FADE_TIME:
            __app_handle.is_dtr ^= 0x01;
            if (__app_handle.is_dtr) {
                goto __ret;
            }

            std_cmd = DALI2_L_APP_LED_CMD_STORE_DTR_AS_FAST_FADE_TIME;
            session_method = DALI2_L_SES_SEND_TWICE;
            break;

        case DALI2_L_APP_CMD_QUERY_GEAR_TYPE:
            std_cmd = DALI2_L_APP_LED_CMD_QUERY_GEAR_TYPE;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_DIMMING_CURVE:
            std_cmd = DALI2_L_APP_LED_CMD_QUERY_DIMMING_CURVE;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_POSSIBLE_OPERATING_MODES:
            std_cmd = DALI2_L_APP_LED_CMD_QUERY_POSSIBLE_OPERATING_MODES;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_FEATURES:
            std_cmd = DALI2_L_APP_LED_CMD_QUERY_FEATURES;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_FAILURE_STATUS:
            std_cmd = DALI2_L_APP_LED_CMD_QUERY_FAILURE_STATUS;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_SHORT_CIRCUIT:
            std_cmd = DALI2_L_APP_LED_CMD_QUERY_SHORT_CIRCUIT;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_OPEN_CIRCUIT:
            std_cmd = DALI2_L_APP_LED_CMD_QUERY_OPEN_CIRCUIT;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_LOAD_DECREASE:
            std_cmd = DALI2_L_APP_LED_CMD_QUERY_LOAD_DECREASE;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_LOAD_INCREASE:
            std_cmd = DALI2_L_APP_LED_CMD_QUERY_LOAD_INCREASE;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_CURRENT_PROTECTOR_ACTIVE:
            std_cmd = DALI2_L_APP_LED_CMD_QUERY_CURRENT_PROTECTOR_ACTIVE;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_THERMAL_SHUT_DOWN:
            std_cmd = DALI2_L_APP_LED_CMD_QUERY_THERMAL_SHUT_DOWN;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_THERMAL_OVERLOAD:
            std_cmd = DALI2_L_APP_LED_CMD_QUERY_THERMAL_OVERLOAD;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_REFERENCE_RUNNING:
            std_cmd = DALI2_L_APP_LED_CMD_QUERY_REFERENCE_RUNNING;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_REFERENCE_MEASUREMENT_FAILED:
            std_cmd = DALI2_L_APP_LED_CMD_QUERY_REFERENCE_MEASUREMENT_FAILED;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_CURRENT_PROTECTOR_ENABLED:
            std_cmd = DALI2_L_APP_LED_CMD_QUERY_CURRENT_PROTECTOR_ENABLED;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_OPERATING_MODE_LED:
            std_cmd = DALI2_L_APP_LED_CMD_QUERY_OPERATING_MODE;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_FAST_FADE_TIME:
            std_cmd = DALI2_L_APP_LED_CMD_QUERY_FAST_FADE_TIME;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_MIN_FAST_FADE_TIME:
            std_cmd = DALI2_L_APP_LED_CMD_QUERY_MIN_FAST_FADE_TIME;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_QUERY_EXTENDED_VERSION_NUMBER:
            std_cmd = DALI2_L_APP_LED_CMD_QUERY_EXTENDED_VERSION_NUMBER;
            session_method = DALI2_L_SES_QUERY;
            break;

        case DALI2_L_APP_CMD_ENABLE_DEVICE_TYPE_6:
            dali2_ret = __query_enable_device_type_6_execute(&frame, cmd_data);
            goto __ret;

        default:
            dali2_ret = DALI2_RET_INVALID_PARAMS;
            goto __ret;
    }

    dali2_ret = dali2_l_net_encode(DALI2_L_NET_SELECTOR_OTHER, cmd_data->std_cmd.net.method,
            cmd_data->std_cmd.net.addr_byte, &net_byte);
    if (dali2_ret != DALI2_RET_SUCCESS) goto __ret;

    dali2_ret = dali2_l_pres_16bit_encode(&frame, net_byte, std_cmd);
    if (dali2_ret != DALI2_RET_SUCCESS) goto __ret;

    dali2_ret = dali2_l_ses_exec(session_method, DALI2_L_PHY_FRAME_16BIT_FW, frame);

__ret:

    //! DTR0 trigger
    if (__app_handle.is_dtr) {
        cmd_data_dtr0.spec_cmd.dtr0 = cmd_data->std_cmd.data;
        dali2_ret = __dtr0_execute(&frame, &cmd_data_dtr0);
    }

    if (dali2_ret == DALI2_RET_SUCCESS) {
        //! Set busy state for Application layer
        __app_handle.state = DALI2_L_APP_STATE_BUSY;
    }

    return dali2_ret;
}
