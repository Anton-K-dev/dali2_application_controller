/**
 * @copyright
 *
 * @file    dali2_addr_alloc.c
 * @author  Anton K.
 * @date    15 Sep 2021
 *
 * @brief   DALI-2 Application Controller.
 *          HAL Address allocation method source file
 */

#include "dali2_hal.h"
#include "dali2_hal_internal.h"

static DALI2_HAL_ADDR_ALLOC_METHOD_T __addr_alloc_method = DALI2_HAL_ADDR_ALLOC_METHOD_UNKNOWN;

static unsigned char __addr_list[DALI2_HAL_ADDR_LIST_SIZE];
static unsigned int __addr_count;

static inline void __single_addr_alloc_dispatch(DALI2_L_APP_EVT_T evt, dali2_l_app_evt_data_t *evt_data)
{
    dali2_l_app_cmd_data_t instr_data;

    switch (evt_data->cmd) {
        case DALI2_L_APP_CMD_RESET:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                //! Assign single address
                instr_data.std_cmd.net.method = DALI2_L_NET_METHOD_BROADCAST;
                instr_data.std_cmd.net.addr_byte = __addr_list[__addr_count];
                instr_data.std_cmd.data = DALI2_L_APP_DTR0_TO_SET_SHORT_ADDRESS(__addr_list[__addr_count]);
                dali2_hal_queue_push(DALI2_L_APP_CMD_SET_SHORT_ADDRESS, &instr_data);
            } else {
                //! REPEAT: Reset
                instr_data.std_cmd.net.method = DALI2_L_NET_METHOD_BROADCAST;
                instr_data.std_cmd.net.addr_byte = __addr_list[__addr_count];
                dali2_hal_queue_push(DALI2_L_APP_CMD_RESET, &instr_data);
            }
            break;

        case DALI2_L_APP_CMD_SET_SHORT_ADDRESS:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                //! Verify address
                instr_data.std_cmd.net.method = DALI2_L_NET_METHOD_SHORT_ADDRESSING;
                instr_data.std_cmd.net.addr_byte = __addr_list[__addr_count];
                dali2_hal_queue_push(DALI2_L_APP_CMD_QUERY_CONTENT_DTR0, &instr_data);
            } else {
                //! REPEAT: Assign single address
                instr_data.std_cmd.net.method = DALI2_L_NET_METHOD_BROADCAST;
                instr_data.std_cmd.net.addr_byte = __addr_list[__addr_count];
                instr_data.std_cmd.data = DALI2_L_APP_DTR0_TO_SET_SHORT_ADDRESS(__addr_list[__addr_count]);
                dali2_hal_queue_push(DALI2_L_APP_CMD_SET_SHORT_ADDRESS, &instr_data);
            }
            break;

        case DALI2_L_APP_CMD_QUERY_CONTENT_DTR0:
            if (evt == DALI2_L_APP_EVT_SUCCESS) {
                //! Assigning single address completed!
                __addr_count = 1;
                __addr_alloc_method = DALI2_HAL_ADDR_ALLOC_METHOD_UNKNOWN;

                //! Free mutex
                dali2_hal_mtx_give();
            } else {
                //! REPEAT: Reset
                instr_data.std_cmd.net.method = DALI2_L_NET_METHOD_BROADCAST;
                instr_data.std_cmd.net.addr_byte = __addr_list[__addr_count];
                dali2_hal_queue_push(DALI2_L_APP_CMD_RESET, &instr_data);
            }
            break;

        default:
            //! REPEAT: Reset
            instr_data.std_cmd.net.method = DALI2_L_NET_METHOD_BROADCAST;
            instr_data.std_cmd.net.addr_byte = __addr_list[__addr_count];
            dali2_hal_queue_push(DALI2_L_APP_CMD_RESET, &instr_data);
            break;
    }
}

static inline void __random_addr_prepare_next(void)
{
    if (__addr_count < DALI2_L_NET_ADDR_SHORT_MAX) {
        __addr_count++;
        __addr_list[__addr_count] = __addr_list[__addr_count - 1] + 1;
    } else {    //! Critical state. Terminate address allocation immediately.
        __addr_alloc_method = DALI2_HAL_ADDR_ALLOC_METHOD_UNKNOWN;

        //! Free mutex
        dali2_hal_mtx_give();
    }
}

static inline void __random_addr_alloc_dispatch(DALI2_L_APP_EVT_T evt, dali2_l_app_evt_data_t *evt_data)
{
    dali2_l_app_cmd_data_t instr_data;

    if (evt != DALI2_L_APP_EVT_SUCCESS) {
        if (evt_data->cmd == DALI2_L_APP_CMD_COMPARE) {
            //! Prepare next address
            __random_addr_prepare_next();

            //! Identification forced. Search Next Low Address [c]
            instr_data.spec_cmd.searchaddress_hml = __addr_list[__addr_count];
            dali2_hal_queue_push(DALI2_L_APP_CMD_SEARCHADDRL, &instr_data);
        } else {
            //! Initialization failed
            //! Give mutex
            dali2_hal_mtx_give();
            return;
        }
    }

    switch (evt_data->cmd) {
        case DALI2_L_APP_CMD_INITIALISE:
            //! Initialization opened. RANDOMISE addresses [b]
            __addr_list[__addr_count] = evt_data->cmd_data.spec_cmd.initialise.addr;
            dali2_hal_queue_push(DALI2_L_APP_CMD_RANDOMISE, &instr_data);
            break;

        case DALI2_L_APP_CMD_RANDOMISE:
            //! RANDOMISE completed. Search Low Address [c]
            instr_data.spec_cmd.searchaddress_hml = __addr_list[__addr_count];
            dali2_hal_queue_push(DALI2_L_APP_CMD_SEARCHADDRL, &instr_data);
            break;

        case DALI2_L_APP_CMD_SEARCHADDRL:
            //! Search address completed. COMPARE address [c]
            dali2_hal_queue_push(DALI2_L_APP_CMD_COMPARE, &instr_data);
            break;

        case DALI2_L_APP_CMD_COMPARE:
            //! COMPARE is found address. Program this short address [d]
            instr_data.spec_cmd.program_short_address = __addr_list[__addr_count];
            dali2_hal_queue_push(DALI2_L_APP_CMD_PROGRAM_SHORT_ADDRESS, &instr_data);
            break;

        case DALI2_L_APP_CMD_PROGRAM_SHORT_ADDRESS:
            //! Programming short address completed. Verify short address [e]
            instr_data.spec_cmd.verify_short_address = __addr_list[__addr_count];
            dali2_hal_queue_push(DALI2_L_APP_CMD_VERIFY_SHORT_ADDRESS, &instr_data);
            break;

        case DALI2_L_APP_CMD_VERIFY_SHORT_ADDRESS:
            //! Short address verified. Identify device [f]
            instr_data.std_cmd.net.method = DALI2_L_NET_METHOD_SHORT_ADDRESSING;
            instr_data.std_cmd.net.addr_byte = __addr_list[__addr_count];
            dali2_hal_queue_push(DALI2_L_APP_CMD_IDENTIFY_DEVICE, &instr_data);
            break;

        case DALI2_L_APP_CMD_IDENTIFY_DEVICE:
            //! Identification forced. Withdraw device
            dali2_hal_queue_push(DALI2_L_APP_CMD_WITHDRAW, &instr_data);
            break;

        case DALI2_L_APP_CMD_WITHDRAW:
            //! Prepare next address
            __random_addr_prepare_next();

            //!  Search Next Low Address [c]
            instr_data.spec_cmd.searchaddress_hml = __addr_list[__addr_count];
            dali2_hal_queue_push(DALI2_L_APP_CMD_SEARCHADDRL, &instr_data);
            break;

        default:
            break;
    }
}

void dali2_hal_addr_alloc_dispatch(DALI2_L_APP_EVT_T evt, dali2_l_app_evt_data_t *evt_data)
{
    switch (__addr_alloc_method) {
        case DALI2_HAL_ADDR_ALLOC_METHOD_SINGLE:
            __single_addr_alloc_dispatch(evt, evt_data);
            break;

        case DALI2_HAL_ADDR_ALLOC_METHOD_RANDOM:
            __random_addr_alloc_dispatch(evt, evt_data);
            break;

        case DALI2_HAL_ADDR_ALLOC_METHOD_UNKNOWN:
        default:
            //! Free mutex
            dali2_hal_mtx_give();
            break;
    }
}

dali2_ret_t dali2_hal_addr_alloc(DALI2_HAL_ADDR_ALLOC_METHOD_T method, dali2_hal_addr_alloc_data_t *data)
{
    dali2_ret_t dali2_ret = DALI2_RET_SUCCESS;
    dali2_l_app_cmd_data_t instr_data;

    //! Capture mutex
    if (dali2_hal_mtx_check() == DALI2_HAL_EVT_ADDR_ALLOC ||
        dali2_hal_mtx_take(DALI2_HAL_EVT_ADDR_ALLOC) != DALI2_HAL_EVT_ADDR_ALLOC) {
        dali2_ret = DALI2_RET_BUSY;
        goto __ret;
    }

    __addr_alloc_method = method;

    switch (method) {
        case DALI2_HAL_ADDR_ALLOC_METHOD_SINGLE:
            //! Internal data initialization
            __addr_count = 0;
            __addr_list[__addr_count] = data->short_addr;

            //! Reset state first of all
            instr_data.std_cmd.net.method = DALI2_L_NET_METHOD_BROADCAST;
            instr_data.std_cmd.net.addr_byte = __addr_list[__addr_count];
            dali2_ret = dali2_hal_queue_push(DALI2_L_APP_CMD_RESET, &instr_data);
            break;

            //! TODO: Random address allocation still not tested!
        case DALI2_HAL_ADDR_ALLOC_METHOD_RANDOM:
            //! Internal data initialization
            __addr_count = 0;

            //! Put devices into INITIALIZE state [a]
            switch (data->random_step) {
                case DALI2_HAL_ADDR_ALLOC_RANDOM_STEP_FIRST:
                    __addr_list[__addr_count] = 0x00;
                    instr_data.spec_cmd.initialise.addressing = DALI2_APP_CMD_INITIALISE_ADDRESSING_ALL;
                    break;

                case DALI2_HAL_ADDR_ALLOC_RANDOM_STEP_AFTERFIRST:
                    __addr_list[__addr_count] = data->short_addr;
                    instr_data.spec_cmd.initialise.addressing = DALI2_APP_CMD_INITIALISE_ADDRESSING_SHORT_ADDRESS;
                    break;

                default:
                    //! Wrong Parameter of step!
                    dali2_ret = DALI2_RET_INVALID_PARAMS;
                    goto __ret;
            }
            instr_data.spec_cmd.initialise.addr = __addr_list[__addr_count];
            dali2_ret = dali2_hal_queue_push(DALI2_L_APP_CMD_INITIALISE, &instr_data);
            break;

        default:
            dali2_ret = DALI2_RET_INVALID_PARAMS;
            break;
    }

__ret:
    return dali2_ret;
}

unsigned int dali2_hal_addr_list_get(const unsigned char **list_ptr)
{
    *list_ptr = __addr_list;
    return __addr_count;
}

