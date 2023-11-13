/**
 * @copyright
 *
 * @file    dali2_l_pres.h
 * @author  Anton K.
 * @date    02 Sep 2021
 *
 * @brief   DALI-2 Application Controller.
 *          Presentation layer header file
 */
#ifndef DALI2_L_PRES_H_
#define DALI2_L_PRES_H_

#include "dali2_error.h"

#define DALI2_L_PRES_ADDR_OFFSET        0x08
#define DALI2_L_PRES_OP_CODE_OFFSET     0x00

/**@brief DALI2 Presentation layer 16bit frame encoding
 *
 * @param[OUT] frame - output frame buffer
 * @param[IN] addr_encoded - encoded device address
 * @param[IN] op_code - operation code for execution
 * @return @see dali2_ret_t
 */
dali2_ret_t dali2_l_pres_16bit_encode(unsigned long int *frame, unsigned char addr_encoded, unsigned char op_code);


#endif /* DALI2_L_PRES_H_ */
