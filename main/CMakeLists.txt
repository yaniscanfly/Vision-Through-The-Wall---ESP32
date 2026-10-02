set(reqs esp_wifi_sensing
        protocol_examples_common
        driver
        esp_driver_ledc
        led_strip
        esp_http_server
        esp_netif
        mdns
        dns_server)

idf_component_register(SRCS "app_main.c"
                            "led_control.c"
                            "web_serial_monitor.c"
                            "wifi_web_server.c"
                            "yanis_wifi.c"
                       INCLUDE_DIRS "."
                       REQUIRES ${reqs}
                       PRIV_REQUIRES esp_driver_usb_serial_jtag
                       EMBED_TXTFILES "web/portal.html"
                                      "web/saved.html"
                                      "web/dashboard.html"
                                      "web/theme.css")
