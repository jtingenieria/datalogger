#include <stdio.h>
#include "serial_logger.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"

static const char *TAG = "serial_logger";

#define EX_UART_NUM UART_NUM_1
#define PATTERN_CHR_NUM    (1)         /*!< Set the number of consecutive and identical characters received by receiver which defines a UART pattern*/

#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)

#define GPIO_UART_1 16

static int number_of_float_vars = 5;
static int serial_packet_size = 0;

static QueueHandle_t uart0_queue;

static QueueHandle_t uart_packet_q;

RTC_DATA_ATTR int serial_packet_id = 0;

void serial_logger_get_data(TickType_t xTicksToWait, serial_packet_t ** packet)
{
    *packet = malloc(serial_packet_size);
    ESP_LOGI(TAG, "Dir de packet: %p. Apunta a %p",(void *) packet, (void *) *packet);
    //vTaskDelay(pdMS_TO_TICKS(100));
	if(xQueueReceive(uart_packet_q, *packet, xTicksToWait) != pdTRUE)
	{
	    ESP_LOGI(TAG, "No data");
		(*packet)->id = -1;
		(*packet)->float_data_qty = number_of_float_vars;
	}
}

static void uart_event_task(void *pvParameters)
{
    uart_event_t event;
    size_t buffered_size;
    uint8_t* dtmp = (uint8_t*) malloc(RD_BUF_SIZE);
    for(;;) {
        //Waiting for UART event.
        if(xQueueReceive(uart0_queue, (void * )&event, portMAX_DELAY))
        {
            bzero(dtmp, RD_BUF_SIZE);
            //ESP_LOGI(TAG, "uart[%d] event:", EX_UART_NUM);
            switch(event.type) {
                //Event of UART receving data
                /*We'd better handler data event fast, there would be much more data events than
                other types of events. If we take too much time on data event, the queue might
                be full.*/
                case UART_DATA:
                    //ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
                   // uart_read_bytes(EX_UART_NUM, dtmp, event.size, portMAX_DELAY);
                    //ESP_LOGI(TAG, "[DATA EVT]:");
                   // uart_write_bytes(EX_UART_NUM, (const char*) dtmp, event.size);
                    break;
                //Event of HW FIFO overflow detected
                case UART_FIFO_OVF:
                    //ESP_LOGI(TAG, "hw fifo overflow");
                    // If fifo overflow happened, you should consider adding flow control for your application.
                    // The ISR has already reset the rx FIFO,
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(EX_UART_NUM);
                    xQueueReset(uart0_queue);
                    break;
                //Event of UART ring buffer full
                case UART_BUFFER_FULL:
                   // ESP_LOGI(TAG, "ring buffer full");
                    // If buffer full happened, you should consider encreasing your buffer size
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(EX_UART_NUM);
                    xQueueReset(uart0_queue);
                    break;
                //Event of UART RX break detected
                case UART_BREAK:
                    //ESP_LOGI(TAG, "uart rx break");
                    break;
                //Event of UART parity check error
                case UART_PARITY_ERR:
                   // ESP_LOGI(TAG, "uart parity error");
                    break;
                //Event of UART frame error
                case UART_FRAME_ERR:
                   // ESP_LOGI(TAG, "uart frame error");
                    break;
                //UART_PATTERN_DET
                case UART_PATTERN_DET:
                    uart_get_buffered_data_len(EX_UART_NUM, &buffered_size);
                    int pos = uart_pattern_pop_pos(EX_UART_NUM);
                   // ESP_LOGI(TAG, "[UART PATTERN DETECTED] pos: %d, buffered size: %d", pos, buffered_size);
                    if (pos == -1) {
                        // There used to be a UART_PATTERN_DET event, but the pattern position queue is full so that it can not
                        // record the position. We should set a larger queue size.
                        // As an example, we directly flush the rx buffer here.
                        uart_flush_input(EX_UART_NUM);
                    } else {

                    	serial_packet_t * packet = malloc(serial_packet_size);
                    	packet->size_of_packet = serial_packet_size;

                        uart_read_bytes(EX_UART_NUM, dtmp, pos, 100 / portTICK_PERIOD_MS);
                        uint8_t pat[PATTERN_CHR_NUM + 1];
                        memset(pat, 0, sizeof(pat));
                        uart_read_bytes(EX_UART_NUM, pat, PATTERN_CHR_NUM, 100 / portTICK_PERIOD_MS);

                        char *token = strtok((char *)dtmp, ",");
                        int j = 0;
                        for(j = 0; j < number_of_float_vars; j++)
                        {
                            if (token != NULL)
                            {
                                packet->float_data_p[j] = strtod(token, NULL);
                                token = strtok(NULL, ",");
                            }
                            else
                            {
                                break;
                            }
                        }
                        packet->float_data_qty = number_of_float_vars;
                        packet->received_data_qty = j;
                        packet->id = serial_packet_id;
                        serial_packet_id ++;
                        xQueueGenericSend( uart_packet_q, (void *)packet, 0, queueSEND_TO_FRONT );
                        //ESP_LOGI(TAG, "read data: %s", dtmp);
                        //ESP_LOGI(TAG, "read pat : %s", pat);
                        free(packet);
                    }
                    break;
                //Others
                default:
                    //ESP_LOGI(TAG, "uart event type: %d", event.type);
                    break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}


int serial_logger_init(int float_data_expected)
{
    number_of_float_vars = float_data_expected;

    serial_packet_size = sizeof (serial_packet_t) + sizeof(float) * number_of_float_vars;
    //serial_packet_t * packet = malloc();

	uart_packet_q = xQueueCreate(10, sizeof(serial_packet_size));

	 /* Configure parameters of an UART driver,
	     * communication pins and install the driver */
	    uart_config_t uart_config = {
	        .baud_rate = 9600,
	        .data_bits = UART_DATA_8_BITS,
	        .parity = UART_PARITY_DISABLE,
	        .stop_bits = UART_STOP_BITS_1,
	        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
	    };
	    uart_param_config(EX_UART_NUM, &uart_config);

	    //Set UART log level
	    esp_log_level_set(TAG, ESP_LOG_INFO);
	    //Set UART pins (using UART0 default pins ie no changes.)
	    uart_set_pin(EX_UART_NUM, UART_PIN_NO_CHANGE, GPIO_UART_1, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	    //Install UART driver, and get the queue.
	    uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &uart0_queue, 0);

	    //Set uart pattern detect function.
	    uart_enable_pattern_det_baud_intr(UART_NUM_1, '\n', 1, 9, 0, 0); // pattern is LF
	    //Reset the pattern queue length to record at most 20 pattern positions.
	    uart_pattern_queue_reset(EX_UART_NUM, 20);

	    //Create a task to handler UART event from ISR
	    xTaskCreate(uart_event_task, "uart_event_task", 2048, NULL, 12, NULL);

	    return 0;

}
