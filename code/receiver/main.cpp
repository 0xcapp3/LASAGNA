#include <stdio.h>

#include <periph/spi.h>
#include "SPBTLE_RF.hpp"
#include "observer.h"

// DISCO BOARD
#ifdef BOARD_B_L475E_IOT01A
#define PIN_BLE_SPI_nCS    GPIO_PIN(PORT_D, 13)
#define PIN_BLE_SPI_RESET  GPIO_PIN(PORT_A, 8)
#define PIN_BLE_SPI_IRQ    GPIO_PIN(PORT_E, 6)
#define PIN_BLE_LED    GPIO_PIN(PORT_A, 5)
spi_t BTLE_SPI = SPI_DEV(2);
uint8_t SERVER_BDADDR[] = {0x12, 0x34, 0x00, 0xE1, 0x80, 0x03};
#endif

// NUCLEO BOARD
#ifdef BOARD_NUCLEO_F401RE
#define PIN_BLE_SPI_nCS    GPIO_PIN(PORT_A, 1)
#define PIN_BLE_SPI_RESET  GPIO_PIN(PORT_A, 8)
#define PIN_BLE_SPI_IRQ    GPIO_PIN(PORT_A, 0)
#define PIN_BLE_LED        GPIO_PIN(PORT_A, 5)
spi_t BTLE_SPI = SPI_DEV(0);
#endif

// LRWAN1 BOARD
#ifdef BOARD_B_L072Z_LRWAN1
#define PIN_BLE_SPI_nCS    GPIO_PIN(PORT_A, 4)
#define PIN_BLE_SPI_RESET  GPIO_PIN(PORT_A, 8)
#define PIN_BLE_SPI_IRQ    GPIO_PIN(PORT_A, 0)
#define PIN_BLE_LED        GPIO_PIN(PORT_B, 6)
spi_t BTLE_SPI = SPI_DEV(0);
uint8_t SERVER_BDADDR[] = {0x12, 0x34, 0x00, 0xE1, 0x80, 0x04};
#endif

// Configure BTLE pins
SPBTLERFClass BTLE(BTLE_SPI, PIN_BLE_SPI_nCS, PIN_BLE_SPI_IRQ, PIN_BLE_SPI_RESET, PIN_BLE_LED);

//Comment this line to use URL mode
// #define USE_UID_MODE

#ifdef USE_UID_MODE
// Beacon ID, the 6 last bytes are used for NameSpace
uint8_t NameSpace[] = "ST BTLE";
uint8_t beaconID[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6};
#else
char url[] = "www.myurl.com";
#endif

void advertising_report_cb(le_advertising_info *adv)
{
    bool ok = false;
    for(int i = 0; i+2 < adv->data_length; i++)
    {
        if(adv->data_RSSI[i] == 'u' && adv->data_RSSI[i+1] == 'r' && adv->data_RSSI[i+2] == 'l')
            ok = true;
    }
    if(!ok) return;

    puts("=== Advertising report ===");
    printf(" evt_type = %d\n", adv->evt_type);
    printf(" bdaddr_type = %d\n", adv->bdaddr_type);
    printf(" tBDAddr = %02X %02X %02X %02X %02X %02X\n",
        adv->bdaddr[0], adv->bdaddr[1], adv->bdaddr[2],
        adv->bdaddr[3], adv->bdaddr[4], adv->bdaddr[5]
    );
    printf(" data_length = %d\n", adv->data_length);
    printf(" data: ");
    for(int i = 0; i + 1 < adv->data_length; i++)
        printf(" %02X", adv->data_RSSI[i]);
    putchar('\n');
    for(int i = 0; i < adv->data_length; i++) {
        if(adv->data_RSSI[i] >= 32 && adv->data_RSSI[i] <= 127)
            putchar(adv->data_RSSI[i]);
        else
            putchar('.');
    }
    putchar('\n');

    printf(" RSSI: %d\n", adv->data_RSSI[adv->data_length]);
    puts("======");
}

int main() {

    puts("Starting");
    if(BTLE.begin())
    {
        puts("Bluetooth module configuration error!");
        while(1);
    }
    puts("Started");

    ObserverService.setAdvertisingCallback(advertising_report_cb);

    if(ObserverService.begin(SERVER_BDADDR))
    {
        puts("Observer init failed");
        while(1);
    }

    puts("Initialized");
    while(1) {
        // Update the BLE module state
        BTLE.update();
    }
}
