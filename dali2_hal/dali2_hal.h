/**
 * @copyright
 *
 * @file    dali2_hal.h
 * @author  Anton K.
 * @date    15 Sep 2021
 *
 * @brief   DALI-2 Application Controller.
 *          HAL header file
 */
#ifndef DALI2_HAL_H_
#define DALI2_HAL_H_

#include "dali2_l_app.h"
#include "dali2_l_bsp.h"
#include "dali2_error.h"

#define DALI2_HAL_ADDR_LIST_SIZE    0xFF

#define DALI2_DIM_LEVEL_MAX               100
#define DALI2_DIM_LEVEL_LIM(level)        ((level > DALI2_DIM_LEVEL_MAX) ? DALI2_DIM_LEVEL_MAX : level)

#define DALI2_DIM_CFG_WRONG_LEVEL         0xFF
#define DALI2_DIM_CFG_MAX_LEVEL           0xFE
#define DALI2_DIM_CFG_MIN_LEVEL           0x9A

typedef enum {
    DALI2_HAL_EVT_ADDR_ALLOC,
    DALI2_HAL_EVT_DIM_CFG,
    DALI2_HAL_EVT_DIM_CTRL,

    DALI2_HAL_EVT_FREE
} DALI2_HAL_EVT_T;

/**@brief Dispatch this function inside dali2_l_app_evt_func_t()
 *
 * @param[IN] evt - @see dali2_l_app_evt_func_t()
 * @param[IN] evt_data - @see dali2_l_app_evt_func_t()
 */
void dali2_hal_app_evt_dispatch(DALI2_L_APP_EVT_T evt, dali2_l_app_evt_data_t *evt_data);

/**@brief HAL process
 * @note  Call this function periodically according your priority.
 *
 * @param[OUT] evt - event for return code
 * @return DALI2_RET_SUCCESS - HAL process successfully done
 *         DALI2_RET_BUSY - HAL process in progress or idle state
 *         [OTHERWISE] - execution error
 */
dali2_ret_t dali2_hal_process(DALI2_HAL_EVT_T *evt);

/********** Address allocation functions **********/
/*** See IEC 62386-102-2014 document @paragraph "Annex A" for addresses allocation ***/
typedef enum {
    DALI2_HAL_ADDR_ALLOC_METHOD_SINGLE,     //! Allocate single address
    DALI2_HAL_ADDR_ALLOC_METHOD_RANDOM,     //! Allocate random addresses
    DALI2_HAL_ADDR_ALLOC_METHOD_UNKNOWN
} DALI2_HAL_ADDR_ALLOC_METHOD_T;

typedef enum {
    DALI2_HAL_ADDR_ALLOC_RANDOM_STEP_FIRST,         //! This step is used for random address allocation
                                                    //! on the first step, all devices got random address
    DALI2_HAL_ADDR_ALLOC_RANDOM_STEP_AFTERFIRST     //! This step is used for address reallocation for
                                                    //! the same generated random address on the control gear
                                                    //! @note Fill @param single_addr for indicating short address
} DALI2_HAL_ADDR_ALLOC_RANDOM_STEP_T;

typedef union {
    unsigned char short_addr;       //! Used for DALI2_HAL_ADDR_ALLOC_METHOD_SINGLE and HAL_ADDR_ALLOC_METHOD_RANDOM methods
    DALI2_HAL_ADDR_ALLOC_RANDOM_STEP_T random_step;     //! Is used only for DALI2_HAL_ADDR_ALLOC_METHOD_RANDOM method
} dali2_hal_addr_alloc_data_t;

/**@brief Address allocation
 *
 * @param[IN] method - method for address allocation
 * @param[IN] data - method data if necessary
 *
 * @return DALI2_RET_SUCCESS - allocation started,
 *          otherwise error and need to use this function again
 */
dali2_ret_t dali2_hal_addr_alloc(DALI2_HAL_ADDR_ALLOC_METHOD_T method, dali2_hal_addr_alloc_data_t *data);

/**@brief Getting device address list
 *
 * @param[OUT] list_ptr - Pointer for getting device address list
 * @return Address count pointed by @param list_ptr
 */
unsigned int dali2_hal_addr_list_get(const unsigned char **list_ptr);

/********** Dimmer configuration related functions **********/
/*** See IEC 62386-102-2014 document for Dimmer configuration possibilities ***/
typedef enum {
    DALI2_HAL_DIM_MODE_NORMAL = DALI2_L_APP_OPERATING_MODE_NORMAL,
    DALI2_HAL_DIM_MODE_MANUFACTURE_SPECIFIC = DALI2_L_APP_OPERATING_MODE_MANUFACTURE_SPECIFIC
} DALI2_HAL_DIM_MODE_T;

typedef struct {
    DALI2_HAL_DIM_MODE_T mode;
    unsigned int fade_time_s;
    unsigned char level_min;
    unsigned char level_max;
    unsigned char curr_protect_en;
    DALI2_L_APP_DIMMING_CURVE_T dim_curve;
} dali2_hal_dim_cfg_t;

/**@brief Dimmer configuration function
 *
 * @param[IN] dim_cfg - Dimmer configuration structure
 * @param[IN] node - DALI node
 * @return DALI2_RET_SUCCESS - configuration started
 *          otherwise error and need to use this function until success
 */
dali2_ret_t dali2_hal_dim_cfg(dali2_hal_dim_cfg_t *dim_cfg, dali2_l_app_network_t *node);

typedef struct {
    unsigned char phy_min;              //! Related to DALI2_L_APP_CMD_QUERY_PHYSICAL_MINIMUM
    unsigned char light_src_type;       //! Related to DALI2_L_APP_CMD_QUERY_LIGHT_SOURCE_TYPE
    unsigned char device_type;          //! Relater to DALI2_L_APP_CMD_QUERY_DEVICE_TYPE
                                        //! Decode type as @ref DALI2_L_APP_CMD_DEVICE_T
    unsigned char led_features;         //! Related to DALI2_L_APP_CMD_QUERY_FEATURES
                                        //! Decode mask as @ref DALI2_L_APP_LED_FEATURE_T
    unsigned char led_operating_mode;   //! Related to DALI2_L_APP_CMD_QUERY_OPERATING_MODE_LED
                                        //! Decode mask as @ref DALI2_L_APP_LED_OPERATING_MODE_T
} dali2_hal_dim_meta_t;

/**@brief Getting Dimmer configuration metadata (internal dimmer constants)
 *
 * @param[OUT] dim_cfg - Dimmer configuration metadata structure
 * @param[IN] node - DALI node
 * @return DALI2_RET_SUCCESS - Metadata retrieve success,
 *          otherwise error and need to use this function until success
 */
dali2_ret_t dali2_hal_dim_meta_get(dali2_hal_dim_meta_t *dim_meta, dali2_l_app_network_t *node);

/********** Dimmer control related functions **********/
/*** See IEC 62386-102-2014 document for Dimmer control possibilities ***/

/**@brief Setting Dimmer level
 *
 * @param[IN] level - target level from physical minimum up to DALI2_DIM_LEVEL_MAX
 * @param[IN] node - DALI node
 * @return DALI2_RET_SUCCESS - if setting level started
 */
dali2_ret_t dali2_hal_dim_set_level(unsigned char level, dali2_l_app_network_t *node);

/**@brief Getting Dimmer level
 * @note This is Blocking function
 *
 * @param[OUT] level - pointer for dimmer level reception. Answer in
 *                       from Physical minimum up to DALI2_DIM_LEVEL_MAX
 * @param[IN] node - DALI node
 * @return Command execution return code, DALI2_RET_SUCCESS in success
 */
dali2_ret_t dali2_hal_dim_get_level(unsigned char *level, dali2_l_app_network_t *node);

/**@brief Obtaining Dimmer status
 *
 * @param[OUT] status - pointer for obtaining dimmer status.
 *  @note Decode status byte according @ref DALI2_L_APP_CMD_STATUS_T
 * @param[IN] node - DALI node
 * @return Command execution return code, DALI2_RET_SUCCESS in success
 */
dali2_ret_t dali2_hal_dim_get_status(unsigned char *status, dali2_l_app_network_t *node);

/**@brief Obtaining LED Failure status
 *
 * @param[OUT] status - pointer for obtaining LED failure status.
 *  @note Decode status byte according @ref DALI2_L_APP_LED_FAILURE_T
 * @param[IN] node - DALI node
 * @return Command execution return code, DALI2_RET_SUCCESS in success
 */
dali2_ret_t dali2_hal_dim_get_led_failure_status(unsigned char *failure_status, dali2_l_app_network_t *node);

#endif /* DALI2_HAL_H_ */
