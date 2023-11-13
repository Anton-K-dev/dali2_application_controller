/**
 * @copyright
 *
 * @file    dali2_led_cmd_list.h
 * @author  Anton K.
 * @date    28 Oct 2021
 *
 * @brief   DALI-2 Application Controller.
 *          LED command list header file
 *          @ref DALI IEC 62386-207-2009
 */
#ifndef DALI2_LED_CMD_LIST_H_
#define DALI2_LED_CMD_LIST_H_

//! LED command list
typedef enum {
    DALI2_L_APP_LED_CMD_REFERENCE_SYSTEM_POWER = 224,
    DALI2_L_APP_LED_CMD_ENABLE_CURRENT_PROTECTOR,
    DALI2_L_APP_LED_CMD_DISABLE_CURRENT_PROTECTOR,
    DALI2_L_APP_LED_CMD_SELECT_DIMMING_CURVE,
    DALI2_L_APP_LED_CMD_STORE_DTR_AS_FAST_FADE_TIME,

    DALI2_L_APP_LED_CMD_QUERY_GEAR_TYPE = 237,
    DALI2_L_APP_LED_CMD_QUERY_DIMMING_CURVE,
    DALI2_L_APP_LED_CMD_QUERY_POSSIBLE_OPERATING_MODES,
    DALI2_L_APP_LED_CMD_QUERY_FEATURES,
    DALI2_L_APP_LED_CMD_QUERY_FAILURE_STATUS,
    DALI2_L_APP_LED_CMD_QUERY_SHORT_CIRCUIT,
    DALI2_L_APP_LED_CMD_QUERY_OPEN_CIRCUIT,
    DALI2_L_APP_LED_CMD_QUERY_LOAD_DECREASE,
    DALI2_L_APP_LED_CMD_QUERY_LOAD_INCREASE,
    DALI2_L_APP_LED_CMD_QUERY_CURRENT_PROTECTOR_ACTIVE,
    DALI2_L_APP_LED_CMD_QUERY_THERMAL_SHUT_DOWN,
    DALI2_L_APP_LED_CMD_QUERY_THERMAL_OVERLOAD,
    DALI2_L_APP_LED_CMD_QUERY_REFERENCE_RUNNING,
    DALI2_L_APP_LED_CMD_QUERY_REFERENCE_MEASUREMENT_FAILED,
    DALI2_L_APP_LED_CMD_QUERY_CURRENT_PROTECTOR_ENABLED,
    DALI2_L_APP_LED_CMD_QUERY_OPERATING_MODE,
    DALI2_L_APP_LED_CMD_QUERY_FAST_FADE_TIME,
    DALI2_L_APP_LED_CMD_QUERY_MIN_FAST_FADE_TIME,
    DALI2_L_APP_LED_CMD_QUERY_EXTENDED_VERSION_NUMBER,
    DALI2_L_APP_LED_CMD_ENABLE_DEVICE_TYPE_6 = 0xC106
} DALI2_L_APP_LED_CMD_T;

#endif /* DALI2_LED_CMD_LIST_H_ */
