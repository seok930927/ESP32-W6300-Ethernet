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

#define ETHERNET_BUF_MAX_SIZE (1024 * 2)
static uint8_t g_tcp_server_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
};


// static const char * TAG= "WIZCHIP_TEST"; 





void msleep(int ms)
{
vTaskDelay(ms / portTICK_PERIOD_MS);
}

void qspi_write_data(uint8_t cmd , uint16_t addr, uint8_t *data, size_t len){
    esp_err_t ret;

    static uint8_t full_buffer[16];
    spi_transaction_t t;

    memset(&t, 0, sizeof(t));
    t.length =  len * 8 ;
    t.addr = addr;
    t.cmd = cmd | 0x20;
    t.tx_buffer = data ;
    t.rx_buffer = NULL;
    t.rxlength = 0;
    t.flags = SPI_TRANS_MODE_QIO  | SPI_TRANS_MULTILINE_ADDR    ;// QUAD I/O 모드로 데이터 수신
     ret = spi_device_transmit(spi_dev, &t);

#if 0
    printf("RX DATA: ");
    for(int i=0; i<len; i++){
        printf("0x%02X ", data[i]);
    }
    printf("\n");

#endif 
    

}


void qspi_read_data(uint8_t cmd , uint16_t addr, uint8_t *data, size_t len){
    esp_err_t ret;

    static uint8_t full_buffer[16];
    spi_transaction_t t;

    memset(&t, 0, sizeof(t));
    t.length = 0 ;
    t.addr = addr;
    t.cmd = cmd;
    t.tx_buffer = NULL ;
    t.rx_buffer = data ;
    t.rxlength = len * 8;
    t.flags = SPI_TRANS_MODE_QIO  | SPI_TRANS_MULTILINE_ADDR    ;// QUAD I/O 모드로 데이터 수신
     ret = spi_device_transmit(spi_dev, &t);

#if 0
    printf("RX DATA: ");
    for(int i=0; i<len; i++){
        printf("0x%02X ", data[i]);
    }
    printf("\n");

#endif 
    

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
    static uint8_t buf_tx[8] = {0x12,0x48,0x12,0x48,0x12,0x48,0x12,0x48};
    static uint8_t buf_rx[8] = {0x00,};




    // SPI 초기화
    // wizchip_reset();
    spi_init_qspi();


    
    static uint8_t full_buffer[16];
    memcpy(full_buffer, buf_tx, 8);  // 처음 8바이트는 송신 데이터
    memset(full_buffer + 8, 0, 8);   // 나머지 8바이트는 0으로 초기화
    memset(&t, 0, sizeof(t));
    t.length = 0 ;
    t.addr = 0x0000;
    t.cmd = 0x80 ; 
    t.tx_buffer = NULL ;
    t.rx_buffer = buf_rx ;
    t.rxlength = 8*8;
    t.flags = SPI_TRANS_MODE_QIO  | SPI_TRANS_MULTILINE_ADDR    ;// QUAD I/O 모드로 데이터 수신
     ret = spi_device_transmit(spi_dev, &t);
    printf("RX DATA: ");
    for(int i=0; i<8; i++){
        printf("0x%02X ", buf_rx[i]);
    }
    printf("\n");


    

while(1){
    uint8_t buf_rx[8] = {0x00,};
    uint8_t buf_IP[8] = {192, 168, 11, 99};
    uint8_t buf_GUAR[16] = {192, 168, 11, 99,
        192, 168, 11, 99,
        192, 168, 11, 99,
        192, 168, 11, 99};
    uint8_t buf_rx_16[16] = {0x00,};
        
    qspi_read_data(0x80, 0x0000, buf_rx, 1);
    printf("Read from address 0x0000: 0x%02x \n", buf_rx[0] );
    qspi_read_data(0x80, 0x0001, buf_rx, 1);
    printf("Read from address 0x0001: 0x%02x \n", buf_rx[0] );
    qspi_read_data(0x80, 0x0002, buf_rx, 1);
    printf("Read from address 0x0002: 0x%02x \n", buf_rx[0] );
    qspi_read_data(0x80, 0x0003, buf_rx, 1);
    printf("Read from address 0x0003: 0x%02x \n", buf_rx[0] );



    
    qspi_read_data(0x80, 0x4138, buf_rx, 4);
    printf(" IP = : [%d.%d.%d.%d] \n", buf_rx[0], buf_rx[1], buf_rx[2], buf_rx[3] );

    qspi_write_data(0x80, 0x4138, buf_IP, 4);

    qspi_read_data(0x80, 0x4138, buf_rx, 4);
    printf(" IP = : [%d.%d.%d.%d] \n", buf_rx[0], buf_rx[1], buf_rx[2], buf_rx[3] );

   qspi_write_data(0x80, 0x4150, buf_GUAR,  16);

    qspi_read_data(0x80, 0x4150, buf_rx_16, 16 );
    printf(" IP = : [%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d] \n", buf_rx_16[0], buf_rx_16[1], buf_rx_16[2], buf_rx_16[3],
                                 buf_rx_16[4], buf_rx_16[5], buf_rx_16[6], buf_rx_16[7],
                                 buf_rx_16[8], buf_rx_16[9], buf_rx_16[10], buf_rx_16[11],
                                 buf_rx_16[12], buf_rx_16[13], buf_rx_16[14], buf_rx_16[15] );
    msleep(1000);
}

    // wizchip_gpio_init();
}