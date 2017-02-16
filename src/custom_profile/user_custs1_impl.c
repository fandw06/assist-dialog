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

struct user_data_buffer accel_buff = {
    .SIZE = 20,
    .data = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	  .pos = 0,
};

struct user_data_buffer ecg_buff = {
    .SIZE = 20,
    .data = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	  .pos = 0,
};

struct user_data_buffer vol_buff = {
    .SIZE = 20,
    .data = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	  .pos = 0,
};

ke_msg_id_t timer_accel;
ke_msg_id_t timer_ecg;
ke_msg_id_t timer_vol;
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
			  timer_vol = app_easy_timer(VOL_INTERVAL, app_vol_val_timer_cb_handler);
			  running = 1;
		}
		else 
		{
			  running = 0;
		}
}

void app_adxl_val_timer_cb_handler()
{
		// Current acceleration values.
		uint8_t accel[] = {read_accel(XDATA), read_accel(YDATA), read_accel(ZDATA)};
		accel_buff.data[accel_buff.pos++] = accel[0];
		accel_buff.data[accel_buff.pos++] = accel[1];
		accel_buff.data[accel_buff.pos++] = accel[2];
		
		// If the data buffer is full, then send a message to kernel to notify.
		if (accel_buff.pos + 3 > accel_buff.SIZE) 
		{
			  accel_buff.pos = 0;
				struct custs1_val_ntf_req* req = KE_MSG_ALLOC_DYN(CUSTS1_VAL_NTF_REQ,
																													TASK_CUSTS1,
																													TASK_APP,
																													custs1_val_ntf_req,
																													DEF_CUST1_ADXL_VAL_CHAR_LEN);
				req->conhdl = app_env->conhdl;
				req->handle = CUST1_IDX_ADXL_VAL_VAL;
				req->length = DEF_CUST1_ADXL_VAL_CHAR_LEN;
				memcpy(req->value, &accel_buff.data, DEF_CUST1_ADXL_VAL_CHAR_LEN);
				ke_msg_send(req);
	  }
		
		if (ke_state_get(TASK_APP) == APP_CONNECTED && running)
		{
				if (running)
						timer_accel = app_easy_timer(ACC_INTERVAL, app_adxl_val_timer_cb_handler);
		}
}

void app_ecg_val_timer_cb_handler()
{		
		// Initialize adc, channel 02
	  adc_init(GP_ADC_SE, 0, 0);
	  adc_enable_channel(ADC_CHANNEL_P02);
		int data = adc_get_sample();
		uint8_t adc[2];
		adc[0] = (data >> 8) & 0x03;
		adc[1] = data & 0xff;

		ecg_buff.data[ecg_buff.pos++] = adc[0];
		ecg_buff.data[ecg_buff.pos++] = adc[1];
		
		// If the data buffer is full, then send a message to kernel to notify.
		if (ecg_buff.pos + 2 > ecg_buff.SIZE) 
		{
			  ecg_buff.pos = 0;
				struct custs1_val_ntf_req* req = KE_MSG_ALLOC_DYN(CUSTS1_VAL_NTF_REQ,
																													TASK_CUSTS1,
																													TASK_APP,
																													custs1_val_ntf_req,
																													DEF_CUST1_ECG_VAL_CHAR_LEN);
				req->conhdl = app_env->conhdl;
				req->handle = CUST1_IDX_ECG_VAL_VAL;
				req->length = DEF_CUST1_ECG_VAL_CHAR_LEN;
				memcpy(req->value, &ecg_buff.data, DEF_CUST1_ECG_VAL_CHAR_LEN);
				ke_msg_send(req);
	  }
		
		if (ke_state_get(TASK_APP) == APP_CONNECTED && running)
		{
				if (running)
						timer_ecg = app_easy_timer(ECG_INTERVAL, app_ecg_val_timer_cb_handler);
		}
}

void app_vol_val_timer_cb_handler()
{		
		// Initialize adc, channel 01
	  adc_init(GP_ADC_SE, 0, 0);
	  adc_enable_channel(ADC_CHANNEL_P01);
		int data = adc_get_sample();
		uint8_t adc[2];
		adc[0] = (data >> 8) & 0x03;
		adc[1] = data & 0xff;

		vol_buff.data[vol_buff.pos++] = adc[0];
		vol_buff.data[vol_buff.pos++] = adc[1];
		
		// If the data buffer is full, then send a message to kernel to notify.
		if (vol_buff.pos + 2 > vol_buff.SIZE) 
		{
			  vol_buff.pos = 0;
				struct custs1_val_ntf_req* req = KE_MSG_ALLOC_DYN(CUSTS1_VAL_NTF_REQ,
																													TASK_CUSTS1,
																													TASK_APP,
																													custs1_val_ntf_req,
																													DEF_CUST1_VOL_VAL_CHAR_LEN);
				req->conhdl = app_env->conhdl;
				req->handle = CUST1_IDX_VOL_VAL_VAL;
				req->length = DEF_CUST1_VOL_VAL_CHAR_LEN;
				memcpy(req->value, &vol_buff.data, DEF_CUST1_VOL_VAL_CHAR_LEN);
				ke_msg_send(req);
	  }
		
		if (ke_state_get(TASK_APP) == APP_CONNECTED && running)
		{
				if (running)
						timer_vol = app_easy_timer(VOL_INTERVAL, app_vol_val_timer_cb_handler);
		}
}

