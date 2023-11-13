/**
 * @copyright
 *
 * @file    dali2_l_pres.c
 * @author  Anton K.
 * @date    02 Sep 2021
 *
 * @brief   DALI-2 Application Controller.
 *          Presentation layer source file
 */

#include "dali2_l_pres.h"

dali2_ret_t dali2_l_pres_16bit_encode(unsigned long int *frame, unsigned char addr_encoded, unsigned char op_code)
{
    dali2_ret_t dali2_ret = DALI2_RET_SUCCESS;

    //! Verify frame buffer
    if (!frame) {
        dali2_ret = DALI2_RET_INVALID_PARAMS;
        goto __ret;
    }

    *frame = (addr_encoded << DALI2_L_PRES_ADDR_OFFSET) | (op_code << DALI2_L_PRES_OP_CODE_OFFSET);

__ret:
    return dali2_ret;
}
