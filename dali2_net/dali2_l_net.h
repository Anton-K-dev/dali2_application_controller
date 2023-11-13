/**
 * @copyright
 *
 * @file    dali2_l_net.h
 * @author  Anton K.
 * @date    26 Aug 2021
 *
 * @brief   DALI-2 Application Controller.
 *          Network layer header file
 */

#ifndef DALI2_L_NET_H_
#define DALI2_L_NET_H_

#include "dali2_l_bsp.h"
#include "dali2_error.h"

#define DALI2_L_NET_ADDR_RESERVED               0xCC
#define DALI2_L_NET_ADDR_SHORT_MAX              0x40
#define DALI2_L_NET_ADDR_GROUP_MAX              0x08

typedef enum {
    DALI2_L_NET_SELECTOR_DAPC,
    DALI2_L_NET_SELECTOR_OTHER
} DALI2_L_NET_SELECTOR_T;

typedef enum {
    DALI2_L_NET_METHOD_SHORT_ADDRESSING,
    DALI2_L_NET_METHOD_GROUP_ADDRESSING,
    DALI2_L_NET_METHOD_BROADCAST_UNADDRESSED,
    DALI2_L_NET_METHOD_BROADCAST,
    DALI2_L_NET_METHOD_SPECIAL_COMMAND,
    DALI2_L_NET_METHOD_RESERVED
} DALI2_L_NET_METHOD_T;

typedef unsigned char dali2_l_net_addr_byte_t;

dali2_ret_t dali2_l_net_encode(DALI2_L_NET_SELECTOR_T selector, DALI2_L_NET_METHOD_T method,
                               dali2_l_net_addr_byte_t addr_byte, unsigned char *encoded_byte);


#endif /* DALI2_L_NET_H_ */
