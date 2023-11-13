/**
 * @copyright
 *
 * @file    dali2_l_ses.h
 * @author  Anton K.
 * @date    26 Aug 2021
 *
 * @brief   DALI-2 Application Controller.
 *          Session layer header file
 */

#ifndef DALI2_L_SES_H_
#define DALI2_L_SES_H_

#include "dali2_l_phy.h"
#include "dali2_l_bsp.h"
#include "dali2_error.h"


//! Settling time between forward and backward frame
#define DALI2_L_SES_SETTLING_TIME_FW_BW_OVERHEAD_MS 10      //! This is overhead extention for practice case,
                                                            //! "DALI IEC 62386-101-2014" doesn't allows this
#define DALI2_L_SES_SETTLING_TIME_FW_BW_MS_MIN  2
#define DALI2_L_SES_SETTLING_TIME_FW_BW_MS_MAX  (13 + DALI2_L_SES_SETTLING_TIME_FW_BW_OVERHEAD_MS)

//! Settling time between forward and forward frame
#define DALI2_L_SES_SETTLING_TIME_FW_FW_MS_MIN          2
#define DALI2_L_SES_SETTLING_TIME_FW_FW_MS_MAX          3
#define DALI2_L_SES_SETTLING_TIME_FW_FW_TWICE_MS_MAX    94

typedef enum {
    DALI2_L_SES_SEND,
    DALI2_L_SES_SEND_TWICE,
    DALI2_L_SES_QUERY
} DALI2_L_SES_MSG_T;

typedef enum {
    DALI2_L_SES_EVT_DONE,
    DALI2_L_SES_EVT_COLLISION,
    DALI2_L_SES_EVT_TIMEOUT,
    DALI2_L_SES_EVT_UNEXPECTED_FRAME
} DALI2_L_SES_EVT_T;

typedef struct {
    DALI2_L_SES_MSG_T msg;
    unsigned int msg_data;
} dali2_l_ses_evt_param_t;

typedef void (* dali2_l_ses_evt_func_t) (DALI2_L_SES_EVT_T evt, dali2_l_ses_evt_param_t *params);

//! @brief One-short Timer callback Handler
//! @note Produced by dali2_l_bsp_ses_timer_start_ms(()
//! @attention Must have to be used!
void dali2_l_ses_timer_cb_handler(void);

/**@brief DALI2 Session layer initialization
 *
 * @param[IN] evt_handler - event handler for DALI2 Session layer
 * @return @see dali2_ret_t
 */
dali2_ret_t dali2_l_ses_init(dali2_l_ses_evt_func_t evt_handler);

/**@brief DALI2 Session layer deinitialization
 *
 * @return @see dali2_ret_t
 */
dali2_ret_t dali2_l_ses_deinit(void);

/**@brief DALI2 Session execution
 *
 * @param[IN] msg - session message type, @see DALI2_L_SES_MSG_T
 * @param[IN] frame_type - Physical layer data frame type, @see DALI2_L_PHY_FRAME_T
 * @param[IN] frame_data - Physical layer data frame
 * @return @see dali2_ret_t
 */
dali2_ret_t dali2_l_ses_exec(DALI2_L_SES_MSG_T msg, DALI2_L_PHY_FRAME_T frame_type, unsigned int frame_data);

#endif /* DALI2_L_SES_H_ */
