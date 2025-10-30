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
#include "wizchip_conf.h"
#include "wizchip_spi.h"

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

#define ETHERNET_BUF_MAX_SIZE (1024 * 2)
static uint8_t g_tcp_server_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
};


// static const char *TAG = "WIZCHIP_TEST"; 

spi_device_handle_t spi_dev2;



// SPI 초기화 함수
void spi_init2(void)
{
    esp_err_t ret;
    
    // SPI 버스 설정
    spi_bus_config_t buscfg = {
        .sclk_io_num = SPI_CLK_PIN,      // CLK
        
        .data0_io_num = SPI_D0_PIN,      // DATA0 (MOSI와 동일)
        .data1_io_num = SPI_D1_PIN,      // DATA1 (MISO와 동일)
        .data2_io_num = SPI_D2_PIN,      // DATA2 (WP)
        .data3_io_num = SPI_D3_PIN,      // DATA3 (HD)
        .data4_io_num = -1,              // OCTAL 모드용 (사용 안함)
        .data5_io_num = -1,              // OCTAL 모드용 (사용 안함)
        .data6_io_num = -1,              // OCTAL 모드용 (사용 안함)
        .data7_io_num = -1,              // OCTAL 모드용 (사용 안함)
        .max_transfer_sz = 4096,
        .flags = SPICOMMON_BUSFLAG_MASTER | SPICOMMON_BUSFLAG_QUAD,  // QUAD 모드 활성화
  
    };
    
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 10000000,      // 10MHz (QSPI는 더 빠르게 가능)
        .mode = 0,                       // SPI mode 0
        .spics_io_num = SPI_CS_PIN,
        .queue_size = 1,
        .flags = SPI_DEVICE_HALFDUPLEX,  // QSPI는 일반적으로 half-duplex
    };
    
    // SPI 버스 초기화 (DMA 비활성화로 시작)
    ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_DISABLED);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI bus init failed: %s", esp_err_to_name(ret));
        return;
    }
    
    ESP_LOGI(TAG, "QSPI bus initialized successfully");
    
    // SPI 디바이스 추가
    ret = spi_bus_add_device(SPI2_HOST, &devcfg, &spi_dev2);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI device add failed: %s", esp_err_to_name(ret));
        return;
    }

     ESP_LOGI(TAG, "QSPI device added successfully");

    
}


void msleep(int ms)
{
vTaskDelay(1000 / portTICK_PERIOD_MS);
}

void app_main(void)
{

    esp_err_t ret;
    spi_transaction_t t;

    msleep(1000);
    msleep(1000);
    msleep(1000);
    msleep(1000);
    msleep(1000);

    ESP_LOGI(TAG, "Starting SPI example");
    static uint8_t buf[1024] = {0x00,};
    for(int i=0; i<16 ; i++){
        buf[i] = 0x01 << (i % 8) ; 
        printf ("buf[%d] = 0x%02X\n", i, buf[i]);
    }




    // SPI 초기화
    // wizchip_reset();
    spi_init2();
    memset(&t, 0, sizeof(t));
    
    t.length = 8 * 8;
    t.tx_buffer = buf;
    t.rx_buffer = NULL;
    t.flags = SPI_TRANS_MODE_QIO; // QUAD I/O 모드로 데이터 수신
    ret = spi_device_transmit(spi_dev2, &t);





    
while(1){


}

    // wizchip_gpio_init();
}