/**
    Copyright (c) 2022 WIZnet Co.,Ltd

    SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef _WIZCHIP_SPI_H_
#define _WIZCHIP_SPI_H_
#include "driver/spi_master.h"

#include <inttypes.h>
#include "wizchip_conf.h"
/**
    ----------------------------------------------------------------------------------------------------
    Macros
    ----------------------------------------------------------------------------------------------------
*/
/* SPI */

static const char *TAG = "SPI_EXAMPLE";

/* SPI */
#define SPI_PORT spi0



// QSPI 핀 설정 (기존 SPI 핀을 QSPI로 확장)

#define SPI_D0_PIN      47   // DATA0 (기존 MOSI)
#define SPI_D1_PIN      38   // DATA1 (기존 MISO)  
#define SPI_D2_PIN      21   // DATA2 (WP) - 새로 추가
#define SPI_D3_PIN      18   // DATA3 (HD) - 새로 추가

#define SPI_CLK_PIN     21   // CLK
#define SPI_CS_PIN      18   // CS

// #define SPI_D0_PIN      37   // DATA0 (기존 MOSI)
// #define SPI_D1_PIN      36 // DATA1 (기존 MISO)  
// #define SPI_D2_PIN      35    // DATA2 (WP) - 새로 추가
// #define SPI_D3_PIN      34   // DATA3 (HD) - 새로 추가

// #define SPI_CLK_PIN     46   // CLK
// #define SPI_CS_PIN      45   // CS



// SPI 핀 설정
#define SPI_MOSI_PIN    47
#define SPI_MISO_PIN    38

#define SPI_RST_PIN     17
#define SPI_PIN_INT     10

extern spi_device_handle_t spi_dev;

// SPI 초기화 함수
void spi_init(void);
void spi_init_qspi(void);

// SPI 데이터 전송 함수
void spi_send_data( uint8_t *data, size_t len) ;


// SPI 명령어/주소 전송 후 데이터 수신 함수 (정적 버퍼 버전)
void spi_receive_data(uint8_t *data,uint8_t *recv_data , size_t cmd_size , size_t len);
void qspi_read_data_fast(uint8_t cmd, uint32_t addr, uint8_t *data, size_t len);
void qspi_write_data(uint8_t cmd , uint16_t addr, uint8_t *data, size_t len);
void qspi_read_data(uint8_t cmd , uint16_t addr, uint8_t *data, size_t len);
/* Use SPI DMA */
//#define USE_SPI_DMA // if you want to use SPI DMA, uncomment.


/**
    ----------------------------------------------------------------------------------------------------
    Functions
    ----------------------------------------------------------------------------------------------------
*/
/* wizchip */
/*! \brief Set CS pin
    \ingroup wizchip_spi

    Set chip select pin of spi0 to low(Active low).

    \param none
*/
static inline void wizchip_select(void);

/*! \brief Set CS pin
    \ingroup wizchip_spi

    Set chip select pin of spi0 to high(Inactive high).

    \param none
*/
static inline void wizchip_deselect(void);

/*! \brief Read from an SPI device, blocking
    \ingroup wizchip_spi

    Set spi_read_blocking function.
    Read byte from SPI to rx_data buffer.
    Blocks until all data is transferred. No timeout, as SPI hardware always transfers at a known data rate.

    \param none
*/
static uint8_t wizchip_read(void);

/*! \brief Write to an SPI device, blocking
    \ingroup wizchip_spi

    Set spi_write_blocking function.
    Write byte from tx_data buffer to SPI device.
    Blocks until all data is transferred. No timeout, as SPI hardware always transfers at a known data rate.

    \param tx_data Buffer of data to write
*/
static void wizchip_write(uint8_t tx_data);


#if (_WIZCHIP_ == W6100)
static void wizchip_read_buf(uint8_t* rx_data, datasize_t len);
static void wizchip_write_buf(uint8_t* tx_data, datasize_t len);
#endif

#ifdef USE_SPI_DMA
/*! \brief Configure all DMA parameters and optionally start transfer
    \ingroup wizchip_spi

    Configure all DMA parameters and read from DMA

    \param pBuf Buffer of data to read
    \param len element count (each element is of size transfer_data_size)
*/
static void wizchip_read_burst(uint8_t *pBuf, uint16_t len);

/*! \brief Configure all DMA parameters and optionally start transfer
    \ingroup wizchip_spi

    Configure all DMA parameters and write to DMA

    \param pBuf Buffer of data to write
    \param len element count (each element is of size transfer_data_size)
*/
static void wizchip_write_burst(uint8_t *pBuf, uint16_t len);
#endif

/*! \brief Enter a critical section
    \ingroup wizchip_spi

    Set ciritical section enter blocking function.
    If the spin lock associated with this critical section is in use, then this
    method will block until it is released.

    \param none
*/
static void wizchip_critical_section_lock(void);

/*! \brief Release a critical section
    \ingroup wizchip_spi

    Set ciritical section exit function.
    Release a critical section.

    \param none
*/
static void wizchip_critical_section_unlock(void);

/*! \brief Initialize SPI instances and Set DMA channel
    \ingroup wizchip_spi

    Set GPIO to spi0.
    Puts the SPI into a known state, and enable it.
    Set DMA channel completion channel.

    \param none
*/
void wizchip_spi_initialize(void);

/*! \brief Initialize a critical section structure
    \ingroup wizchip_spi

    The critical section is initialized ready for use.
    Registers callback function for critical section for WIZchip.

    \param none
*/
void wizchip_cris_initialize(void);

/*! \brief wizchip reset
    \ingroup wizchip_spi

    Set a reset pin and reset.

    \param none
*/
void wizchip_hw_reset(void);

void wizchip_reset(void);

/*! \brief Initialize WIZchip
    \ingroup wizchip_spi

    Set callback function to read/write byte using SPI & QSPI.
    Set callback function for WIZchip select/deselect.
    Set memory size of wizchip and monitor PHY link status.

    \param none
*/
void wizchip_initialize(void);

/*! \brief Check chip version
    \ingroup wizchip_spi

    Get version information.

    \param none
*/
void wizchip_check(void);

/* Network */
/*! \brief Initialize network
    \ingroup wizchip_spi

    Set network information.

    \param net_info network information.
*/
void network_initialize(wiz_NetInfo net_info);

/*! \brief Print network information
    \ingroup wizchip_spi

    Print network information about MAC address, IP address, Subnet mask, Gateway, DHCP and DNS address.

    \param net_info network information.
*/
void print_network_information(wiz_NetInfo net_info);

/*! \brief Print IPv6 Address
    \ingroup wizchip_spi

    Print IPv6 Address.

    \param net_info network information.
*/
void print_ipv6_addr(uint8_t* name, uint8_t* ip6addr);

#endif /* _WIZCHIP_SPI_H_ */