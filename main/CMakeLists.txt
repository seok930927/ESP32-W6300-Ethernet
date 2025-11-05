idf_component_register(SRCS "wizchip_TEST.c"
                            "../port/ioLibrary_Driver/src/wizchip_spi.c"   
                            "../lib/ioLibrary_Driver/Ethernet/wizchip_conf.c"
                            "../lib/ioLibrary_Driver/Ethernet/socket.c"
                            "../lib/ioLibrary_Driver/Ethernet/W5500/w5500.c"
                            "../lib/ioLibrary_Driver/Ethernet/W6300/w6300.c"
                            "../lib/iperf/iperf.c"
                            "../lib/iperf/cJSON.c"
                            "../lib/ioLibrary_Driver/Application/loopback/loopback.c"
                       PRIV_REQUIRES spi_flash driver esp_timer
                    #    INCLUDE_DIRS "")
                       INCLUDE_DIRS "" 
                                    "../lib/ioLibrary_Driver/Ethernet"
                                    "../lib/ioLibrary_Driver/Ethernet/W5500"
                                    "../lib/ioLibrary_Driver/Ethernet/W6300"
                                    "../lib/ioLibrary_Driver/Internet"
                                    "../lib/ioLibrary_Driver/Application"
                                    "../lib/ioLibrary_Driver/Application/loopback"
                                    "../port/ioLibrary_Driver/inc"
                                    "../lib/iperf"

                                    )