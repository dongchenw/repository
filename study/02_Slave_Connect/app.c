/********************************************************************************************************
 * @file     feature_adv_power.c 
 *
 * @brief    for TLSR chips
 *
 * @author	 public@telink-semi.com;
 * @date     May. 10, 2018
 *
 * @par      Copyright (c) Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *           
 *			 The information contained herein is confidential and proprietary property of Telink 
 * 		     Semiconductor (Shanghai) Co., Ltd. and is available under the terms 
 *			 of Commercial License Agreement between Telink Semiconductor (Shanghai) 
 *			 Co., Ltd. and the licensee in separate contract or the terms described here-in. 
 *           This heading MUST NOT be removed from this file.
 *
 * 			 Licensees are granted free, non-transferable use of the information in this 
 *			 file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided. 
 *           
 *******************************************************************************************************/
#include <stack/ble/ble.h>
#include "tl_common.h"
#include "drivers.h"
#include "app_config.h"
#include "vendor/common/blt_led.h"
#include "application/keyboard/keyboard.h"
#include "vendor/common/tl_audio.h"
#include "vendor/common/blt_soft_timer.h"
#include "vendor/common/blt_common.h"

//#define		MY_RF_POWER_INDEX	RF_POWER_P10p29dBm //  10.29 dbm 
#define	MY_RF_POWER_INDEX	RF_POWER_P0p04dBm

#define RX_FIFO_SIZE	64
#define RX_FIFO_NUM		8

#define TX_FIFO_SIZE	40
#define TX_FIFO_NUM		16


_attribute_data_retention_  u8 		 	blt_rxfifo_b[RX_FIFO_SIZE * RX_FIFO_NUM] = {0};
_attribute_data_retention_	my_fifo_t	blt_rxfifo = {
												RX_FIFO_SIZE,
												RX_FIFO_NUM,
												0,
												0,
												blt_rxfifo_b,};

_attribute_data_retention_  u8 		 	blt_txfifo_b[TX_FIFO_SIZE * TX_FIFO_NUM] = {0};
_attribute_data_retention_	my_fifo_t	blt_txfifo = {
												TX_FIFO_SIZE,
												TX_FIFO_NUM,
												0,
												0,
												blt_txfifo_b,};

void connect_callback (u8 e, u8 *p, int n)
{
	bls_l2cap_requestConnParamUpdate (8, 8, 99, 400);   // 1 S
	at_print((unsigned char *)"\r\n+BLE_CONNECTED\r\n");
}

void disconnect_callback (u8 e, u8 *p, int n)
{
	at_print((unsigned char *)"\r\n+BLE_DISCONNECTED\r\n");
}

void conn_para_req_callback (u8 e, u8 *p, int n)
{
	at_print((unsigned char *)"\r\n+BLE_CONN_PARA_REQ\r\n");
}

void conn_para_up_callback (u8 e, u8 *p, int n)
{
	at_print((unsigned char *)"\r\n+BLE_CONN_PARA_UP\r\n");
}

void user_init_normal(void)
{
	random_generator_init();  //???????????????????????????

	u8  mac_public[6];
	u8  mac_random_static[6];
	blc_initMacAddress(CFG_ADR_MAC, mac_public, mac_random_static); //?????????MAC??????

	////// Controller Initialization  //////////
	blc_ll_initBasicMCU();   //?????????MCU
	blc_ll_initStandby_module(mac_public);		//?????????????????????????????????
	blc_ll_initAdvertising_module(mac_public);  //?????????????????????????????????
	blc_ll_initConnection_module();				//?????????????????????????????????
	blc_ll_initSlaveRole_module();				//?????????????????????????????????

	blc_smp_setSecurityLevel(No_Security); //????????????????????????

	u8 tbl_advData[] = {0x05, 0x09, 'A', 'B', 'C', 'D'}; //??????????????????

	bls_ll_setAdvData( (u8 *)tbl_advData, sizeof(tbl_advData) ); //??????????????????

	bls_ll_setScanRspData(NULL,0); //??????????????????????????????

	u8 status = bls_ll_setAdvParam( ADV_INTERVAL_50MS , //???????????????????????????
									ADV_INTERVAL_50MS , //???????????????????????????
									ADV_TYPE_CONNECTABLE_UNDIRECTED, //?????????????????????????????????
									OWN_ADDRESS_PUBLIC, //??????????????????
									0,  //??????????????????
									NULL, //????????????
									BLT_ENABLE_ADV_ALL, //?????????????????????(37,38,39)???????????????
									ADV_FP_NONE); //????????????

	if(status != BLE_SUCCESS)//??????????????????????????????
	{
		write_reg8(0x40000, 0x11);  //debug
		while(1);
	}

	rf_set_power_level_index (MY_RF_POWER_INDEX); //??????????????????

	bls_ll_setAdvEnable(1);  //????????????

	bls_app_registerEventCallback (BLT_EV_FLAG_CONNECT, &connect_callback);  //????????????????????????????????????
	bls_app_registerEventCallback (BLT_EV_FLAG_TERMINATE, &disconnect_callback); //????????????????????????????????????

	bls_app_registerEventCallback (BLT_EV_FLAG_CONN_PARA_REQ, &conn_para_req_callback);//??????????????????????????????????????????
	bls_app_registerEventCallback (BLT_EV_FLAG_CONN_PARA_UPDATE, &conn_para_up_callback);//??????????????????????????????????????????

	app_uart_init(); //??????????????????????????????????????????

	irq_enable();
}


_attribute_ram_code_ void main_loop (void)
{
	blt_sdk_main_loop();
}