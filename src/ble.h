//
// Created by Mike Mitterer on 15.01.18.
//

#ifndef NODEMCUESP32_BLE_H
#define NODEMCUESP32_BLE_H

#include "ble.h"
#include "gpio.h"

#include <stdio.h>
#include <string.h>


static const uint8_t BD_ADDR_LEN = 6;   // Device address length
typedef uint8_t bd_addr_t[BD_ADDR_LEN]; // Device address 

// - public ------------------------------------------------------------------------------------------------------------

int initBLE();
void bleAdvtTask(void* pvParameters);

// - private -----------------------------------------------------------------------------------------------------------

static void controller_rcv_pkt_ready();

static int host_rcv_pkt(uint8_t* data, uint16_t len);

static uint16_t make_cmd_reset(uint8_t* buf);

static uint16_t make_cmd_ble_set_adv_enable(uint8_t* buf, uint8_t adv_enable);

static uint16_t make_cmd_ble_set_adv_param(uint8_t* buf, uint16_t adv_int_min, uint16_t adv_int_max,
                                    uint8_t adv_type, uint8_t addr_type_own,
                                    uint8_t addr_type_dir, bd_addr_t direct_bda,
                                    uint8_t channel_map, uint8_t adv_filter_policy);

static uint16_t make_cmd_ble_set_adv_data(uint8_t* buf, uint8_t data_len, uint8_t* p_data);

static void hci_cmd_send_reset();

static void hci_cmd_send_ble_adv_start();

static void hci_cmd_send_ble_set_adv_param();

static void hci_cmd_send_ble_set_adv_url();

static void hci_cmd_send_ble_set_adv_uuid();

#endif //NODEMCUESP32_BLE_H
