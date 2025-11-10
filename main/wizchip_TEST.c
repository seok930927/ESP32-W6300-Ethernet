/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_log.h"
#include "W5500/w5500.h"
#include "W6300/w6300.h"
#include "wizchip_conf.h"
#include "wizchip_spi.h"
#include "esp_task_wdt.h"
#include "loopback.h"
#include "iperf.h"
#include "cJSON.h"
#include "socket.h"


/* Buffer */

/* Socket */
#define SOCKET_DATA 0
#define SOCKET_CTRL 1

/* Port */


/* Socket */
#define SOCKET_IPERF 0

/* Port */
#define PORT_IPERF 5001

#define MAX_RESULT_LEN 1024

/* Cookie size */
#define COOKIE_SIZE 37


/* iperf3 Commands */
#define PARAM_EXCHANGE 9
#define CREATE_STREAMS 10
#define TEST_START 1
#define TEST_RUNNING 2
#define TEST_END 4
#define EXCHANGE_RESULTS 13
#define DISPLAY_RESULTS 14
#define IPERF_DONE 16


/* iperf */
static uint8_t g_iperf_buf[1024 * 16] = {
    0,
};
int32_t recv_iperf(uint8_t sn, uint8_t * buf, uint16_t len) ;
static uint8_t cookie[COOKIE_SIZE] = {0};

uint8_t dest_ip[4];
uint16_t destport;

/* Clock */
void handle_param_exchange(bool *reverse, bool *udp);
void handle_create_streams(bool udp, uint8_t *dest_ip, uint16_t destport);
void start_iperf_test(Stats *stats, bool reverse, bool udp, uint8_t *dest_ip, uint16_t destport);
void exchange_results(Stats *stats);


static wiz_NetInfo g_net_info = {
    .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56}, // MAC address
    .ip = {192, 168, 11, 2},                     // IP address
    .sn = {255, 255, 255, 0},                    // Subnet Mask
    .gw = {192, 168, 11, 1},                     // Gateway
    .dns = {8, 8, 8, 8},                         // DNS server
#if _WIZCHIP_ > W5500
    .lla = {
        0xfe, 0x80, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x02, 0x08, 0xdc, 0xff,
        0xfe, 0x57, 0x57, 0x25
    },             // Link Local Address
    .gua = {
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    },             // Global Unicast Address
    .sn6 = {
        0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    },             // IPv6 Prefix
    .gw6 = {
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    },             // Gateway IPv6 Address
    .dns6 = {
        0x20, 0x01, 0x48, 0x60,
        0x48, 0x60, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x88, 0x88
    },             // DNS6 server
    .ipmode = NETINFO_STATIC_ALL
#else
    .dhcp = NETINFO_STATIC
#endif
};

#define ETHERNET_BUF_MAX_SIZE (1024 * 16)
static uint8_t g_tcp_server_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
};


// static const char * TAG= "WIZCHIP_TEST"; 


static uint32_t get_time_us_ptr(void){
    return (uint32_t)(esp_timer_get_time() & 0xFFFFFFFF);
}

void msleep(int ms)
{
vTaskDelay(ms / portTICK_PERIOD_MS);
}


void app_main(void)
{
    // vTaskSuspendAll();
    /* Initialize */
    uint8_t socket_status;
    Stats stats;

    bool reverse = false;
    bool udp = false;


    esp_task_wdt_deinit(); // 전역 watchdog 해제

    // Setup iperf library timer callbacks
    iperf_init(get_time_us_ptr);
    ESP_LOGI(TAG, "Iperf library timer callbacks initialized");

    esp_err_t ret;
    spi_transaction_t t;
    spi_transaction_t r;


    ESP_LOGI(TAG, "Starting SPI example");
    wizchip_reset();
    //spi_init_qspi();
    spi_init(); 

    uint8_t spi_data[4] = {0,0x39 ,0x00,0x00};
    uint8_t recv_Data[4] = {0,0 ,0x00,0x00};
  
    msleep(1000);
//   while(1 ) {
//     spi_receive_data(spi_data, recv_Data,  3, 1);
//     printf ("recv_Data: 0x%02X\n", recv_Data[0]);
//     msleep(100);
//     }
    wizchip_initialize();

    network_initialize(g_net_info);
    print_network_information(g_net_info);

   int retval = 0;


    uint32_t pack_len = 0;
    
    while (1) {
        switch (getSn_SR(SOCKET_IPERF)) {
        case SOCK_ESTABLISHED :
            while (1) {
                getsockopt(SOCKET_IPERF, SO_RECVBUF, &pack_len);
                if (pack_len > 0) {
                    recv_iperf(SOCKET_IPERF, (uint8_t *)g_iperf_buf, ETHERNET_BUF_MAX_SIZE - 1);
                }
            }
        case SOCK_CLOSE_WAIT :
            disconnect(SOCKET_IPERF);
            break;
        case SOCK_INIT :
            listen(SOCKET_IPERF);
            break;
        case SOCK_CLOSED:
            socket(SOCKET_IPERF, Sn_MR_TCP, PORT_IPERF, 0x20);
            break;
        default:
            break;
        }
    }
}



void handle_param_exchange(bool *reverse, bool *udp) {
    char buffer[512] = {0};
    uint8_t cmd;
    uint16_t len = 0;
    uint8_t raw_len[4] = {0};
    int cookie_len;
    cJSON *json;
    cJSON *reverseItem;
    cJSON *udpItem;

    cookie_len = recv(SOCKET_CTRL, cookie, COOKIE_SIZE);
    if (cookie_len != COOKIE_SIZE) {
        printf("[iperf] Failed to receive cookie. Received: %d bytes\n", cookie_len);
        return;
    }

#ifdef IPERF_DEBUG
    printf("[iperf] Received cookie: %s\n", cookie);
#endif

    cmd = PARAM_EXCHANGE;
    send(SOCKET_CTRL, &cmd, 1);
    recv(SOCKET_CTRL, raw_len, 4);

    len = (raw_len[0] << 24) | (raw_len[1] << 16) | (raw_len[2] << 8) | raw_len[3];
#ifdef IPERF_DEBUG
    printf("[iperf] Raw length bytes: 0x%02X 0x%02X 0x%02X 0x%02X, Parsed length: %d\n", raw_len[0], raw_len[1], raw_len[2], raw_len[3], len);
#endif

    recv(SOCKET_CTRL, (uint8_t *)buffer, len);
    buffer[len] = '\0'; // Null-terminate

#ifdef IPERF_DEBUG
    printf("[iperf] Received parameters: %s\n", buffer);
#endif

    json = cJSON_Parse(buffer);
    if (json == NULL) {
        printf("[iperf] Failed to parse JSON: %s\n", cJSON_GetErrorPtr());
    } else {
        reverseItem = cJSON_GetObjectItem(json, "reverse");
        udpItem = cJSON_GetObjectItem(json, "udp");

        *reverse = (reverseItem && cJSON_IsBool(reverseItem)) ? reverseItem->valueint : 0;
        *udp = (udpItem && cJSON_IsBool(udpItem)) ? udpItem->valueint : 0;

#ifdef IPERF_DEBUG
        printf("[iperf] Parsed JSON: %s\n", cJSON_Print(json));
        printf("[iperf] Parsed JSON: reverse=%d, udp=%d\n", *reverse, *udp);
#endif
        cJSON_Delete(json);
    }
}

int32_t recv_iperf(uint8_t sn, uint8_t * buf, uint16_t len) {
    wiz_recv_data(sn, buf, len);
    setSn_CR(sn, Sn_CR_RECV);
    while (getSn_CR(sn));

    return (int32_t)len;
}