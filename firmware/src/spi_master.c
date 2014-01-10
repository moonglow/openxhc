#include "stm32f10x.h"
#include "spi_master.h"


static uint8_t spi_master_send_byte( SPI_TypeDef *dev, uint8_t data );

void spi_init( int dev )
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;
    SPI_TypeDef *SPI = 0;
    
    if( 1 == dev )
    {
      RCC_APB2PeriphClockCmd( RCC_APB2Periph_SPI1, ENABLE );
      SPI = SPI1;
      
      RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE);
      /* MOSI & CLOCK */
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_5;
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
      GPIO_Init( GPIOA, &GPIO_InitStructure );
      
      /* MISO */
      GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6;
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
      GPIO_Init( GPIOA, &GPIO_InitStructure );
    }
    else if( 2 == dev )
    {
      RCC_APB1PeriphClockCmd( RCC_APB1Periph_SPI2, ENABLE );
      SPI = SPI2;
      
      RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE);
      /* MOSI & CLOCK */
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15 | GPIO_Pin_13;
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
      GPIO_Init( GPIOB, &GPIO_InitStructure );
      
      /* MISO */
      GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_14;
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
      GPIO_Init( GPIOB, &GPIO_InitStructure );
    }
    
    SPI_I2S_DeInit( SPI );
    
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    /* good for long wire */
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
  
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init( SPI, &SPI_InitStructure );
  
    SPI_Cmd( SPI, ENABLE);
}

uint8_t spi1_send_byte( uint8_t data )
{
  return spi_master_send_byte( SPI1, data );
}

uint8_t spi2_send_byte( uint8_t data )
{
  return spi_master_send_byte( SPI2, data );
}


static uint8_t spi_master_send_byte( SPI_TypeDef *dev, uint8_t data )
{
  uint32_t timeout;

  timeout = 100000;
  while (SPI_I2S_GetFlagStatus( dev, SPI_I2S_FLAG_TXE) == RESET)
  {
    if( --timeout == 0 )
      break;
  }

  SPI_I2S_SendData( dev, data );

  timeout = 100000;
  while (SPI_I2S_GetFlagStatus( dev, SPI_I2S_FLAG_RXNE) == RESET)
  {
    if( --timeout == 0 )
      break;
  }
  
  return SPI_I2S_ReceiveData( dev );  
}