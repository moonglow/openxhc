#pragma once

void spi_init( int dev );
void spi_init_ex( int dev, uint32_t hz );
uint8_t spi1_send_byte( uint8_t data );
uint8_t spi2_send_byte( uint8_t data );
