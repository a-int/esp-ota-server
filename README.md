# ESP-OTA-Server
The basic HTTP-server with functionallity to upload, delete, download and flash binaries onto the stm32. The project is not supposed to flash the stm32 yourself, just adding the wifi feature for already written stm [bootloader](https://github.com/a-int/stm-bootloader).

# Details

After powering on, the ESP32 starts the HTTP-server (please be sure the SSID and PASSWORD is set correctly in menuconfig). Afther the server is started ESP32 can handler HTTP requests. Files are saved in SPIFFS memory and the maximum size for one file is 200 KiB (may be changed in main/server.c). For the safety every command checks the correctnes of the name, its length, wheter the file is already exist and so on. If any error is found the command is aborted and the error is printed on both: terminal/app where the request is made and esp monitor. 
The UART used to transmitt data is defined in main/uart.c (by default UART_NUM_2). 

The following commands are supported:

- to upload the file onto the server (please be sure the lenght of file-name-on-server is less than 32 character);
    ```
    curl -X POST --data-binary @file-name-on-pc ip:port/upload/file-name-on-server
    ```
- to delete the file from the server;
    ```
    curl -X POST ip:port/delete/file-name-on-server
    ```
- to flash the file to stm32 via UART;
    ```
    curl -X GET ip:port/flash/file-name-on-server
    ```
- to download the file from the server onto PC.
    ```
    curl -X GET ip:port/download/file-name-on-server > file-name-to-be-saved
    ```

# Usage
* Make sure you have installed esp-idf according to [official installation guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/#installation)
    ```
    $ idf.py --version
    ```
    If any error returned check whether the [environment variables](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/linux-macos-setup.html#step-4-set-up-the-environment-variables) are set up correctly
 * if everything is installed then you can clone this repo and do the following:
	```
	$ cd <path-of-this-repo>
    $ idf.py --list-targets 
    $ idf.py set-target <your-esp-model>
    $ idf.py build
    $ idf.py flash -p <path-to-your-usbserial>
    $ idf.py monitor
	```
* after that you need to wire you ESP32 with stm32 (ESP_TX <-> STM_RX; ESP_RX <-> STM_TX).

# Troubleshooting

- Nothing works(((
    - check that ESP and STM are wirred correctly. By default ESP_TX is D17, and ESP_RX is D16. Be sure the TX on ESP is connected to RX on STM and vice versa.
- The data isn't sent to STM32: 
    - Be sure the UART is configured identically on both chips;
    - Check that stm32 UART IRQ is written properly (everything was tested with this [bootloader](https://github.com/a-int/stm-bootloader));
- The transmitted data is wrong:
    - Be sure the UART is configured identically on both chips;
    - Try to power ESP32 and STM32 separately and start stm32 after the ESP is started.


# References
- ESP WiFi driver [documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/wifi.html#esp32-wi-fi-station-general-scenario)
- ESP SPIFFS [api-reference](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/spiffs.html)
- ESP UART [api-reference](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/uart.html)
- Basic HTTP-Server [example](https://github.com/espressif/esp-idf/tree/master/examples/protocols/http_server/file_serving)
