/**
 * @copyright
 *
 * @file    dali2_error.h
 * @author  Anton K.
 * @date    19 Jul 2021
 *
 * @brief   DALI-2 Application Controller.
 *          Return codes header file
 */

#ifndef DALI2_ERROR_H_
#define DALI2_ERROR_H_

typedef enum {
    DALI2_RET_SUCCESS,
    DALI2_RET_INVALID_PARAMS,
    DALI2_RET_BUSY,
    DALI2_RET_TIMEOUT,
    DALI2_RET_NOT_SUPPORTED,
    DALI2_RET_INTERNAL_ERROR
} dali2_ret_t;


#endif /* DALI2_ERROR_H_ */
