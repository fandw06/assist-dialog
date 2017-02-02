/**
 ****************************************************************************************
 *
 * @file user_custs1_impl.c
 *
 * @brief Peripheral project Custom1 Server implementation source code.
 *
 * Copyright (C) 2015. Dialog Semiconductor Ltd, unpublished work. This computer
 * program includes Confidential, Proprietary Information and is a Trade Secret of
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited
 * unless authorized in writing. All Rights Reserved.
 *
 * <bluetooth.support@diasemi.com> and contributors.
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gpio.h"
#include "app_api.h"
#include "app.h"
#include "user_custs1_def.h"
#include "user_custs1_impl.h"
#include "user_main.h"
#include "user_periph_setup.h"

#include "spi_adxl.h"

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

ke_msg_id_t timer_used;

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void user_custs1_ctrl_wr_ind_handler(ke_msg_id_t const msgid,
                                      struct custs1_val_write_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    uint8_t val = 0;
    memcpy(&val, &param->value[0], param->length);

    if (val != CUSTS1_CP_ADC_VAL1_DISABLE)
    {
        timer_used = app_easy_timer(APP_PERIPHERAL_CTRL_TIMER_DELAY, app_adxlvals_timer_cb_handler);
    }
    else
    {
        if (timer_used != 0xFFFF)
        {
            app_easy_timer_cancel(timer_used);
            timer_used = 0xFFFF;
        }
    }
}

void user_custs1_adxl_val_cfg_ind_handler(ke_msg_id_t const msgid,
                                            struct custs1_val_write_ind const *param,
                                            ke_task_id_t const dest_id,
                                            ke_task_id_t const src_id)
{
}

void user_custs1_adxl_val_ntf_cfm_handler(ke_msg_id_t const msgid,
                                            struct custs1_val_write_ind const *param,
                                            ke_task_id_t const dest_id,
                                            ke_task_id_t const src_id)
{
}

void app_adxlvals_timer_cb_handler()
{
    struct custs1_val_ntf_req* req = KE_MSG_ALLOC_DYN(CUSTS1_VAL_NTF_REQ,
                                                      TASK_CUSTS1,
                                                      TASK_APP,
                                                      custs1_val_ntf_req,
                                                      DEF_CUST1_ADXL_VAL_CHAR_LEN);
	
    uint8_t accel[] = {read_accel(XDATA), read_accel(YDATA), read_accel(ZDATA)};
		
    req->conhdl = app_env->conhdl;
    req->handle = CUST1_IDX_ADXL_VAL_VAL;
    req->length = DEF_CUST1_ADXL_VAL_CHAR_LEN;
    memcpy(req->value, &accel, DEF_CUST1_ADXL_VAL_CHAR_LEN);

    ke_msg_send(req);

    if (ke_state_get(TASK_APP) == APP_CONNECTED)
    {
        // Set it once again until Stop command is received in Control Characteristic
        timer_used = app_easy_timer(APP_PERIPHERAL_CTRL_TIMER_DELAY, app_adxlvals_timer_cb_handler);
    }
}
