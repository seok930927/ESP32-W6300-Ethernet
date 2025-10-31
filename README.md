


<img width="1152" height="674" alt="image" src="https://github.com/user-attachments/assets/c41ecac1-5872-4fcc-84b7-50a131d8d9ef" />


### Pin Define
```c
#define SPI_D0_PIN      47   // DATA0 (기존 MOSI)
#define SPI_D1_PIN      38   // DATA1 (기존 MISO)  
#define SPI_D2_PIN      21   // DATA2 (WP) - 새로 추가
#define SPI_D3_PIN      18   // DATA3 (HD) - 새로 추가

#define SPI_CLK_PIN     17   // CLK
#define SPI_CS_PIN      10   // CS
```
### Qspi  config 
```c
 spi_device_handle_t spi_dev2;

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

```

<img width="611" height="821" alt="image" src="https://github.com/user-attachments/assets/735e9567-f754-4a15-b9d4-a34d0c97e189" />


