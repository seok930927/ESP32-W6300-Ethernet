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

#define ETHERNET_BUF_MAX_SIZE (1024 * 4)
static uint8_t g_tcp_server_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
};


// static const char * TAG= "WIZCHIP_TEST"; 





void msleep(int ms)
{
vTaskDelay(ms / portTICK_PERIOD_MS);
}


void app_main(void)
{


    esp_task_wdt_deinit(); // 전역 watchdog 해제

    esp_err_t ret;
    spi_transaction_t t;
    spi_transaction_t r;

    msleep(1000);
    msleep(1000);
    msleep(1000);
    msleep(1000);
    msleep(1000);

    ESP_LOGI(TAG, "Starting SPI example");
    wizchip_reset();
    spi_init_qspi();
    printf("CHip_id = %d \n", getCIDR());

    wizchip_initialize();

    network_initialize(g_net_info);
    print_network_information(g_net_info);
   int retval = 0;

while(1){
  
        if ((retval = loopback_tcps(0, g_tcp_server_buf, 5000)) < 0) {
            printf(" loopback_tcps error : %d\n", retval);

            while (1)
                ;
        }
}

}