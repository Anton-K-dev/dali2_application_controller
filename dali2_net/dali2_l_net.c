/**
 * @copyright
 *
 * @file    dali2_l_net.c
 * @author  Anton K.
 * @date    26 Aug 2021
 *
 * @brief   DALI-2 Application Controller.
 *          Network layer source file
 */

#include <stdint.h>
#include <stddef.h>

#include "dali2_l_net.h"

dali2_ret_t dali2_l_net_encode(DALI2_L_NET_SELECTOR_T selector, DALI2_L_NET_METHOD_T method,
                               dali2_l_net_addr_byte_t addr_byte, unsigned char *encoded_byte)
{
    dali2_ret_t dali2_ret = DALI2_RET_SUCCESS;

    //! Verify output byte pointer
    if (!encoded_byte) {
        dali2_ret = DALI2_RET_INVALID_PARAMS;
        goto __ret;
    }

    switch (method) {
        case DALI2_L_NET_METHOD_SHORT_ADDRESSING:
            //! Verify short address
            if (addr_byte >= DALI2_L_NET_ADDR_SHORT_MAX) {
                dali2_ret = DALI2_RET_INVALID_PARAMS;
                goto __ret;
            }
            //! Bit:  7 6 5 4 3 2 1 0
            //! Addr: 0 A A A A A A X
            *encoded_byte = addr_byte << 1;
            break;

        case DALI2_L_NET_METHOD_GROUP_ADDRESSING:
            //! Verify group address
            if (addr_byte >= DALI2_L_NET_ADDR_GROUP_MAX) {
                dali2_ret = DALI2_RET_INVALID_PARAMS;
                goto __ret;
            }
            //! Bit:  7 6 5 4 3 2 1 0
            //! Addr: 1 0 0 A A A A X
            *encoded_byte = (addr_byte << 1) | 0x80;
            break;

        case DALI2_L_NET_METHOD_BROADCAST_UNADDRESSED:
            //! Bit:  7 6 5 4 3 2 1 0
            //! Addr: 1 1 1 1 1 1 0 X
            *encoded_byte = 0xFC;
            break;

        case DALI2_L_NET_METHOD_BROADCAST:
            //! Bit:  7 6 5 4 3 2 1 0
            //! Addr: 1 1 1 1 1 1 1 X
            *encoded_byte = 0xFE;
            break;

        case DALI2_L_NET_METHOD_SPECIAL_COMMAND:
            *encoded_byte = addr_byte;
            break;

        case DALI2_L_NET_METHOD_RESERVED:
        default:
            dali2_ret = DALI2_RET_NOT_SUPPORTED;
            goto __ret;
    }

    //! Selector BIT setting
    if (selector != DALI2_L_NET_SELECTOR_DAPC) {
        *encoded_byte |= 0x01;
    }

__ret:
    return dali2_ret;
}
