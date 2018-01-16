//
// Created by Mike Mitterer on 15.01.18.
//

#include "stdafx.h"


#define HCI_H4_CMD_PREAMBLE_SIZE           (4)

/*  HCI Command opcode group field(OGF) */
#define HCI_GRP_HOST_CONT_BASEBAND_CMDS    (0x03 << 10)            /* 0x0C00 */
#define HCI_GRP_BLE_CMDS                   (0x08 << 10)

#define HCI_RESET                          (0x0003 | HCI_GRP_HOST_CONT_BASEBAND_CMDS)
#define HCI_BLE_WRITE_ADV_ENABLE           (0x000A | HCI_GRP_BLE_CMDS)
#define HCI_BLE_WRITE_ADV_PARAMS           (0x0006 | HCI_GRP_BLE_CMDS)
#define HCI_BLE_WRITE_ADV_DATA             (0x0008 | HCI_GRP_BLE_CMDS)

#define HCIC_PARAM_SIZE_WRITE_ADV_ENABLE        (1)
#define HCIC_PARAM_SIZE_BLE_WRITE_ADV_PARAMS    (15)
#define HCIC_PARAM_SIZE_BLE_WRITE_ADV_DATA      (31)



#define UINT16_TO_STREAM(p, u16) {*(p)++ = (uint8_t)(u16); *(p)++ = (uint8_t)((u16) >> 8);}
#define UINT8_TO_STREAM(p, u8)   {*(p)++ = (uint8_t)(u8);}
#define BDADDR_TO_STREAM(p, a)   {int ijk; for (ijk = 0; ijk < BD_ADDR_LEN;  ijk++) *(p)++ = (uint8_t) a[BD_ADDR_LEN - 1 - ijk];}
#define ARRAY_TO_STREAM(p, a, len) {int ijk; for (ijk = 0; ijk < len;        ijk++) *(p)++ = (uint8_t) a[ijk];}

const uint8_t FRAME_TYPE_UID = 0x00;
const uint8_t FRAME_TYPE_URL = 0x10;
const uint8_t FRAME_TYPE_TLM = 0x20;

static const int8_t TX_POWER = -18;

// In der main definiert
static uint8_t hci_cmd_buf[128];

enum {
    H4_TYPE_COMMAND = 1,
    H4_TYPE_ACL = 2,
    H4_TYPE_SCO = 3,
    H4_TYPE_EVENT = 4
};

static esp_vhci_host_callback_t vhci_host_cb = {
        controller_rcv_pkt_ready,
        host_rcv_pkt
};

int initBLE() {
    esp_err_t ret;

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(TAG, "%s initialize controller failed\n", __func__);
        return 1;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BTDM);
    if (ret) {
        ESP_LOGE(TAG, "%s enable controller failed\n", __func__);
        return 1;
    }

    return 0;
}

/*
 * @brief: send HCI commands to perform BLE advertising;
 */
void bleAdvtTask(void* pvParameters) {
    int cmd_cnt = 0;
    bool send_avail = false;

    esp_log_level_set("*", ESP_LOG_INFO);
    init_gpio(internalLED, GPIO_MODE_OUTPUT);

    esp_vhci_host_register_callback(&vhci_host_cb);
    Log.notice(F("BLE advt task start" CR));

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

    while (true) {
        vTaskDelay(2500 / portTICK_PERIOD_MS);
        gpio_set_level(internalLED, LED_OFF);
        vTaskDelay(2500 / portTICK_PERIOD_MS);
        gpio_set_level(internalLED, LED_ON);

        send_avail = esp_vhci_host_check_send_available();
        if (send_avail) {
            switch (cmd_cnt) {
                case 0:
                    hci_cmd_send_reset();
                    ++cmd_cnt;
                    break;
                case 1:
                    hci_cmd_send_ble_set_adv_param();
                    ++cmd_cnt;
                    break;
                case 2:
                    hci_cmd_send_ble_set_adv_url();
                    //hci_cmd_send_ble_set_adv_uuid();
                    ++cmd_cnt;
                    break;
                case 3:
                    hci_cmd_send_ble_adv_start();
                    ++cmd_cnt;
                    break;
            }
        }

        Log.trace(F("BLE Advertise, flag_send_avail: %d, cmd_sent: %d" CR),send_avail, cmd_cnt);
    }

#pragma clang diagnostic pop
}

// - private -----------------------------------------------------------------------------------------------------------


/*
 * @brief: BT controller callback function, used to notify the upper layer that
 *         controller is ready to receive command
 */
static void controller_rcv_pkt_ready() {
    printf("controller rcv pkt ready\n");
}

/*
 * @brief: BT controller callback function, to transfer data packet to upper
 *         controller is ready to receive command
 */
static int host_rcv_pkt(uint8_t* data, uint16_t len) {
    printf("host rcv pkt: ");
    for (uint16_t i = 0; i < len; i++)
        printf("%02x", data[i]);
    printf("\n");
    return 0;
}


static uint16_t make_cmd_reset(uint8_t* buf) {
    UINT8_TO_STREAM (buf, H4_TYPE_COMMAND);
    UINT16_TO_STREAM (buf, HCI_RESET);
    UINT8_TO_STREAM (buf, 0);
    return HCI_H4_CMD_PREAMBLE_SIZE;
}

static uint16_t make_cmd_ble_set_adv_enable(uint8_t* buf, uint8_t adv_enable) {
    UINT8_TO_STREAM (buf, H4_TYPE_COMMAND);
    UINT16_TO_STREAM (buf, HCI_BLE_WRITE_ADV_ENABLE);
    UINT8_TO_STREAM  (buf, HCIC_PARAM_SIZE_WRITE_ADV_ENABLE);
    UINT8_TO_STREAM (buf, adv_enable);
    return HCI_H4_CMD_PREAMBLE_SIZE + HCIC_PARAM_SIZE_WRITE_ADV_ENABLE;
}

static uint16_t make_cmd_ble_set_adv_param(uint8_t* buf, uint16_t adv_int_min, uint16_t adv_int_max,
                                           uint8_t adv_type, uint8_t addr_type_own,
                                           uint8_t addr_type_dir, bd_addr_t direct_bda,
                                           uint8_t channel_map, uint8_t adv_filter_policy) {
    UINT8_TO_STREAM (buf, H4_TYPE_COMMAND);
    UINT16_TO_STREAM (buf, HCI_BLE_WRITE_ADV_PARAMS);
    UINT8_TO_STREAM  (buf, HCIC_PARAM_SIZE_BLE_WRITE_ADV_PARAMS);

    UINT16_TO_STREAM (buf, adv_int_min);
    UINT16_TO_STREAM (buf, adv_int_max);
    UINT8_TO_STREAM (buf, adv_type);
    UINT8_TO_STREAM (buf, addr_type_own);
    UINT8_TO_STREAM (buf, addr_type_dir);
    BDADDR_TO_STREAM (buf, direct_bda);
    UINT8_TO_STREAM (buf, channel_map);
    UINT8_TO_STREAM (buf, adv_filter_policy);
    return HCI_H4_CMD_PREAMBLE_SIZE + HCIC_PARAM_SIZE_BLE_WRITE_ADV_PARAMS;
}


static uint16_t make_cmd_ble_set_adv_data(uint8_t* buf, uint8_t data_len, uint8_t* p_data) {
    UINT8_TO_STREAM (buf, H4_TYPE_COMMAND);
    UINT16_TO_STREAM (buf, HCI_BLE_WRITE_ADV_DATA);
    UINT8_TO_STREAM  (buf, HCIC_PARAM_SIZE_BLE_WRITE_ADV_DATA + 1);

    memset(buf, 0, HCIC_PARAM_SIZE_BLE_WRITE_ADV_DATA);

    if (p_data != NULL && data_len > 0) {
        if (data_len > HCIC_PARAM_SIZE_BLE_WRITE_ADV_DATA) {
            data_len = HCIC_PARAM_SIZE_BLE_WRITE_ADV_DATA;
        }

        UINT8_TO_STREAM (buf, data_len);

        ARRAY_TO_STREAM (buf, p_data, data_len);
    }
    return HCI_H4_CMD_PREAMBLE_SIZE + HCIC_PARAM_SIZE_BLE_WRITE_ADV_DATA + 1;
}

static void hci_cmd_send_reset() {
    uint16_t sz = make_cmd_reset(hci_cmd_buf);
    esp_vhci_host_send_packet(hci_cmd_buf, sz);
}

static void hci_cmd_send_ble_adv_start() {
    uint16_t sz = make_cmd_ble_set_adv_enable(hci_cmd_buf, 1);
    esp_vhci_host_send_packet(hci_cmd_buf, sz);
}

static void hci_cmd_send_ble_set_adv_param() {
    uint16_t adv_intv_min = 256; // 160ms
    uint16_t adv_intv_max = 256; // 160ms
    uint8_t adv_type = 0; // connectable undirected advertising (ADV_IND)
    uint8_t own_addr_type = 0; // Public Device Address
    uint8_t peer_addr_type = 0; // Public Device Address
    uint8_t peer_addr[6] = {0x80, 0x81, 0x82, 0x83, 0x84, 0x85};
    uint8_t adv_chn_map = 0x07; // 37, 38, 39
    uint8_t adv_filter_policy = 0; // Process All Conn and Scan

    uint16_t sz = make_cmd_ble_set_adv_param(hci_cmd_buf,
                                             adv_intv_min,
                                             adv_intv_max,
                                             adv_type,
                                             own_addr_type,
                                             peer_addr_type,
                                             peer_addr,
                                             adv_chn_map,
                                             adv_filter_policy);

    esp_vhci_host_send_packet(hci_cmd_buf, sz);
}

/// Eddystone-UID
///     https://github.com/google/eddystone/tree/master/eddystone-uid
///
/// Type-IDs:
///     https://www.bluetooth.com/specifications/assigned-numbers/generic-access-profile
///
static void hci_cmd_send_ble_set_adv_url() {
    // 31 ist die Länge dies ADV-packages
    uint8_t adv_data[31];
    uint8_t adv_data_len;

    adv_data[0] = 2;      // Len
    adv_data[1] = 0x01;   // Type Flags
    adv_data[2] = 0x06;   // GENERAL_DISC_MODE 0x02 | BR_EDR_NOT_SUPPORTED 0x04

    // Der für Eddystone reservierte UUID-Service lautet 0xFEAA
    //      https://www.bluetooth.com/specifications/assigned-numbers/16-bit-uuids-for-members
    adv_data[3] = 3;      // Len
    adv_data[4] = 0x03;   // Type 16-Bit UUID
    adv_data[5] = 0xAA;   // Eddystone UUID 2 -> 0xFEAA LSB
    adv_data[6] = 0xFE;   // Eddystone UUID 1 MSB

    adv_data[7] = 19;               // Length of Beacon Data
    adv_data[8] = 0x16;             // Type Service Data
    adv_data[9] = 0xAA;             // Eddystone UUID 2 -> 0xFEAA LSB
    adv_data[10] = 0xFE;            // Eddystone UUID 1 MSB
    adv_data[11] = FRAME_TYPE_URL;  // Eddystone-URL Frame Type (0x10)
    adv_data[12] = TX_POWER;        // Beacons TX power at 0m

    // URL-Scheme
    adv_data[13] = 0x03;            // URL Scheme 'https://'

    // URL
    adv_data[14] = 0x67;            // URL add  1 'g'
    adv_data[15] = 0x6F;            // URL add  2 'o'
    adv_data[16] = 0x6F;            // URL add  3 'o'
    adv_data[17] = 0x2E;            // URL add  4 '.'
    adv_data[18] = 0x67;            // URL add  5 'g'
    adv_data[19] = 0x6C;            // URL add  6 'l'
    adv_data[20] = 0x2F;            // URL add  7 '/'
    adv_data[21] = 0x32;            // URL add  8 '2'
    adv_data[22] = 0x79;            // URL add  9 'y'
    adv_data[23] = 0x43;            // URL add 10 'C'
    adv_data[24] = 0x36;            // URL add 11 '6'
    adv_data[25] = 0x4B;            // URL add 12 'K'
    adv_data[26] = 0x58;            // URL add 13 'X'

    adv_data_len = 27;

    printf("Eddystone adv_data [%d]=", adv_data_len);
    for (int i = 0; i < adv_data_len; i++) {
        printf("%02x", adv_data[i]);
    }
    printf("\n");


    uint16_t sz = make_cmd_ble_set_adv_data(hci_cmd_buf, adv_data_len, (uint8_t*) adv_data);
    esp_vhci_host_send_packet(hci_cmd_buf, sz);
}

static void hci_cmd_send_ble_set_adv_uuid() {
    // 31 ist die Länge dies ADV-packages
    uint8_t adv_data[31];
    uint8_t adv_data_len;

    adv_data[0] = 2;      // Len
    adv_data[1] = 0x01;   // Type Flags
    adv_data[2] = 0x06;   // GENERAL_DISC_MODE 0x02 | BR_EDR_NOT_SUPPORTED 0x04

    // Der für Eddystone reservierte UUID-Service lautet 0xFEAA
    //      https://www.bluetooth.com/specifications/assigned-numbers/16-bit-uuids-for-members
    adv_data[3] = 3;      // Len
    adv_data[4] = 0x03;   // Type 16-Bit UUID
    adv_data[5] = 0xAA;   // Eddystone UUID 2 -> 0xFEAA LSB
    adv_data[6] = 0xFE;   // Eddystone UUID 1 MSB

    adv_data[7] = 21;               // Length of Beacon Data
    adv_data[8] = 0x16;             // Type Service Data
    adv_data[9] = 0xAA;             // Eddystone UUID 2 -> 0xFEAA LSB
    adv_data[10] = 0xFE;            // Eddystone UUID 1 MSB
    adv_data[11] = FRAME_TYPE_UID;  // Eddystone-UID Frame Type (0x00)
    adv_data[12] = TX_POWER;        // Beacons TX power at 0m

    // UUID-Namespace (mikemitterer.at - 5606642e464150ec0265)
    adv_data[13] = 0x56;
    adv_data[14] = 0x06;
    adv_data[15] = 0x64;
    adv_data[16] = 0x2e;
    adv_data[17] = 0x46;
    adv_data[18] = 0x41;
    adv_data[19] = 0x50;
    adv_data[20] = 0xec;
    adv_data[21] = 0x02;
    adv_data[22] = 0x65;

    // UUID-Instance
    adv_data[23] = 0x00;
    adv_data[24] = 0x00;
    adv_data[25] = 0x00;
    adv_data[26] = 0x00;
    adv_data[27] = 0x00;
    adv_data[28] = 0x01;

    adv_data_len = 29;

    printf("Eddystone adv_data [%d]=", adv_data_len);
    for (int i = 0; i < adv_data_len; i++) {
        printf("%02x", adv_data[i]);
    }
    printf("\n");


    uint16_t sz = make_cmd_ble_set_adv_data(hci_cmd_buf, adv_data_len, (uint8_t*) adv_data);
    esp_vhci_host_send_packet(hci_cmd_buf, sz);
}



