#define ENABLE_DEBUG 0

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "xtimer.h" // https://doc.riot-os.org/xtimer_8h.html

// BLE includes
#include <periph/spi.h>
#include "SPBTLE_RF.hpp"
#include "observer.h"

// LoRa includes
#include "fmt.h"

#define public public_
#include "net/loramac.h"
#include "semtech_loramac.h"
#undef public

#if IS_USED(MODULE_SX127X)
#include "sx127x.h" //SX1276
#include "sx127x_netdev.h"
#include "sx127x_params.h"
#endif

// GPS includes
#include "minmea.h"
#include "periph/uart.h"
#include "isrpipe.h"

#include "mutex.h"

// LRWAN1 BOARD
#define PIN_BLE_SPI_nCS GPIO_PIN(PORT_A, 4)
#define PIN_BLE_SPI_RESET GPIO_PIN(PORT_A, 8)
#define PIN_BLE_SPI_IRQ GPIO_PIN(PORT_A, 0)
#define PIN_BLE_LED GPIO_PIN(PORT_B, 6)
spi_t BTLE_SPI = SPI_DEV(0);
uint8_t SERVER_BDADDR[] = {0x12, 0x34, 0x00, 0xE1, 0x80, 0x04};
SPBTLERFClass BTLE(BTLE_SPI, PIN_BLE_SPI_nCS, PIN_BLE_SPI_IRQ, PIN_BLE_SPI_RESET, PIN_BLE_LED);

// #define DEBUG

static semtech_loramac_t loramac;
#if IS_USED(MODULE_SX127X)
static sx127x_t sx127x;
#endif

static uint8_t deveui[LORAMAC_DEVEUI_LEN];
static uint8_t appeui[LORAMAC_APPEUI_LEN];
static uint8_t appkey[LORAMAC_APPKEY_LEN];

#define MESSAGE_MAXLEN (80U)

// Last data received from GPS
struct {
    float lat, lon;
    int timestamp;
} gps_state = {0,0,0};


// Ping data queue
typedef struct {
    uint8_t id[6];
} ble_ping_t;

constexpr int PING_QUEUE_LEN = 64;

struct {
    mutex_t mutex = MUTEX_INIT;
    ble_ping_t list[PING_QUEUE_LEN];
    size_t size = 0;
} ping_queue;


char lora_thread_stack[THREAD_STACKSIZE_MAIN];

static const char message_structure[] = "{\"beacon_ids\":[%s], \"receiver_id\":\"%d\", \"lat\":\"%f\", \"lon\":\"%f\", \"timestamp\":\"%d\"}";
char beacon_ids_buffer[PING_QUEUE_LEN * 15];
char lora_buffer[sizeof(beacon_ids_buffer) + 256];
int8_t sample_rate = 15; // X s

void* lora_thread(void* arg)
{
    (void)arg;

    puts("[+] LoRa thread...");

    while (1)
    {
        if(gps_state.timestamp == 0) {
            puts("Waiting for GPS lock");
        } else {

            if(ping_queue.size == 0) {
                puts("Ping queue empty, skipping send");
            } else {

                mutex_lock(&ping_queue.mutex);

                for(size_t i = 0; i < ping_queue.size; i++) {
                    ble_ping_t ping = ping_queue.list[i];

                    sprintf(beacon_ids_buffer + i * 15, "\"%02X%02X%02X%02X%02X%02X\",", ping.id[0], ping.id[1], ping.id[2], ping.id[3], ping.id[4], ping.id[5]);
                    if(i == ping_queue.size - 1)
                        beacon_ids_buffer[15 * i + 14] = 0;
                }
                ping_queue.size = 0;

                mutex_unlock(&ping_queue.mutex);

                // Send data via Lora
                int id_rcvr = 001;

                sprintf(lora_buffer, message_structure, beacon_ids_buffer, id_rcvr, gps_state.lat, gps_state.lon, gps_state.timestamp);
                printf("Sending: %s\r\n", lora_buffer);

                /* Try to send the message */
                uint8_t ret = semtech_loramac_send(&loramac, (uint8_t *)lora_buffer, strlen(lora_buffer));
                if (ret != SEMTECH_LORAMAC_TX_DONE)
                {
                    printf("[X] Cannot send message '%s', returned code: %u\n", lora_buffer, ret);
                }
            }

        }
        xtimer_sleep(sample_rate);
    }
    return NULL;
}

int8_t lora_initialization(void) {

    /* Convert identifiers and application key */
    fmt_hex_bytes(deveui, "E24F43FFFE39CB71");
    fmt_hex_bytes(appeui, "00000000000000A1");
    fmt_hex_bytes(appkey, "FA401D00EBF0F36C56E2D8409E2D5352");

    /* Initialize the radio driver */
#if IS_USED(MODULE_SX127X)
    sx127x_setup(&sx127x, &sx127x_params[0], 0);
    loramac.netdev = &sx127x.netdev;
    loramac.netdev->driver = &sx127x_driver;
#endif

    /* Initialize the loramac stack */
    if (semtech_loramac_init(&loramac) == -1)
    {
        printf("semtech_loramac_init() failed!\r\n");
        return (-1);
    }

    semtech_loramac_set_deveui(&loramac, deveui);
    semtech_loramac_set_appeui(&loramac, appeui);
    semtech_loramac_set_appkey(&loramac, appkey);

    u_int8_t tmp1[50];
    semtech_loramac_get_deveui(&loramac, tmp1);
    printf("[+] deveui: %x %x %x %x %x %x %x %x \r\n", tmp1[0], tmp1[1], tmp1[2], tmp1[3], tmp1[4], tmp1[5], tmp1[6], tmp1[7]);

    u_int8_t tmp2[50];
    semtech_loramac_get_appeui(&loramac, tmp2);
    printf("[+] appeui: %x %x %x %x %x %x %x %x \r\n", tmp2[0], tmp2[1], tmp2[2], tmp2[3], tmp2[4], tmp2[5], tmp2[6], tmp2[7]);

    u_int8_t tmp3[50];
    semtech_loramac_get_appkey(&loramac, tmp3);
    printf("[+] appkey: %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x \r\n", tmp3[0], tmp3[1], tmp3[2], tmp3[3], tmp3[4], tmp3[5], tmp3[6], tmp3[7], tmp3[8], tmp3[9], tmp3[10], tmp3[11], tmp3[12], tmp3[13], tmp3[14], tmp3[15]);

    /* Use a fast datarate, e.g. BW125/SF7 in EU868 */
    semtech_loramac_set_dr(&loramac, 5);

    /* Start the Over-The-Air Activation (OTAA) procedure to retrieve the
     * generated device address and to get the network and application session
     * keys.
     */

    puts("Starting join procedure");
    u_int8_t res = semtech_loramac_join(&loramac, LORAMAC_JOIN_OTAA);
    switch (res)
    {
    case SEMTECH_LORAMAC_JOIN_FAILED:
        puts("semtech_loramac_join() - SEMTECH_LORAMAC_JOIN_FAILED");
        return (-1);
        break;
    case SEMTECH_LORAMAC_BUSY:
        puts("semtech_loramac_join() - SEMTECH_LORAMAC_BUSY - the mac is already active (join or tx in progress)");
        return (-1);
        break;
    case SEMTECH_LORAMAC_ALREADY_JOINED:
        puts("semtech_loramac_join() - SEMTECH_LORAMAC_ALREADY_JOINED - network was already joined");
        return (-1);
        break;
    default:
        break;
    }

    puts("Join procedure succeeded");
    return (0);
}

uint8_t tmp_buffer[sizeof(le_advertising_info) + 256];

void advertising_report_cb(le_advertising_info *adv_in)
{
    memcpy(tmp_buffer, adv_in, sizeof(le_advertising_info) + adv_in->data_length + 1);
    le_advertising_info *adv = (le_advertising_info *)tmp_buffer;

    bool ok = false;
    uint8_t bid[6];
    for (int i = 0; i + 15 < adv->data_length && !ok; i++)
    {
        if(!memcmp(adv->data_RSSI + i, "LASAGNA", 7))
        {
            memcpy(bid, adv->data_RSSI + i + 10, 6);
            ok = true;
        }
    }
    if (!ok)
        return;

    printf("[=] beacon id: ");
    for (int i = 0; i < 6; i++)
        printf(" %02X", bid[i]);
    printf("\r\n");

    mutex_lock(&ping_queue.mutex);

    if(ping_queue.size < PING_QUEUE_LEN) {
        memcpy(&ping_queue.list[ping_queue.size], bid, 6);
        ping_queue.size++;
    } else {
        puts("[+] Ping queue full, dropping packet");
    }

    mutex_unlock(&ping_queue.mutex);

#ifdef DEBUG
    puts("=== Advertising report ===");
    printf(" evt_type = %d\n", adv->evt_type);
    printf(" bdaddr_type = %d\n", adv->bdaddr_type);
    printf(" tBDAddr = %02X %02X %02X %02X %02X %02X\n",
           adv->bdaddr[0], adv->bdaddr[1], adv->bdaddr[2],
           adv->bdaddr[3], adv->bdaddr[4], adv->bdaddr[5]);
    printf(" data_length = %d\n", adv->data_length);
    printf(" data: ");
    for (int i = 0; i + 1 < adv->data_length; i++)
        printf(" %02X", adv->data_RSSI[i]);
    putchar('\n');
    for (int i = 0; i < adv->data_length; i++)
    {
        if (adv->data_RSSI[i] >= 32 && adv->data_RSSI[i] <= 127 && (adv->data_RSSI[i] != 13 && adv->data_RSSI[i] != 10))
            putchar(adv->data_RSSI[i]);
        else
            putchar('.');
    }
    putchar('\n');

    printf(" RSSI: %d\n", adv->data_RSSI[adv->data_length]);
    puts("======");
    putchar('\n');
#endif
}

isrpipe_t isrpipe;
uint8_t isrpipe_buf[128];

uint8_t* uart_read_line(uint8_t* buf, size_t buf_size) {
    size_t read_bytes = 0;

    while (read_bytes < buf_size - 1) {
        uint8_t x;
        if (isrpipe_read(&isrpipe, &x, 1) != 1) {
            return NULL;
        }

        buf[read_bytes++] = x;

        if (buf[read_bytes-1] == '\n') {
            break;
        }
    }

    buf[read_bytes] = '\0';
    return buf;
}

void uart_rx_cb(void *arg, uint8_t data) {
    (void) arg;
    isrpipe_write(&isrpipe, &data, 1);
}

char gps_thread_stack[THREAD_STACKSIZE_MAIN];

void* gps_reader_thread(void* arg) {
    (void)arg;
    puts("[+] Starting GPS parser");

    isrpipe_init(&isrpipe, isrpipe_buf, sizeof(isrpipe_buf));

    uart_init(UART_DEV(1), 9600, uart_rx_cb, NULL);
    gpio_init(GPIO_PIN(PORT_A, 9), GPIO_IN); // PA9 is in conflict with the EEPROM CS on bluetooth shield, so we disable the uart tx pin

    puts("[+] GPS init OK");

    char line[MINMEA_MAX_LENGTH];
    while(1) {
        if(uart_read_line((uint8_t*) line, sizeof(line)) != NULL) {
            int res = minmea_sentence_id(line, false);
            if (res == MINMEA_SENTENCE_RMC) {
                struct minmea_sentence_rmc frame;
                if (minmea_parse_rmc(&frame, line)) {
                    float lat = minmea_tocoord(&frame.latitude);
                    float lon = minmea_tocoord(&frame.longitude);
                    struct timespec ts;

                    if(!minmea_gettime(&ts, &frame.date, &frame.time) && !isnan(lat) && !isnan(lon)) {
                        int timestamp = ts.tv_sec;
                        printf("Updating GPS data, lat = %f, lon = %f, timestamp = %d\n", lat, lon, timestamp);
                        gps_state.lat = lat;
                        gps_state.lon = lon;
                        gps_state.timestamp = timestamp;
                    }
                }
            }
        }
    }

    return NULL;
}

int main() {
    thread_create(gps_thread_stack, sizeof(gps_thread_stack),
        THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
        gps_reader_thread, NULL, "gps_reader_thread");

    puts("[+] Starting LoRa connection");

    // Connection initialization
    int8_t res = lora_initialization();
    if (res != 0) {
        printf("[-] LoRa initialization failed with **Error %d**\r\n", res);
    } else {
        puts("[+] LoRa initialization completed successfully\r\n");

        thread_create(lora_thread_stack, sizeof(lora_thread_stack),
            THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
            lora_thread, NULL, "lora_thread");
    }

    puts("[+] Starting BLE");
    if (BTLE.begin()) {
        puts("Bluetooth module configuration error!");
        while (1);
    }
    puts("[+] BLE started");

    ObserverService.setAdvertisingCallback(advertising_report_cb);

    if (ObserverService.begin(SERVER_BDADDR)) {
        puts("[-] Observer init failed");
        while (1);
    }

    puts("[+] Initialized");
    while (1) {
        // Update the BLE module state
        BTLE.update();
    }
}
