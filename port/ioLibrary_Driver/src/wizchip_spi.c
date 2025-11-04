/**
    Copyright (c) 2022 WIZnet Co.,Ltd

    SPDX-License-Identifier: BSD-3-Clause
*/

/**
    ----------------------------------------------------------------------------------------------------
    Includes
    ----------------------------------------------------------------------------------------------------
*/
#include <stdio.h>
#include <inttypes.h>
#include "wizchip_spi.h"
#include <string.h>  
#include "wizchip_conf.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"      // ESP_LOGI, ESP_LOGE를 위해 추가
#include "esp_err.h"      // ESP_ERROR_CHECK를 위해 추가
/**
    ----------------------------------------------------------------------------------------------------
    Macros
    ----------------------------------------------------------------------------------------------------
*/

/**
    ----------------------------------------------------------------------------------------------------
    Variables
    ----------------------------------------------------------------------------------------------------
*/

#ifdef USE_SPI_DMA
static uint dma_tx;
static uint dma_rx;
static dma_channel_config dma_channel_config_tx;
static dma_channel_config dma_channel_config_rx;
#endif

/**
    ----------------------------------------------------------------------------------------------------
    Functions
    ----------------------------------------------------------------------------------------------------
*/
static inline void wizchip_select(void) {
    // gpio_put(PIN_CS, 0);
}

static inline void wizchip_deselect(void) {
    // gpio_put(PIN_CS, 1);w
}


spi_device_handle_t spi_dev;



// SPI 초기화 함수
void spi_init(void)
{
    esp_err_t ret;
    
    // SPI 버스 설정
    spi_bus_config_t buscfg = {
        .miso_io_num = SPI_MISO_PIN,
        .mosi_io_num = SPI_MOSI_PIN,
        .sclk_io_num = SPI_CLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };
    
    // SPI 디바이스 설정
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 1000000,  // 1MHz
        .mode = 0,                  // SPI mode 0
        .spics_io_num = SPI_CS_PIN,
        .queue_size = 7,
    };
    
    // SPI 버스 초기화
    ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);
    
    ESP_LOGI(TAG, "SPI bus initialized successfully");

    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi_dev));
    
}

void spi_init_qspi(void)
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
        .clock_speed_hz = 40000000,      // 10MHz (QSPI는 더 빠르게 가능)
        .mode = 0,                       // SPI mode 0
        .spics_io_num = SPI_CS_PIN,
        .queue_size = 1,
        .address_bits = 16,
        .command_bits = 8,
        .dummy_bits = 2,
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
    ret = spi_bus_add_device(SPI2_HOST, &devcfg, &spi_dev);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI device add failed: %s", esp_err_to_name(ret));
        return;
    }

     ESP_LOGI(TAG, "QSPI device added successfully");

    
}



#if 0
// SPI 데이터 전송 함수
void spi_send_data(spi_device_handle_t spi, uint8_t *data, size_t len)
{
    esp_err_t ret;
    spi_transaction_t t;
    
    memset(&t, 0, sizeof(t));
    t.length = len * 8;  // 비트 단위로 설정
    t.tx_buffer = data;
    
    ret = spi_device_transmit(spi, &t);
    ESP_ERROR_CHECK(ret);
    
    ESP_LOGI(TAG, "SPI data sent successfully");
}

// SPI 명령어/주소 전송 후 데이터 수신 함수 (정적 버퍼 버전)
void spi_receive_data(spi_device_handle_t spi, uint16_t addr , uint8_t cmd ,  uint8_t *data, size_t len)
{
    esp_err_t ret;
    spi_transaction_t t;
    
    memset(&t, 0, sizeof(t));
    
    // 최대 8바이트까지 지원 (명령어 1 + 주소 1 + 데이터 6)
    uint8_t tx_rx_buffer[8] = {0};
    uint8_t total_len = 3 + len;  // 명령어+주소(2바이트) + 수신데이터(len바이트)
    
    if (total_len > sizeof(tx_rx_buffer)) {
        ESP_LOGE(TAG, "Data length too large. Max supported: %d bytes", 
                 sizeof(tx_rx_buffer) - 2);
        return;
    }
    
    // 명령어와 주소 설정
    tx_rx_buffer[0] = (addr & 0xff00) >> 8 ;  // 명령어
    tx_rx_buffer[1] = (addr & 0x00ff) >> 0;  // 주소
    tx_rx_buffer[2] = cmd    & 0xff;  // 주소
    
    t.length = total_len * 8;  // total_len << 3 // 전체 길이 (비트 단위)
    t.tx_buffer = tx_rx_buffer;
    t.rx_buffer = tx_rx_buffer;  // 같은 버퍼 사용
    
    ret = spi_device_transmit(spi, &t);
    ESP_ERROR_CHECK(ret);
    
    // 수신된 데이터만 복사 (명령어/주소 부분 제외)
    memcpy(data, &tx_rx_buffer[3], len);
    
    ESP_LOGI(TAG, "SPI command sent: 0x%02X, address: 0x%02X", 
             tx_rx_buffer[0], tx_rx_buffer[1]);
    ESP_LOGI(TAG, "SPI data received successfully");
}

#endif 



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
// SPI 데이터 전송 함수
void spi_send_data( uint8_t *data, size_t len)
{
    esp_err_t ret;
    spi_transaction_t t;
    
    memset(&t, 0, sizeof(t));
    t.length = len * 8;  // 비트 단위로 설정
    t.tx_buffer = data;


    ret = spi_device_transmit(spi_dev, &t);
    ESP_ERROR_CHECK(ret);
    
}

// SPI 명령어/주소 전송 후 데이터 수신 함수 (정적 버퍼 버전)
void spi_receive_data( uint8_t *data,uint8_t *recv_data , size_t cmd_size , size_t len)
{
    esp_err_t ret;
    spi_transaction_t t;
    
    memset(&t, 0, sizeof(t));
    
    // 최대 8바이트까지 지원 (명령어 1 + 주소 1 + 데이터 6)
    uint32_t total_len = cmd_size + len;  // 명령어+주소(2바이트) + 수신데이터(len바이트)

    uint8_t* tx_rx_buffer = malloc(total_len);
    memset(tx_rx_buffer, 0, total_len);
    
    t.length = total_len * 8;  // total_len << 3 // 전체 길이 (비트 단위)
    memcpy(tx_rx_buffer, data, cmd_size);
    t.tx_buffer = tx_rx_buffer;  // 같은 버퍼 사용
    t.rx_buffer = tx_rx_buffer;  // 같은 버퍼 사용

    ret = spi_device_transmit(spi_dev, &t);
    ESP_ERROR_CHECK(ret);
    

    // 수신된 데이터만 복사 (명령어/주소 부분 제외)
    memcpy(recv_data, tx_rx_buffer + cmd_size , len);

    free(tx_rx_buffer);  // 메모리 해제
}


// GPIO 초기화 함수
void wizchip_gpio_init(void)
{
    // 리셋 핀을 출력으로 설정
    gpio_config_t io_conf = {0};
    io_conf.intr_type = GPIO_INTR_DISABLE;     // 인터럽트 비활성화
    io_conf.mode = GPIO_MODE_OUTPUT;           // 출력 모드
    io_conf.pin_bit_mask = (1ULL << SPI_RST_PIN); // 리셋 핀 설정
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    
    // 초기 상태를 HIGH로 설정 (리셋 비활성화)
    gpio_set_level(SPI_RST_PIN, 1);
    
    ESP_LOGI(TAG, "GPIO initialized - RST pin: %d", SPI_RST_PIN);
}

// 하드웨어 리셋 함수
void wizchip_hw_reset(void)
{
    ESP_LOGI(TAG, "Starting W5500 hardware reset...");



    // RST 핀을 LOW로 설정 (리셋 활성화)
    gpio_set_level(SPI_RST_PIN, 1);
    ESP_LOGI(TAG, "RST pin set to LOW");
    
    // 1ms 대기 (리셋 신호 유지)
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // RST 핀을 HIGH로 설정 (리셋 해제)
    gpio_set_level(SPI_RST_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(1000));

    gpio_set_level(SPI_RST_PIN, 1);

    ESP_LOGI(TAG, "RST pin set to HIGH");
    // 10ms 대기 (W5500 부팅 완료 대기)
    vTaskDelay(pdMS_TO_TICKS(10));
    
    ESP_LOGI(TAG, "W5500 hardware reset completed");
}

// 기존 wizchip_reset 함수 수정
void wizchip_reset() {
    wizchip_gpio_init() ;
    wizchip_hw_reset();
}


static uint8_t wizchip_read(void) {
    return 0;
}

static void wizchip_write(uint8_t tx_data) {
    
}



void wizchip_spi_initialize(void) {

}

void wizchip_cris_initialize(void) {

}

void wizchip_initialize(void) {


    /* W5x00, W6x00 initialize */
    uint8_t temp;
#if (_WIZCHIP_ == W5100S)
    uint8_t memsize[2][4] = {{2, 2, 2, 2}, {2, 2, 2, 2}};
#elif (_WIZCHIP_ == W5500)
    uint8_t memsize[2][8] = {{2, 2, 2, 2, 2, 2, 2, 2}, {2, 2, 2, 2, 2, 2, 2, 2}};
#elif (_WIZCHIP_ == W6100)
    uint8_t memsize[2][8] = {{2, 2, 2, 2, 2, 2, 2, 2}, {2, 2, 2, 2, 2, 2, 2, 2}};
#elif (_WIZCHIP_ == W6300)
    uint8_t memsize[2][8] = {{4, 4, 4, 4, 4, 4, 4, 4}, {4, 4, 4, 4, 4, 4, 4, 4}};
#endif

    if (ctlwizchip(CW_INIT_WIZCHIP, (void *)memsize) == -1) {
#if _WIZCHIP_ <= W5500
        printf(" W5x00 initialized fail\n");
#else
        printf(" W6x00 initialized fail\n");
#endif

        return;
    }
    /* Check PHY link status */
    do {
        if (ctlwizchip(CW_GET_PHYLINK, (void *)&temp) == -1) {
            printf(" Unknown PHY link status\n");

            return;
        }
    } while (temp == PHY_LINK_OFF);
}

void wizchip_check(void) {
#if (_WIZCHIP_ == W5100S)
    /* Read version register */
    if (getVER() != 0x51) {
        printf(" ACCESS ERR : VERSION != 0x51, read value = 0x%02x\n", getVER());

        while (1)
            ;
    }
#elif (_WIZCHIP_ == W5500)
    /* Read version register */
    if (getVERSIONR() != 0x04) {
        printf(" ACCESS ERR : VERSION != 0x04, read value = 0x%02x\n", getVERSIONR());

        while (1)
            ;
    }
#elif (_WIZCHIP_ == W6100)
    /* Read version register */
    if (getCIDR() != 0x6100) {
        printf(" ACCESS ERR : VERSION != 0x6100, read value = 0x%02x\n", getCIDR());

        while (1)
            ;
    }
#elif (_WIZCHIP_ == W6300)
    /* Read version register */
    if (getCIDR() != 0x6300) {
        printf(" ACCESS ERR : VERSION != 0x6100, read value = 0x%02x\n", getCIDR());

        while (1)
            ;
    }
#endif
}

/* Network */
void network_initialize(wiz_NetInfo net_info) {
#if _WIZCHIP_ <= W5500
    ctlnetwork(CN_SET_NETINFO, (void *)&net_info);
#else
    uint8_t syslock = SYS_NET_LOCK;
    ctlwizchip(CW_SYS_UNLOCK, &syslock);
    ctlnetwork(CN_SET_NETINFO, (void *)&net_info);
#endif
}

void print_network_information(wiz_NetInfo net_info) {
    uint8_t tmp_str[8] = {
        0,
    };

    ctlnetwork(CN_GET_NETINFO, (void *)&net_info);
    ctlwizchip(CW_GET_ID, (void *)tmp_str);
#if _WIZCHIP_ <= W5500
    if (net_info.dhcp == NETINFO_DHCP) {
        printf("====================================================================================================\n");
        printf(" %s network configuration : DHCP\n\n", (char *)tmp_str);
    } else {
        printf("====================================================================================================\n");
        printf(" %s network configuration : static\n\n", (char *)tmp_str);
    }

    printf(" MAC         : %02X:%02X:%02X:%02X:%02X:%02X\n", net_info.mac[0], net_info.mac[1], net_info.mac[2], net_info.mac[3], net_info.mac[4], net_info.mac[5]);
    printf(" IP          : %d.%d.%d.%d\n", net_info.ip[0], net_info.ip[1], net_info.ip[2], net_info.ip[3]);
    printf(" Subnet Mask : %d.%d.%d.%d\n", net_info.sn[0], net_info.sn[1], net_info.sn[2], net_info.sn[3]);
    printf(" Gateway     : %d.%d.%d.%d\n", net_info.gw[0], net_info.gw[1], net_info.gw[2], net_info.gw[3]);
    printf(" DNS         : %d.%d.%d.%d\n", net_info.dns[0], net_info.dns[1], net_info.dns[2], net_info.dns[3]);
    printf("====================================================================================================\n\n");
#else
    printf("==========================================================\n");
    printf(" %s network configuration\n\n", (char *)tmp_str);

    printf(" MAC         : %02X:%02X:%02X:%02X:%02X:%02X\n", net_info.mac[0], net_info.mac[1], net_info.mac[2], net_info.mac[3], net_info.mac[4], net_info.mac[5]);
    printf(" IP          : %d.%d.%d.%d\n", net_info.ip[0], net_info.ip[1], net_info.ip[2], net_info.ip[3]);
    printf(" Subnet Mask : %d.%d.%d.%d\n", net_info.sn[0], net_info.sn[1], net_info.sn[2], net_info.sn[3]);
    printf(" Gateway     : %d.%d.%d.%d\n", net_info.gw[0], net_info.gw[1], net_info.gw[2], net_info.gw[3]);
    printf(" DNS         : %d.%d.%d.%d\n", net_info.dns[0], net_info.dns[1], net_info.dns[2], net_info.dns[3]);
    // print_ipv6_addr(" GW6 ", net_info.gw6);
    // print_ipv6_addr(" LLA ", net_info.lla);
    // print_ipv6_addr(" GUA ", net_info.gua);
    // print_ipv6_addr(" SUB6", net_info.sn6);
    // print_ipv6_addr(" DNS6", net_info.dns6);
    // printf("==========================================================\n\n");
#endif
}

// void print_ipv6_addr(uint8_t* name, uint8_t* ip6addr) {
//     printf("%s        : ", name);
//     printf("%04X:%04X", ((uint16_t)ip6addr[0] << 8) | ((uint16_t)ip6addr[1]), ((uint16_t)ip6addr[2] << 8) | ((uint16_t)ip6addr[3]));
//     printf(":%04X:%04X", ((uint16_t)ip6addr[4] << 8) | ((uint16_t)ip6addr[5]), ((uint16_t)ip6addr[6] << 8) | ((uint16_t)ip6addr[7]));
//     printf(":%04X:%04X", ((uint16_t)ip6addr[8] << 8) | ((uint16_t)ip6addr[9]), ((uint16_t)ip6addr[10] << 8) | ((uint16_t)ip6addr[11]));
//     printf(":%04X:%04X\r\n", ((uint16_t)ip6addr[12] << 8) | ((uint16_t)ip6addr[13]), ((uint16_t)ip6addr[14] << 8) | ((uint16_t)ip6addr[15]));
// }