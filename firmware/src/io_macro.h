#pragma once

/* allow more flexible IO initializing */

#define SPEED_10MHz     0x01
#define SPEED_2MHz      0x02
#define SPEED_50MHz     0x03

/* compiler will calculate this hell staticaly */
#define EX_GPIO_PIN_MODE( PORT, PIN, MODE, SPEED ) do{\
((uint32_t*)GPIO##PORT##_BASE)[PIN>>3u] &= ~(0x0Fu<<((PIN%8u)<<2u));\
((uint32_t*)GPIO##PORT##_BASE)[PIN>>3u] |= (MODE|SPEED)<<((PIN%8u)<<2u);\
}while(0)


#define EX_GPIO_INPUT_PD( PORT, PIN, ... )              do{ EX_GPIO_PIN_MODE( PORT, PIN, 0x08u, 0 );  GPIO##PORT->BRR = (1u<<PIN); }while(0)
#define EX_GPIO_INPUT_PU( PORT, PIN, ... )              do{ EX_GPIO_PIN_MODE( PORT, PIN, 0x08u, 0 );  GPIO##PORT->BSRR = (1u<<PIN); }while(0)
#define EX_GPIO_INPUT_FLOATING( PORT, PIN, ... )        EX_GPIO_PIN_MODE( PORT, PIN, 0x04u, 0 )
#define EX_GPIO_INPUT_ANALOG( PORT, PIN, ... )          EX_GPIO_PIN_MODE( PORT, PIN, 0x00u, 0 )

#define EX_GPIO_OUT_PP( PORT, PIN, SPEED )              EX_GPIO_PIN_MODE( PORT, PIN, 0x00u, SPEED )
#define EX_GPIO_OUT_OD( PORT, PIN, SPEED )              EX_GPIO_PIN_MODE( PORT, PIN, 0x04u, SPEED )
#define EX_GPIO_OUT_ALT_PP PORT, PIN, SPEED )           EX_GPIO_PIN_MODE( PORT, PIN, 0x08u, SPEED )
#define EX_GPIO_OUT_ALT_OD( PORT, PIN, SPEED )          EX_GPIO_PIN_MODE( PORT, PIN, 0x0Cu, SPEED )

#define EX_GPIO_PIN_STAT( PORT, PIN, ... )      ( (GPIO##PORT->IDR & (1u<<PIN)) == (1u<<PIN) )
#define EX_GPIO_PIN_HI( PORT, PIN, ... )        GPIO##PORT->BSRR = (1u<<PIN)
#define EX_GPIO_PIN_LOW( PORT, PIN, ... )       GPIO##PORT->BRR = (1u<<PIN)
#define EX_GPIO_ENABLE_CLOCK( PORT, PIN, ... )  RCC->APB2ENR |= RCC_APB2ENR_IOP##PORT##EN

#define EX_GPIO_ENABLE_CLOCK_ENTRY( PORT, PIN, ...  )  | RCC_APB2ENR_IOP##PORT##EN

/* arrays to optimize for many pins */
#define PORT_ENABLE_CLOCK_START() RCC->APB2ENR = 0
#define PORT_ENABLE_CLOCK_ENTRY( CONFIG ) EX_GPIO_ENABLE_CLOCK_ENTRY( CONFIG )  
#define PORT_ENABLE_CLOCK_END() ;

/* user macro */
#define PORT_ENABLE_CLOCK( CONFIG ) EX_GPIO_ENABLE_CLOCK( CONFIG )  

#define PIN_STAT( CONFIG ) EX_GPIO_PIN_STAT( CONFIG )
#define PIN_HI( CONFIG ) EX_GPIO_PIN_HI( CONFIG )
#define PIN_LOW( CONFIG ) EX_GPIO_PIN_LOW( CONFIG ) 

#define PIN_INPUT_PD( CONFIG )          EX_GPIO_INPUT_PD( CONFIG )
#define PIN_INPUT_PU( CONFIG )          EX_GPIO_INPUT_PU( CONFIG )
#define PIN_INPUT_FLOATING( CONFIG )    EX_GPIO_INPUT_FLOATING( CONFIG )
#define PIN_INPUT_ANALOG( CONFIG )      EX_GPIO_INPUT_ANALOG( CONFIG )

#define PIN_OUT_PP( CONFIG )            EX_GPIO_OUT_PP( CONFIG )
#define PIN_OUT_OD( CONFIG )            EX_GPIO_OUT_OD( CONFIG )
#define PIN_OUT_ALT_PP CONFIG )         EX_GPIO_OUT_ALT_PP CONFIG )
#define PIN_OUT_ALT_OD( CONFIG )        EX_GPIO_OUT_ALT_OD( CONFIG )  
