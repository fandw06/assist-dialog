/**
 ****************************************************************************************
 *
 * @file user_custs1_impl.c
 *
 * @brief Custom1 Server implementation source code.
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
#include "adc.h"
#include "spi_adxl.h"

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

ke_msg_id_t timer_accel;
ke_msg_id_t timer_ecg;
bool running;
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

		if (val == CUSTS1_DATA_ENABLE) 
		{
				timer_accel = app_easy_timer(ACC_INTERVAL, app_adxl_val_timer_cb_handler);
				timer_ecg = app_easy_timer(ECG_INTERVAL, app_ecg_val_timer_cb_handler);
			  running = 1;
		}
		else 
		{
			  running = 0;
			/*
			  if (timer_accel != 0xFFFF)
        {
            app_easy_timer_cancel(timer_accel);
            timer_accel = 0xFFFF;
        }
				if (timer_ecg != 0xFFFF)
        {
            app_easy_timer_cancel(timer_ecg);
            timer_ecg = 0xFFFF;
        }
			*/
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

void user_custs1_ecg_val_cfg_ind_handler(ke_msg_id_t const msgid,
                                            struct custs1_val_write_ind const *param,
                                            ke_task_id_t const dest_id,
                                            ke_task_id_t const src_id)
{
}

void user_custs1_ecg_val_ntf_cfm_handler(ke_msg_id_t const msgid,
                                            struct custs1_val_write_ind const *param,
                                            ke_task_id_t const dest_id,
                                            ke_task_id_t const src_id)
{
}

void app_adxl_val_timer_cb_handler()
{
    struct custs1_val_ntf_req* req = KE_MSG_ALLOC_DYN(CUSTS1_VAL_NTF_REQ,
                                                      TASK_CUSTS1,
                                                      TASK_APP,
                                                      custs1_val_ntf_req,
                                                      DEF_CUST1_ADXL_VAL_CHAR_LEN);
	
    uint8_t accel[] = {read_accel(ZDATA), read_accel(YDATA), read_accel(XDATA)};
		int ecg = adc_get_sample();
		
    req->conhdl = app_env->conhdl;
    req->handle = CUST1_IDX_ADXL_VAL_VAL;
    req->length = DEF_CUST1_ADXL_VAL_CHAR_LEN;
    memcpy(req->value, &accel, DEF_CUST1_ADXL_VAL_CHAR_LEN);

    ke_msg_send(req);

    if (ke_state_get(TASK_APP) == APP_CONNECTED && running)
    {
        // Set it once again until Stop command is received in Control Characteristic
        timer_accel = app_easy_timer(ACC_INTERVAL, app_adxl_val_timer_cb_handler);
    }
}

void app_ecg_val_timer_cb_handler()
{
    struct custs1_val_ntf_req* req = KE_MSG_ALLOC_DYN(CUSTS1_VAL_NTF_REQ,
                                                      TASK_CUSTS1,
                                                      TASK_APP,
                                                      custs1_val_ntf_req,
                                                      DEF_CUST1_ECG_VAL_CHAR_LEN);
	  adc_init(GP_ADC_SE, 0, 0);
	  adc_enable_channel(ADC_CHANNEL_P02);
		int ecg = adc_get_sample();
		uint8_t low = ecg & 0xff;
		uint8_t high = (ecg >> 8) & 0x03;
		uint8_t ecg_val[2];
		ecg_val[0] = high;
		ecg_val[1] = low;
    req->conhdl = app_env->conhdl;
    req->handle = CUST1_IDX_ECG_VAL_VAL;
    req->length = DEF_CUST1_ECG_VAL_CHAR_LEN;
    memcpy(req->value, &ecg_val, DEF_CUST1_ECG_VAL_CHAR_LEN);

    ke_msg_send(req);

    if (ke_state_get(TASK_APP) == APP_CONNECTED && running)
    {
        // Set it once again until Stop command is received in Control Characteristic
        timer_ecg = app_easy_timer(ECG_INTERVAL, app_ecg_val_timer_cb_handler);
    }
}
