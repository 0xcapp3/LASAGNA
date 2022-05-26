#include <stdio.h>

#include <periph/spi.h>
#include "SPBTLE_RF.hpp"
#include "beacon_service.h"
#include "ztimer.h"

#define PIN_BLE_SPI_nCS    GPIO_PIN(PORT_D, 13)
#define PIN_BLE_SPI_RESET  GPIO_PIN(PORT_A, 8)
#define PIN_BLE_SPI_IRQ    GPIO_PIN(PORT_E, 6)
#define PIN_BLE_LED    GPIO_PIN(PORT_A, 5)
spi_t BTLE_SPI = SPI_DEV(2);
uint8_t SERVER_BDADDR[] = {0x12, 0x34, 0x00, 0xE1, 0x80, 0x03};

// Configure BTLE pins
SPBTLERFClass BTLE(BTLE_SPI, PIN_BLE_SPI_nCS, PIN_BLE_SPI_IRQ, PIN_BLE_SPI_RESET, PIN_BLE_LED);

// Beacon ID, the 6 last bytes are used for NameSpace
uint8_t NameSpace[] = "LASAGNA";
uint8_t beaconID[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6};

void disable_unused_components()
{
    gpio_t wifi_reset = GPIO_PIN(PORT_E, 8);
    gpio_t qspi_ncs = GPIO_PIN(PORT_E, 11);
    gpio_t radio_ncs = GPIO_PIN(PORT_B, 5);
    gpio_t VL53L0X_xshut = GPIO_PIN(PORT_C, 6);
    gpio_t nfc_disable = GPIO_PIN(PORT_E, 2);

    // Keep the wifi module in reset in order to save ~250mW
    gpio_init(wifi_reset, GPIO_OUT);
    gpio_clear(wifi_reset);

    // QSPI consumes ~10mW if active
    gpio_init(qspi_ncs, GPIO_OUT);
    gpio_set(qspi_ncs);

    // Disable the remaining components even if they don't significantly
    // contribute to power consumption

    gpio_init(radio_ncs, GPIO_OUT);
    gpio_set(radio_ncs);

    gpio_init(VL53L0X_xshut, GPIO_OUT);
    gpio_set(VL53L0X_xshut);

    gpio_init(nfc_disable, GPIO_OUT);
    gpio_set(nfc_disable);
}

uint8_t stop_ble = 0;

void timer_callback(void *arg) {
    (void) arg;
    stop_ble = 1;
}

void send_id() {
    puts("Starting");
    if(BTLE.begin())
    {
        puts("Bluetooth module configuration error!");
        while(1);
    }
    puts("Started");

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

    puts("Initialized");

    stop_ble = 0;

    ztimer_t timeout;
    timeout.callback = timer_callback;
    timeout.arg = NULL;
    ztimer_set(ZTIMER_SEC, &timeout, 5);

    while(!stop_ble) {
        // Update the BLE module state
        BTLE.update();
    }

    puts("Stopped");
}

int main() {
    disable_unused_components();

    while(1) {
        send_id();
        ztimer_sleep(ZTIMER_SEC, 15);
    }
}
