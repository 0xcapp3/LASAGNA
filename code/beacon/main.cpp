#include <stdio.h>

#include <periph/spi.h>
#include "SPBTLE_RF.hpp"
#include "beacon_service.h"

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
#define PIN_BLE_SPI_nCS    GPIO_PIN(PORT_A, 4) // to make this work, connect A1 and A2 with a jumper
#define PIN_BLE_SPI_RESET  GPIO_PIN(PORT_A, 8)
#define PIN_BLE_SPI_IRQ    GPIO_PIN(PORT_A, 0)
#define PIN_BLE_LED        GPIO_PIN(PORT_B, 6)
spi_t BTLE_SPI = SPI_DEV(0);
uint8_t SERVER_BDADDR[] = {0x12, 0x34, 0x00, 0xE1, 0x80, 0x04};
#endif

// Configure BTLE pins
SPBTLERFClass BTLE(BTLE_SPI, PIN_BLE_SPI_nCS, PIN_BLE_SPI_IRQ, PIN_BLE_SPI_RESET, PIN_BLE_LED);

//Comment this line to use URL mode
#define USE_UID_MODE

#ifdef USE_UID_MODE
// Beacon ID, the 6 last bytes are used for NameSpace
uint8_t NameSpace[] = "LASAGNA";
uint8_t beaconID[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6};
#else
char url[] = "www.myurl.com";
#endif

int main() {

    puts("Starting");
    if(BTLE.begin())
    {
        puts("Bluetooth module configuration error!");
        while(1);
    }
    puts("Started");

    #ifdef USE_UID_MODE
    // Enable the beacon service in UID mode
    if(BeaconService.begin(SERVER_BDADDR, beaconID, NameSpace))
    {
        puts("Beacon service configuration error!");
        while(1);
    }
    else
    {
        puts("Beacon service started!");
    }
    #else
    //Enable the beacon service in URL mode
    if(BeaconService.begin(SERVER_BDADDR, url))
    {
        puts("Beacon service configuration error!");
        while(1);
    }
    else
    {
        puts("Beacon service started!");
    }
    #endif

    puts("Initialized");
    while(1) {
        // Update the BLE module state
        BTLE.update();
    }
}
