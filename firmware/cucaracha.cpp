#include <stdio.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/irq.h"

#define ADC_GPIO 26
#define ADC_CHANNEL 0

#define SAMPLE_RATE 10000
#define BUFFER_SIZE 1000

uint8_t buffer_a[BUFFER_SIZE];
uint8_t buffer_b[BUFFER_SIZE];

volatile bool buffer_a_ready = false;
volatile bool buffer_b_ready = false;

volatile uint active_buffer = 0;

uint dma_channel;

void dma_handler()
{
    dma_hw->ints0 = 1u << dma_channel;

    if (active_buffer == 0)
    {
        buffer_a_ready = true;
        active_buffer = 1;

        dma_channel_set_write_addr(
            dma_channel,
            buffer_b,
            false
        );
    }
    else
    {
        buffer_b_ready = true;
        active_buffer = 0;

        dma_channel_set_write_addr(
            dma_channel,
            buffer_a,
            false
        );
    }

    dma_channel_set_transfer_count(
        dma_channel,
        BUFFER_SIZE,
        true
    );
}

void send_buffer(uint8_t *buffer)
{
    putchar_raw(0xAA);
    putchar_raw(0x55);

    for (uint32_t i = 0; i < BUFFER_SIZE; i++)
    {
        putchar_raw(buffer[i]);
    }
}

int main()
{
    stdio_init_all();

    sleep_ms(2000);
    adc_gpio_init(ADC_GPIO);
    
    adc_init();
    adc_select_input(ADC_CHANNEL);

    adc_fifo_setup(
        true,
        true,
        1,
        false,
        true
    );

    float adc_divider =
        (48000000.0f / SAMPLE_RATE) - 1.0f;

    adc_set_clkdiv(adc_divider);

    dma_channel =
        dma_claim_unused_channel(true);

    dma_channel_config config =
        dma_channel_get_default_config(dma_channel);

    channel_config_set_transfer_data_size(
        &config,
        DMA_SIZE_8
    );

    channel_config_set_read_increment(
        &config,
        false
    );

    channel_config_set_write_increment(
        &config,
        true
    );

    channel_config_set_dreq(
        &config,
        DREQ_ADC
    );

    dma_channel_configure(
        dma_channel,
        &config,
        buffer_a,
        &adc_hw->fifo,
        BUFFER_SIZE,
        false
    );

    dma_channel_set_irq0_enabled(
        dma_channel,
        true
    );

    irq_set_exclusive_handler(
        DMA_IRQ_0,
        dma_handler
    );

    irq_set_enabled(
        DMA_IRQ_0,
        true
    );

    printf("ADC DMA iniciado\n");
    printf("GPIO: %d\n", ADC_GPIO);
    printf("Sample rate: %d S/s\n", SAMPLE_RATE);
    printf("Resolution: 8 bits\n");
    printf("Buffer: %d samples\n", BUFFER_SIZE);

    adc_run(true);

    dma_channel_set_transfer_count(
        dma_channel,
        BUFFER_SIZE,
        true
    );

    while (true)
    {
        if (buffer_a_ready)
        {
            send_buffer(buffer_a);
            buffer_a_ready = false;
        }

        if (buffer_b_ready)
        {
            send_buffer(buffer_b);
            buffer_b_ready = false;
        }

        tight_loop_contents();
    }

    return 0;
}