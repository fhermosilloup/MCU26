#ifndef __STM32F103_HAL_H
#define __STM32F103_HAL_H

#include "stm32f103xb.h"
#include <stdbool.h>

/*
 * Configuración de los relojes (no tocar)
 */
#define HSI_VALUE    ((uint32_t)8000000)    /* Oscilador interno high-speed a 8 MHz */
#define HSE_VALUE    ((uint32_t)8000000)    /* Oscilador externo high-speed a 8 MHz en bluepill */
#define LSI_VALUE    ((uint32_t)40000)      /* Oscilador interno low-speed a ~40 kHz */
#define LSE_VALUE    ((uint32_t)32768)      /* Oscilador externo low-speed a 32.768 kHz */



/*
 **************************************************************
 * NESTED VECTORED INTERRUPT CONTROLLER (NVIC) PERIPHERAL
 **************************************************************
 */
void nvic_enable_irq(IRQn_Type IRQn);

/**
  \brief   Enable IRQ Interrupts
  \details Enables IRQ interrupts by clearing the I-bit in the CPSR.
           Can only be executed in Privileged modes.
 */
void nvic_enable(void);

/**
  \brief   Disable IRQ Interrupts
  \details Disables IRQ interrupts by setting the I-bit in the CPSR.
           Can only be executed in Privileged modes.
 */
void nvic_disable(void);

/*
 **************************************************************
 * RESET AND CLOCK CONTROL PERIPHERAL
 **************************************************************
 */

typedef enum {
	RCC_SYSCLK_HSI=0,
	RCC_SYSCLK_HSE,
	RCC_SYSCLK_PLL_HSI_DIV_2,
	RCC_SYSCLK_PLL_HSE_DIV_1,
	RCC_SYSCLK_PLL_HSE_DIV_2
} rcc_sysclk_src_t;

typedef enum {
	RCC_PLL_X2=0,
	RCC_PLL_X3,
	RCC_PLL_X4,
	RCC_PLL_X5,
	RCC_PLL_X6,
	RCC_PLL_X7,
	RCC_PLL_X8,
	RCC_PLL_X9,
	RCC_PLL_X10,
	RCC_PLL_X11,
	RCC_PLL_X12,
	RCC_PLL_X13,
	RCC_PLL_X14,
	RCC_PLL_X15,
	RCC_PLL_X16
} rcc_pll_mul_t;

typedef enum {
	RCC_HSI=0,
	RCC_HSE=1
} rcc_hsclk_t;

typedef enum {
	RCC_LSI=0,
	RCC_LSE=1
} rcc_lsclk_t;

typedef enum {
	RCC_BUS_AHB  = 0,
	RCC_BUS_APB1 = 1,
	RCC_BUS_APB2 = 2
} rcc_bus_t;

typedef enum {
	RCC_DIV_1=0,
	RCC_DIV_2=1,
	RCC_DIV_4=2,
	RCC_DIV_8=3,
	RCC_DIV_16=4,
	RCC_DIV_64=5,
	RCC_DIV_128=6,
	RCC_DIV_256=7,
	RCC_DIV_512=8
} rcc_div_t;

typedef enum {
	RCC_USB_DIV_1_5 	= 0,
	RCC_USB_DIV_1	= 1,
} rcc_usbclk_div_t;

typedef enum {
	RCC_ADC_DIV_2 = 0,
	RCC_ADC_DIV_4 = 1,
	RCC_ADC_DIV_6 = 2,
	RCC_ADC_DIV_8 = 3
} rcc_adcclk_div_t;

typedef enum {
	/* APB1 */
	RCC_TIM2=0,
	RCC_TIM3=1,
	RCC_TIM4=2,
	RCC_TIM5=3,
	RCC_TIM6=4,
	RCC_TIM7=5,
	RCC_TIM12=6,
	RCC_TIM13=7,
	RCC_TIM14=8,
	RCC_WWDG=11,
	RCC_SPI2=14,
	RCC_SPI3=15,
	RCC_USART2=17,
	RCC_USART3=18,
	RCC_USART4=19,
	RCC_USART5=20,
	RCC_I2C1=21,
	RCC_I2C2=22,
	RCC_USB=23,
	RCC_CAN=25,
	RCC_BKP=27,
	RCC_PWR=28,
	RCC_DAC=29,
	/* APB2 */
	RCC_AFIO=32,
	RCC_GPIOA=34,
	RCC_GPIOB=35,
	RCC_GPIOC=36,
	RCC_GPIOD=37,
	RCC_GPIOE=38,
	RCC_GPIOF=39,
	RCC_GPIOG=40,
	RCC_ADC1=41,
	RCC_ADC2=42,
	RCC_TIM1=43,
	RCC_SPI1=44,
	RCC_TIM8=45,
	RCC_USART1=46,
	RCC_ADC3=47,
	RCC_TIM9=51,
	RCC_TIM10=52,
	RCC_TIM11=53,
	/* AHB */
	RCC_DMA1=64,
	RCC_DMA2=65,
	RCC_SRAM=66,
	RCC_FLITF=68,
	RCC_CRC=70,
	RCC_FSMC=72,
	RCC_SDIO=74
} rcc_periph_clk_t;

#define RCC_EXTI	RCC_AFIO

/**
 * @brief Habilita el reloj de un periférico del microcontrolador.
 *
 * Esta función habilita el clock del periférico especificado dentro del
 * módulo RCC. Recuerde que un periférico debe tener su reloj
 * habilitado antes de poder ser utilizado.
 *
 * Dependiendo del periférico seleccionado, el bit correspondiente será
 * habilitado en uno de los registros:
 * - RCC->APB1ENR
 * - RCC->APB2ENR
 * - RCC->AHBENR
 *
 * @param clk
 * Identificador del periférico cuyo reloj será habilitado. El valor debe
 * pertenecer al enum @ref rcc_periph_clk_t, el cual incluye periféricos
 * conectados a los buses AHB, APB1 y APB2.
 *
 * @note
 * Si el reloj no es habilitado previamente, el periférico permanecerá
 * deshabilitado y cualquier acceso a sus registros no tendrá efecto.
 */
void rcc_clock_enable(rcc_periph_clk_t clk);

/**
 * @brief Configura la fuente del reloj principal del sistema (SYSCLK).
 *
 * Esta función selecciona la fuente de reloj que alimentará al sistema
 * completo del microcontrolador (SYSCLK). Opcionalmente puede utilizarse
 * el PLL para multiplicar la frecuencia de entrada.
 *
 * Las posibles fuentes de SYSCLK están definidas en el enum
 * @ref rcc_sysclk_src_t:
 *
 * - HSI (oscilador interno de alta velocidad, 8 MHz)
 * - HSE (oscilador externo)
 * - PLL con HSI/2 como entrada
 * - PLL con HSE como entrada
 * - PLL con HSE/2 como entrada
 *
 * Si se selecciona una fuente basada en PLL, el multiplicador del PLL
 * será configurado mediante el parámetro @p pll_mul.
 *
 * @param src
 * Fuente de reloj principal del sistema definida en @ref rcc_sysclk_src_t.
 *
 * @param pll_mul
 * Factor multiplicador del PLL definido en @ref rcc_pll_mul_t.
 * Este parámetro solo tiene efecto cuando la fuente SYSCLK utiliza el PLL.
 *
 * @note
 * El reloj del sistema afecta directamente a los buses:
 * - AHB
 * - APB1
 * - APB2
 *
 * Por lo tanto, los divisores de estos buses deben configurarse
 * adecuadamente si la frecuencia de SYSCLK es alta con las funciones
 * - @ref rcc_set_bus_clk_div
 * - @ref rcc_set_adc_clk_div
 * - @ref rcc_set_usb_clk_div
 */
void rcc_set_clock(rcc_sysclk_src_t src, rcc_pll_mul_t pll_mul);

/**
 * @brief Configura el divisor de reloj de uno de los buses del sistema.
 *
 * Esta función permite dividir la frecuencia del reloj que alimenta
 * uno de los buses principales del microcontrolador:
 *
 * - AHB  → bus principal del sistema
 * - APB1 → periféricos de baja velocidad
 * - APB2 → periféricos de alta velocidad
 *
 * El divisor seleccionado se aplica sobre el reloj de entrada
 * proveniente del SYSCLK.
 *
 * @param bus
 * Bus al que se desea aplicar el divisor. Definido en @ref rcc_bus_t.
 *
 * @param clkdiv
 * Factor de división del reloj definido en @ref rcc_div_t.
 *
 * @note
 * En STM32F103 existen restricciones de frecuencia:
 * - APB1 ≤ 36 MHz
 * - APB2 ≤ 72 MHz
 *
 * Por lo tanto, si el SYSCLK es alto, es necesario dividir APB1.
 */
void rcc_set_bus_clk_div(rcc_bus_t bus, rcc_div_t clkdiv);

/**
 * @brief Configura el divisor de reloj de uno de los buses del sistema.
 *
 * Esta función permite dividir la frecuencia del reloj que alimenta
 * uno de los buses principales del microcontrolador:
 *
 * - AHB  → bus principal del sistema
 * - APB1 → periféricos de baja velocidad
 * - APB2 → periféricos de alta velocidad
 *
 * El divisor seleccionado se aplica sobre el reloj de entrada
 * proveniente del SYSCLK.
 *
 * @param bus
 * Bus al que se desea aplicar el divisor. Definido en @ref rcc_bus_t.
 *
 * @param clkdiv
 * Factor de división del reloj definido en @ref rcc_div_t.
 *
 * @note
 * En STM32F103 existen restricciones de frecuencia:
 * - APB1 ≤ 36 MHz
 * - APB2 ≤ 72 MHz
 *
 * Por lo tanto, si el SYSCLK es alto, es necesario dividir APB1.
 */
void rcc_set_adc_clk_div(rcc_adcclk_div_t clkdiv);

/**
 * @brief Configura el divisor del reloj del periférico USB.
 *
 * El módulo USB requiere una frecuencia específica de 48 MHz para
 * funcionar correctamente. Esta función configura el divisor aplicado
 * al reloj PLL para generar el clock del USB.
 *
 * @param clkdiv
 * Factor de división del reloj USB definido en @ref rcc_usbclk_div_t.
 *
 * Opciones disponibles:
 * - PLL / 1
 * - PLL / 1.5
 *
 * @note
 * Para operación correcta del USB, el resultado final debe ser 48 MHz.
 */
void rcc_set_usb_clk_div(rcc_usbclk_div_t clkdiv);

/**
 * @brief Obtiene la frecuencia actual del reloj del sistema (SYSCLK).
 *
 * Esta función devuelve la frecuencia actual del reloj principal
 * del sistema después de aplicar las configuraciones del RCC.
 *
 * @return
 * Frecuencia del SYSCLK en Hz.
 *
 * @note
 * Esta frecuencia es utilizada como base para derivar los relojes
 * de los buses AHB, APB1 y APB2.
 */
uint32_t rcc_get_sysclk(void);

/**
 * @brief Obtiene la frecuencia actual de un bus del sistema.
 *
 * Esta función calcula y devuelve la frecuencia efectiva del bus
 * seleccionado considerando el SYSCLK y los divisores configurados
 * en el RCC.
 *
 * @param bus
 * Bus cuya frecuencia se desea obtener, definido en @ref rcc_bus_t.
 *
 * @return
 * Frecuencia del bus en Hz.
 *
 * Ejemplo:
 * - AHB  → SystemCoreClock / ahb_divisor
 * - APB1 → SystemCoreClock / apb1_divisor
 * - APB2 → SystemCoreClock / apb2_divisor
 */
uint32_t rcc_get_bus_clock(rcc_bus_t bus);

/**
 * @brief Obtiene la frecuencia actual del reloj del ADC.
 *
 * Calcula la frecuencia efectiva del clock que alimenta al
 * periférico ADC considerando:
 *
 * - Frecuencia del bus APB2
 * - Divisor configurado en RCC_CFGR
 *
 * @return
 * Frecuencia del reloj del ADC en Hz.
 */
uint32_t rcc_get_adc_clock(void);

/**
 * @brief Obtiene la frecuencia actual del reloj del USB.
 *
 * Calcula la frecuencia del reloj que alimenta al periférico USB
 * a partir del PLL y el divisor configurado en RCC.
 *
 * @return
 * Frecuencia del reloj USB en Hz.
 *
 * @note
 * Para funcionamiento correcto del USB esta frecuencia debe ser
 * exactamente 48 MHz.
 */
uint32_t rcc_get_usb_clock(void);









/*
 **************************************************************
 * GENERAL PURPOSE INPUT/OUTPUT PERIPHERAL
 **************************************************************
 */
#define GPIO_PIN_0		(0)
#define GPIO_PIN_1		(1)
#define GPIO_PIN_2		(2)
#define GPIO_PIN_3		(3)
#define GPIO_PIN_4		(4)
#define GPIO_PIN_5		(5)
#define GPIO_PIN_6		(6)
#define GPIO_PIN_7		(7)
#define GPIO_PIN_8		(8)
#define GPIO_PIN_9		(9)
#define GPIO_PIN_10		(10)
#define GPIO_PIN_11		(11)
#define GPIO_PIN_12		(12)
#define GPIO_PIN_13		(13)
#define GPIO_PIN_14		(14)
#define GPIO_PIN_15		(15)
#define GPIO_PIN_MAX	(16)

#define GPIO_MODE_INPUT					(0x00)
#define GPIO_MODE_OUTPUT_10MHZ	(0x01)
#define GPIO_MODE_OUTPUT_2MHZ		(0x02)
#define GPIO_MODE_OUTPUT_50MHZ	(0x03)


#define GPIO_CFN_ANALOG	(0x00)
#define GPIO_CFN_FLOAT	(0x01)
#define GPIO_CFN_PUPD	  (0x02)
#define GPIO_CFN_PP		  (0x00)
#define GPIO_CFN_OD		  (0x01)
#define GPIO_CFN_ALT_PP	(0x02)
#define GPIO_CFN_ALT_OD	(0x03)

typedef enum {
	GPIO_INPUT_ANALOG	= 0b00000,
	GPIO_INPUT_FLOAT 	= 0b00100,
	GPIO_INPUT_PD 	 	= 0b01000,
	GPIO_INPUT_PU 		= 0b11000
} gpio_input_t;

typedef enum {
	GPIO_OUTPUT_PP		= 0b0000,
	GPIO_OUTPUT_OD		= 0b0100,
	GPIO_OUTPUT_AF_PP	= 0b1000,
	GPIO_OUTPUT_AF_OD	= 0b1100
} gpio_output_t;

typedef enum {
	GPIO_SPEED_2MHZ		= 0b0010,
	GPIO_SPEED_10MHZ	= 0b0001,
	GPIO_SPEED_50MHZ	= 0b0011
} gpio_speed_t;

/**
 * @brief Deshabilita los pines de depuración del microcontrolador.
 *
 * Esta función libera los pines utilizados por la interfaz de depuración
 * del microcontrolador (JTAG/SWD) para que puedan ser empleados como terminales
 * de entada y salida de proposito general (GPIO).
 *
 * En STM32F103 los pines de depuración incluyen:
 *
 * - PA13 → SWDIO
 * - PA14 → SWCLK
 * - PA15 → JTDI
 * - PB3  → JTDO
 * - PB4  → NJTRST
 *
 * @note
 * Si estos pines se reconfiguran como GPIO normales, se perderá la
 * capacidad de depuración hasta reiniciar el microcontrolador.
 */
void gpio_disable_debug_pins(void);

/**
 * @brief Configura un pin GPIO como entrada.
 *
 * Esta función configura un pin específico de un puerto GPIO para operar
 * como entrada, utilizando el modo indicado en el parámetro @p mode.
 *
 * La configuración afecta los campos MODE y CNF del registro:
 *
 * - GPIOx->CRL  (pines 0–7)
 * - GPIOx->CRH  (pines 8–15)
 *
 * @param GPIOx
 * Puerto GPIO al que pertenece el pin (GPIOA, GPIOB, GPIOC, etc.).
 *
 * @param pin_num
 * Número del pin dentro del puerto (0 a 15).
 *
 * @param mode
 * Modo de entrada definido en el enum @ref gpio_input_t.
 *
 * Ejemplos típicos:
 * - Entrada flotante
 * - Entrada con pull-up
 * - Entrada con pull-down
 * - Entrada analógica
 *
 * @note
 * Si se selecciona un modo pull-up o pull-down, el estado inicial del pin
 * dependerá del valor del bit correspondiente en el registro GPIOx->ODR.
 */
void gpio_set_input(GPIO_TypeDef *GPIOx, uint32_t pin_num, gpio_input_t mode);

/**
 * @brief Configura un pin GPIO como salida.
 *
 * Esta función configura un pin GPIO para operar como salida digital,
 * permitiendo seleccionar el tipo de salida y la velocidad máxima.
 *
 * La configuración se realiza sobre los campos MODE y CNF en los registros
 * CRL o CRH del puerto correspondiente.
 *
 * @param GPIOx
 * Puerto GPIO al que pertenece el pin (GPIOA, GPIOB, GPIOC, etc.).
 *
 * @param pin_num
 * Número del pin dentro del puerto (0 a 15).
 *
 * @param mode
 * Tipo de salida definido en el enum @ref gpio_output_t.
 *
 * Ejemplos:
 * - Push-pull
 * - Open-drain
 * - Alternate function push-pull
 * - Alternate function open-drain
 *
 * @param speed
 * Velocidad máxima de conmutación del pin definida en @ref gpio_speed_t.
 *
 * Opciones típicas:
 * - 2 MHz
 * - 10 MHz
 * - 50 MHz
 *
 * @note
 * La velocidad configurada corresponde a la capacidad máxima de cambio
 * de estado del pin y no a la frecuencia real de operación.
 */
void gpio_set_output(GPIO_TypeDef *GPIOx, uint32_t pin_num, gpio_output_t mode, gpio_speed_t speed);

void gpio_set_mode(GPIO_TypeDef *GPIOx, uint32_t pin_num, uint8_t mode);
void gpio_set_cnf(GPIO_TypeDef *GPIOx, uint32_t pin_num, uint8_t cfn);

void gpio_bitwise_set(GPIO_TypeDef *GPIOx, uint16_t mask);
void gpio_bitwise_clear(GPIO_TypeDef *GPIOx, uint16_t mask);
void gpio_bitwise_toggle(GPIO_TypeDef *GPIOx, uint16_t mask);
uint8_t gpio_read_pin(GPIO_TypeDef *GPIOx, uint8_t pin_num);


/*
 **************************************************************
 * EXTERNAL INTERRUPT CONTROLLER
 **************************************************************
 */

typedef enum {
	PA0=0,
	PA1,
	PA2,
	PA3,
	PA4,
	PA5,
	PA6,
	PA7,
	PA8,
	PA9,
	PA10,
	PA11,
	PA12,
	PA13,
	PA14,
	PA15,
	PB0,
	PB1,
	PB2,
	PB3,
	PB4,
	PB5,
	PB6,
	PB7,
	PB8,
	PB9,
	PB10,
	PB11,
	PB12,
	PB13,
	PB14,
	PB15,
	PC0,
	PC1,
	PC2,
	PC3,
	PC4,
	PC5,
	PC6,
	PC7,
	PC8,
	PC9,
	PC10,
	PC11,
	PC12,
	PC13,
	PC14,
	PC15
} pin_t;

typedef enum {
    EXTI_IRQ_FALLING,  ///< Falling edge: Interrupción por flanco descendente
    EXTI_IRQ_RISING,   ///< Rising edge: Interrupción por flanco ascendente
    EXTI_IRQ_BOTH      ///< Both edges: Interrupción por ambos flancos
} exti_irq_t;

typedef enum {
    IRQ_DISABLE = 0,
    IRQ_ENABLE  = 1
} irq_state_t;

/**
 * @brief Configura la interrupción externa (EXTI) asociada a un pin GPIO.
 *
 * Esta función habilita o deshabilita la generación de interrupciones externas
 * a partir de un pin GPIO específico. Internamente configura el controlador
 * EXTI del STM32, permitiendo que un cambio de estado en el pin genere una
 * interrupción hacia la NVIC.
 *
 * El pin seleccionado se mapea a la línea EXTI correspondiente mediante
 * el registro AFIO_EXTICR. Cada pin GPIO puede generar una interrupción
 * externa si está asociado a una línea EXTI válida.
 *
 * La interrupción puede configurarse para activarse en:
 *
 * - Flanco de subida (rising edge)
 * - Flanco de bajada (falling edge)
 * - Ambos flancos
 *
 * Cuando ocurre el evento configurado, se genera la interrupción
 * ejecutando el contenido de la función asociada de interrupción:
 *
 * - void EXTI0_IRQHandler(void): EXTI0
 * - EXTI1_IRQHandler(void): EXTI1
 * - EXTI2_IRQHandler(void): EXTI2
 * - EXTI3_IRQHandler(void): EXTI3
 * - EXTI4_IRQHandler(void): EXTI4
 * - EXTI9_5_IRQHandler(void): EXTI5 - EXTI9
 * - EXTI15_10_IRQHandler(void): EXTI10 - EXTI15
 *
 * @param pin
 * Pin GPIO que se utilizará como fuente de interrupción externa, definido en el enum
 * @ref pin_t.
 *
 * @param irq
 * Tipo de evento que generará la interrupción, definido en el enum
 * @ref exti_irq_t.
 *
 * Ejemplos:
 * - Interrupción por flanco de subida
 * - Interrupción por flanco de bajada
 * - Interrupción por ambos flancos
 *
 * @note
 * Antes de habilitar una interrupción EXTI es comunmente para pines configurados
 * como entrada mediante @ref gpio_set_input().
 *
 * @warning
 * No olvidar habilitar también la interrupción correspondiente en la NVIC
 * para que el microcontrolador pueda atender el evento generado por EXTI,
 * mediante NVIC_IRQEnable(#IRQ)
 *
 * @see gpio_set_input
 */
void exti_set_interrupt(pin_t pin, exti_irq_t irq);


/*
 **************************************************************
 * DIRECT MEMORY ACCESS (DMA)
 **************************************************************
 */
typedef enum {
	DMA_DIR_PERIPH_TO_MEM = 0,
	DMA_DIR_MEM_TO_PERIPH,
	DMA_DIR_MEM_TO_MEM
} dma_dir_t;

typedef enum {
	DMA_SIZE_8BIT = 0,
	DMA_SIZE_16BIT,
	DMA_SIZE_32BIT
} dma_size_t;

typedef enum {
	DMA_MODE_NORMAL = 0,
	DMA_MODE_CIRCULAR
} dma_mode_t;

typedef enum {
	DMA_PRIORITY_LOW = 0,
	DMA_PRIORITY_MEDIUM,
	DMA_PRIORITY_HIGH,
	DMA_PRIORITY_VERY_HIGH
} dma_priority_t;

typedef enum {
	DMA_IRQ_TC = 0,
	DMA_IRQ_HT,
	DMA_IRQ_TE
} dma_irq_t;



void dma_init(
	DMA_Channel_TypeDef *ch,
	dma_dir_t dir,
	dma_size_t mem_size,
	dma_size_t periph_size,
	bool mem_inc,
	bool periph_inc,
	dma_mode_t mode,
	dma_priority_t priority
);

void dma_set_addresses(
	DMA_Channel_TypeDef *ch,
	volatile void *periph_addr,
	void *mem_addr,
	uint16_t length
);

void dma_set_interrupt(
	DMA_Channel_TypeDef *ch,
	dma_irq_t irq,
	irq_state_t state
);

void dma_start(DMA_Channel_TypeDef *ch);
void dma_stop(DMA_Channel_TypeDef *ch);



/*
 **************************************************************
 * SYSTEM CLOCK TIMER
 **************************************************************
 */
typedef enum {
	SYSTICK_CLKSRC_AHB_DIV_8 = 0,
	SYSTICK_CLKSRC_AHB = 1
} systick_clksrc_t;

void systick_config(uint32_t period, systick_clksrc_t clksrc);







/*
 **************************************************************
 * TIMER PERIPHERAL
 **************************************************************
 */
typedef enum {
    TIMER_MODE_UP,       ///< Counter increments from 0 to ARR ( upcounting mode)
    TIMER_MODE_DOWN,     ///< Counter decrements from ARR to 0 (downcounting mode)
    TIMER_MODE_CENTER    ///< Center-aligned mode (counter counts up then down)
} timer_mode_t;

typedef enum {
    TIMER_CONTINUOUS = 0,	///< Timer runs continuously (auto-restart after update event)
    TIMER_ONESHOT        	///< Timer stops automatically after first update event
} timer_run_mode_t;

typedef enum{
    TIMER_CHANNEL_1 = 0, ///< Timer channel 1
    TIMER_CHANNEL_2 = 1, ///< Timer channel 2
    TIMER_CHANNEL_3 = 2, ///< Timer channel 3
    TIMER_CHANNEL_4 = 3  ///< Timer channel 4
} timer_channel_t;

typedef enum {
    TIMER_CLK_INTERNAL = 0,   ///< Internal clock from APB bus (CK_INT)
    TIMER_CLK_EXTERNAL_TI1,   ///< External clock from TI1 input pin
    TIMER_CLK_EXTERNAL_TI2,   ///< External clock from TI2 input pin
    TIMER_CLK_EXTERNAL_ETR,   ///< External clock from ETR pin (External Trigger)
    TIMER_CLK_TRIGGER,        ///< Clock from internal trigger (TRGI) from another timer
    TIMER_CLK_ENCODER         ///< Encoder interface mode using TI1 and TI2
} timer_clk_src_t;

typedef enum {
    TIMER_IC_DIRECT=1,    ///< Capture from the channel's own input (TIx → ICx)
    TIMER_IC_INDIRECT=2,  ///< Capture from the opposite channel input (TIy → ICx), used for duty-cycle measurement
    TIMER_IC_TRIGGER=3    ///< Capture from internal trigger controller (TRC)
} timer_ic_src_t;

typedef enum {
    TIMER_IC_DIV_1 = 0, ///< Capture each valid edge event (no prescaler)
    TIMER_IC_DIV_2,     ///< Capture every 2 edge events
    TIMER_IC_DIV_4,     ///< Capture every 4 edge events
    TIMER_IC_DIV_8      ///< Capture every 8 edge events
} timer_ic_div_t;

typedef enum {
    TIMER_IC_FILTER_CKDTS_NONE = 0, ///< No digital filter applied
    TIMER_IC_FILTER_CKINT_2N,       ///< Filter with 2 sampling clock cycles
    TIMER_IC_FILTER_CKINT_4N,       ///< Filter with 4 sampling clock cycles
    TIMER_IC_FILTER_CKINT_8N,       ///< Filter with 8 sampling clock cycles
    TIMER_IC_FILTER_CKDTS_DIV2_6N,	///< Filter with 6 sampling clock cycles
    TIMER_IC_FILTER_CKDTS_DIV2_8N,	///< Filter with 8 sampling clock cycles
	TIMER_IC_FILTER_CKDTS_DIV4_6N,	///< Filter with 6 sampling clock cycles
	TIMER_IC_FILTER_CKDTS_DIV4_8N,	///< Filter with 8 sampling clock cycles
	TIMER_IC_FILTER_CKDTS_DIV8_6N,	///< Filter with 6 sampling clock cycles
	TIMER_IC_FILTER_CKDTS_DIV8_8N,	///< Filter with 8 sampling clock cycles
	TIMER_IC_FILTER_CKDTS_DIV16_5N,	///< Filter with 5 sampling clock cycles
	TIMER_IC_FILTER_CKDTS_DIV16_6N,	///< Filter with 6 sampling clock cycles
	TIMER_IC_FILTER_CKDTS_DIV16_8N,	///< Filter with 8 sampling clock cycles
	TIMER_IC_FILTER_CKDTS_DIV32_5N,	///< Filter with 5 sampling clock cycles
	TIMER_IC_FILTER_CKDTS_DIV32_6N,	///< Filter with 6 sampling clock cycles
	TIMER_IC_FILTER_CKDTS_DIV32_8N 	///< Filter with 8 sampling clock cycles
} timer_ic_filter_t;

typedef enum {
    TIMER_IC_RISING=0,    ///< Capture on rising edge
    TIMER_IC_FALLING   ///< Capture on falling edge
} timer_ic_polarity_t;

typedef enum {
    TIMER_OC_POLARITY_HIGH = 0, ///< Active high output polarity
    TIMER_OC_POLARITY_LOW       ///< Active low output polarity
} timer_oc_polarity_t;

typedef enum {
    TIMER_OC_FROZEN = 0, ///< Output compare has no effect on output pin
    TIMER_OC_SET_ACTIVE,     ///< Output set active on compare match
    TIMER_OC_SET_INACTIVE,   ///< Output set inactive on compare match
	TIMER_OC_TOGGLE,     ///< Output toggles when counter matches compare value
	TIMER_OC_FORCE_INACTIVE,     ///< Output forced active on compare match
    TIMER_OC_FORCE_ACTIVE,   ///< Output forced inactive on compare match
    TIMER_OC_PWM1,       ///< PWM mode 1 (active until compare match)
    TIMER_OC_PWM2        ///< PWM mode 2 (inactive until compare match)
} timer_oc_mode_t;


typedef enum {
    TIMER_ENCODER_MODE_TI1 = 1,  ///< Encoder mode 1: cuenta usando TI1, TI2 define dirección
    TIMER_ENCODER_MODE_TI2 = 2,  ///< Encoder mode 2: cuenta usando TI2, TI1 define dirección
    TIMER_ENCODER_MODE_TI12 = 3  ///< Encoder mode 3: cuenta en TI1 y TI2 (cuadratura x4)
} timer_encoder_mode_t;

typedef enum {
    TIMER_IRQ_UPDATE = 0,	///< Update: Desbordamiento o recarga del timer
    TIMER_IRQ_CC1,    		///< Capture/Compare: Evento de captura o comparación en canal 1
	TIMER_IRQ_CC2,    		///< Capture/Compare: Evento de captura o comparación en canal 2
	TIMER_IRQ_CC3,    		///< Capture/Compare: Evento de captura o comparación en canal 3
	TIMER_IRQ_CC4,    		///< Capture/Compare: Evento de captura o comparación en canal 4
    TIMER_IRQ_TRIGGER,		///< Trigger: Evento de disparo externo/interno
    TIMER_IRQ_BREAK  		///< Break: Parada por protección (PWM, fault, etc.)
} timer_irq_t;

typedef enum {
    TIMER_TRGO_RESET        = 0,
    TIMER_TRGO_CNTEN        = 1,
    TIMER_TRGO_UPDATE       = 2,
    TIMER_TRGO_COMPARE_PULSE= 3,
    TIMER_TRGO_OC1          = 4,
    TIMER_TRGO_OC2          = 5,
    TIMER_TRGO_OC3          = 6,
    TIMER_TRGO_OC4          = 7
} timer_trgo_t;

/**
 * @brief Trigger Input (TRGI) source selection for timer slave mode.
 *
 * This enum defines the possible sources that can act as trigger input (TRGI)
 * for the timer. Internally, this maps to the TS[2:0] bits in the SMCR register.
 *
 * These sources can come from:
 * - Internal triggers (ITR0–ITR3) from other timers (depends on the actual TIMx, see next table)
 * - Timer input channels (TI1, TI2)
 * - External trigger pin (ETR)
 *
 *
 * The table below shows which timer controls the actual TIMx count when using ITRx input.
 *
 * --------------------------------------------------------
 *  TIMx   |   ITR0     |   ITR1     |   ITR2     |   ITR3
 * --------------------------------------------------------
 *  TIM1   |   TIM2     |   TIM3     |   TIM4     |   TIM8
 *  TIM2   |   TIM1     |   TIM8     |   TIM3     |   TIM4
 *  TIM3   |   TIM1     |   TIM2     |   TIM4     |   TIM5
 *  TIM4   |   TIM1     |   TIM2     |   TIM3     |   TIM8
 * --------------------------------------------------------
 *
 */
typedef enum {
    TIMER_TRGI_ITR0 = 0,    /**< Internal Trigger 0 (from another timer) */
    TIMER_TRGI_ITR1,        /**< Internal Trigger 1 */
    TIMER_TRGI_ITR2,        /**< Internal Trigger 2 */
    TIMER_TRGI_ITR3,        /**< Internal Trigger 3 */
    TIMER_TRGI_TI1FP1,      /**< Filtered Timer Input 1 (TI1) */
    TIMER_TRGI_TI2FP2,      /**< Filtered Timer Input 2 (TI2) */
    TIMER_TRGI_ETRF         /**< External Trigger input (ETR pin) */
} timer_trgi_t;


/**
 * @brief Timer Slave Mode selection.
 *
 * This enum defines how the timer behaves when a trigger input (TRGI)
 * event occurs. It maps to the SMS[2:0] bits in the SMCR register.
 *
 * In encoder mode, the TIMx is able to count the pulses of encoders automatically
 * according to the mode selected:
 *
 * ------------------------------------------------------
 * |Encoder Mode|	Mode 1	|	Mode 2	|	Mode 3		|
 * ------------------------------------------------------
 * |	Count	|	TI2		|	TI1		|	Both		|
 * |	Dir		|	TI1		|	TI2		|	Quadrature	|
 * ------------------------------------------------------
 */
typedef enum {
    TIMER_SLAVE_DISABLED = 0,        /**< Slave mode disabled */
    TIMER_SLAVE_ENCODER_MODE1,       /**< Encoder mode 1 */
    TIMER_SLAVE_ENCODER_MODE2,       /**< Encoder mode 2 */
    TIMER_SLAVE_ENCODER_MODE3,       /**< Encoder mode 3 */
    TIMER_SLAVE_RESET,               /**< Reset counter on trigger */
    TIMER_SLAVE_GATED,               /**< Count only while trigger is high */
    TIMER_SLAVE_TRIGGER,             /**< Start counter on trigger */
    TIMER_SLAVE_EXTERNAL_CLOCK       /**< External clock mode (TRGI as clock) */
} timer_slave_mode_t;

/**
 * @brief Inicializa un temporizador.
 *
 * Configura los parámetros básicos de operación del timer:
 * - Modo de conteo (ascendente, descendente o center-aligned)
 * - Periodo de auto-recarga (ARR)
 * - Prescaler del reloj
 *
 * El timer no comienza a contar hasta que se llame a timer_start().
 *
 * La frecuencia final del contador se calcula como:
 *
 *     f_timer = f_timer_clk / (prescaler + 1)
 *
 * y el periodo del timer será:
 *
 *     T = (period + 1) / f_timer
 *
 * @param TIMx       Puntero al periférico del timer (TIM1, TIM2, TIM3, TIM4).
 * @param mode       Modo de conteo del timer.
 * @param period     Valor de auto-recarga (ARR). Define cuándo se reinicia el contador.
 * @param prescaler  Prescaler del timer (PSC).
 */
void timer_init(TIM_TypeDef *TIMx, timer_mode_t mode, uint32_t period, uint16_t prescaler);

/**
 * @brief Selecciona la fuente de reloj del timer.
 *
 * Permite que el timer cuente usando diferentes fuentes de reloj:
 * - Reloj interno del bus APB
 * - Señales externas en los pines TI1 o TI2
 * - Entrada externa ETR
 * - Trigger de otro timer
 * - Modo encoder (para decodificación de encoders en cuadratura)
 *
 * @param TIMx  Puntero al periférico del timer.
 * @param src   Fuente de reloj seleccionada.
 */
void timer_set_clock_source(TIM_TypeDef *TIMx, timer_clk_src_t src);

uint32_t timer_get_clock(TIM_TypeDef *TIMx);

/**
 * @brief Habilita o deshabilita una interrupción del timer.
 *
 * Permite activar interrupciones generadas por el timer como:
 * - Evento de actualización (overflow)
 * - Input capture
 * - Output compare
 * - Trigger
 *
 * La ISR correspondiente debe implementarse en el vector de interrupciones
 * del microcontrolador.
 *
 * @param TIMx   Puntero al periférico del timer.
 * @param irq    Tipo de interrupción a configurar.
 * @param state  Estado de la interrupción (habilitada o deshabilitada).
 */
void timer_set_interrupt(TIM_TypeDef *TIMx, timer_irq_t irq, irq_state_t state);


/**
 * @brief Configura la fuente de Trigger Output (TRGO) del timer.
 *
 * Permite seleccionar qué evento interno del timer será utilizado como
 * señal de disparo (TRGO), la cual puede ser utilizada para sincronizar
 * otros periféricos como ADC, DAC u otros timers.
 *
 * Las posibles fuentes incluyen:
 * - Evento de actualización (overflow)
 * - Habilitación del contador
 * - Evento de comparación (Output Compare)
 * - Pulso de comparación
 * - Reset por software
 *
 * Esta señal es comúnmente utilizada para disparar conversiones del ADC
 * de manera periódica o sincronizada con eventos de temporización.
 *
 * @param TIMx  Puntero al periférico del timer.
 * @param trgo  Fuente de trigger de salida (TRGO) a configurar.
 */
void timer_set_trgo(TIM_TypeDef *TIMx, timer_trgo_t trgo);

/**
 * @brief Inicia el conteo del timer.
 *
 * Habilita el contador del timer. A partir de este momento el timer
 * comenzará a contar según la configuración establecida.
 *
 * Puede configurarse en:
 * - Modo continuo
 * - Modo one-shot (se detiene automáticamente después del primer periodo)
 *
 * @param TIMx      Puntero al periférico del timer.
 * @param one_shot  Si es distinto de cero, el timer se ejecuta en modo one-shot.
 */
void timer_start(TIM_TypeDef *TIMx, timer_run_mode_t run_mode);

/**
 * @brief Inicia el conteo del timer.
 *
 * Habilita el contador del timer. A partir de este momento el timer
 * comenzará a contar según la configuración establecida.
 *
 * Puede configurarse en:
 * - Modo continuo
 * - Modo one-shot (se detiene automáticamente después del primer periodo)
 *
 * @param TIMx      Puntero al periférico del timer.
 * @param one_shot  Si es distinto de cero, el timer se ejecuta en modo one-shot.
 */
void timer_stop(TIM_TypeDef *TIMx);


/**
 * @brief Detiene el conteo del timer.
 *
 * Deshabilita el contador del timer, deteniendo el conteo inmediatamente.
 *
 * @param TIMx  Puntero al periférico del timer.
 */
void timer_reset(TIM_TypeDef *TIMx);

/**
 * @brief Configura un canal del timer en modo Input Capture.
 *
 * Permite capturar el valor del contador cuando ocurre un evento
 * en una señal externa conectada al pin del canal del timer.
 *
 * Este modo se utiliza comúnmente para:
 * - Medir frecuencia
 * - Medir periodo
 * - Medir duty cycle
 * - Detectar tiempos entre eventos
 *
 * Cuando ocurre el evento configurado, el valor del contador se
 * almacena automáticamente en el registro CCRx del canal.
 *
 * @param TIMx      Puntero al periférico del timer.
 * @param ch        Canal del timer (1–4).
 * @param src       Fuente de la señal de captura (directa, indirecta o trigger).
 * @param polarity  Flanco que genera la captura (subida, bajada o ambos).
 * @param div       Prescaler de captura (capturar cada N eventos).
 * @param filter    Filtro digital aplicado a la entrada.
 */
void timer_set_ic_channel(
        TIM_TypeDef *TIMx,
        timer_channel_t ch,
        timer_ic_src_t src,
        timer_ic_polarity_t polarity,
        timer_ic_div_t div,
        timer_ic_filter_t filter
);

void timer_set_ic_polarity(TIM_TypeDef *TIMx, timer_channel_t ch, timer_ic_polarity_t polarity);
uint32_t timer_get_capture_value(TIM_TypeDef *TIMx, timer_channel_t ch);


/**
 * @brief Configura un canal de Output Compare de un temporizador.
 *
 * Esta función configura uno de los canales del temporizador en modo
 * Output Compare. Permite seleccionar el modo de comparación, la
 * polaridad de la salida, el valor de comparación y si la salida
 * del canal se encuentra habilitada o no.
 *
 * Internamente, la función configura los registros CCMR, CCER y
 * el registro de comparación CCRx correspondientes al canal
 * seleccionado.
 *
 * @param TIMx Puntero a la instancia del periférico temporizador.
 *             Ejemplos: TIM1, TIM2, TIM3, etc.
 *
 * @param ch Canal del temporizador a configurar.
 *           Los valores posibles están definidos en @ref timer_channel_t:
 *           - TIMER_CHANNEL_1
 *           - TIMER_CHANNEL_2
 *           - TIMER_CHANNEL_3
 *           - TIMER_CHANNEL_4
 *
 * @param mode Modo de operación de Output Compare.
 *             Definido en @ref timer_oc_mode_t. Algunos modos típicos son:
 *             - Frozen
 *             - Active on match
 *             - Inactive on match
 *             - Toggle
 *             - PWM mode 1
 *             - PWM mode 2
 *
 * @param polarity Polaridad de la salida del canal.
 *                 Definida en @ref timer_oc_polarity_t:
 *                 - TIMER_OC_POLARITY_HIGH
 *                 - TIMER_OC_POLARITY_LOW
 *
 * @param compare_value Valor cargado en el registro de comparación del canal (CCRx).
 *                      Cuando el contador del temporizador alcanza este valor,
 *                      se ejecuta la acción definida por el modo de Output Compare.
 *
 * @param output_enabled Indica si la salida del canal debe habilitarse.
 *                       - true  : habilita la salida en el pin asociado.
 *                       - false : deshabilita la salida.
 *
 * @note El pin GPIO correspondiente debe configurarse previamente en
 *       modo de función alternativa (Alternate Function) para que la
 *       señal del temporizador pueda observarse en el pin.
 *
 * @note La configuración base del temporizador (prescaler, auto-reload,
 *       habilitación del contador, etc.) debe realizarse previamente.
 *
 * @warning En temporizadores de control avanzado (por ejemplo TIM1 o TIM8),
 *          también es necesario habilitar la salida principal (bit MOE)
 *          en el registro BDTR.
 */
void timer_set_oc_channel(
        TIM_TypeDef *TIMx,
        timer_channel_t ch,
        timer_oc_mode_t mode,
        timer_oc_polarity_t polarity,
        uint32_t compare_value,
        bool output_enabled
);

void timer_set_compare_value(TIM_TypeDef *TIMx, timer_channel_t ch, uint32_t value);


/**
 * @brief Configure timer slave mode and trigger input source.
 *
 * This function configures the timer to operate in slave mode, defining
 * both the behavior of the counter (slave mode) and the trigger input source (TRGI).
 *
 * Internally modifies the SMCR register:
 * - SMS[2:0]: 	Slave mode selection
 * - TS[2:0]: 	Trigger input selection
 *
 * @param tim       Pointer to timer instance (e.g., TIM1, TIM2, ...)
 * @param mode      Slave mode configuration (@ref timer_slave_mode_t)
 * @param trgi      Trigger input source (@ref timer_trgi_t)
 */
void timer_set_slave(TIM_TypeDef *tim,
                     timer_slave_mode_t mode,
                     timer_trgi_t trgi);

/**
 * @brief Configura el temporizador para operar en modo encoder incremental.
 *
 * Esta función configura el temporizador para decodificar señales de un
 * encoder rotatorio en cuadratura utilizando dos entradas (TI1 y TI2).
 * El hardware del timer se encarga automáticamente de:
 * - Contar los pulsos del encoder
 * - Determinar la dirección de rotación
 * - Incrementar o decrementar el contador (CNT)
 *
 * Dependiendo del modo seleccionado, el temporizador utilizará una o
 * ambas señales del encoder como fuente de conteo.
 *
 * Los canales del timer se configuran internamente como entradas:
 * - CH1 → señal A del encoder
 * - CH2 → señal B del encoder
 *
 * El contador del timer (CNT) aumentará o disminuirá automáticamente
 * según la dirección detectada a partir de las señales de cuadratura.
 *
 * @note
 * Para que el modo encoder funcione correctamente:
 * - Los pines GPIO correspondientes a CH1 y CH2 deben configurarse como
 *   entradas (floating o pull-up).
 * - El timer debe estar habilitado posteriormente con timer_start().
 *
 * @note
 * La dirección actual de conteo puede consultarse mediante el bit DIR
 * del registro CR1 del timer.
 *
 * @param TIMx
 * Puntero al temporizador a configurar (por ejemplo: TIM1, TIM2, TIM3, TIM4).
 *
 * @param mode
 * Modo de decodificación del encoder:
 * - TIMER_ENCODER_MODE_1 : Cuenta flancos de TI1, TI2 determina dirección.
 * - TIMER_ENCODER_MODE_2 : Cuenta flancos de TI2, TI1 determina dirección.
 * - TIMER_ENCODER_MODE_3 : Cuenta flancos de TI1 y TI2 (máxima resolución).
 *
 * @see timer_get_encoder_count()
 * @see timer_start()
 */
void timer_set_encoder_mode(TIM_TypeDef *TIMx, timer_encoder_mode_t mode);

/**
 * @brief Obtiene el conteo actual del encoder.
 *
 * Devuelve el valor actual del contador del timer utilizado
 * para decodificar el encoder.
 *
 * El valor puede ser positivo o negativo dependiendo
 * de la dirección de movimiento.
 *
 * @param TIMx  Puntero al periférico del timer.
 *
 * @return Valor actual del contador del encoder.
 */
int32_t timer_get_encoder_count(TIM_TypeDef *TIMx);


/**
 * @brief Tipos de eventos DMA para TIM
 */
typedef enum {
    TIMER_DMA_UPDATE = 0, /**< DMA para evento Update */
    TIMER_DMA_CC1,        /**< DMA para Capture/Compare 1 */
    TIMER_DMA_CC2,        /**< DMA para Capture/Compare 2 */
    TIMER_DMA_CC3,        /**< DMA para Capture/Compare 3 */
    TIMER_DMA_CC4         /**< DMA para Capture/Compare 4 */
} timer_dma_event_t;

typedef enum {
    TIMER_DMA_PERIPH_ARR,
    TIMER_DMA_PERIPH_CCR1,
    TIMER_DMA_PERIPH_CCR2,
    TIMER_DMA_PERIPH_CCR3,
    TIMER_DMA_PERIPH_CCR4
} timer_dma_periph_t;

/**
 * @brief Configura DMA para un Timer
 *
 * Inicializa y habilita el DMA para un evento específico del Timer.
 * Habilita la solicitud DMA en el timer y las interrupciones del DMA.
 *
 * @param tim: Puntero al periférico TIM (TIM1, TIM2, ...)
 * @param dir: Direccion de DMA (DMA_DIR_MEM_TO_PERIPH, DMA_DIR_PERIPH_TO_MEM)
 * @param mode: Modo de operación del DMA (DMA_MODE_NORMAL o DMA_MODE_CIRCULAR)
 * @param dma_event: Evento del timer para el DMA (UPDATE, CC1, CC2, CC3, CC4)
 * @param periph_reg: Registro de periférico (ARR, CCR1, CCR2, CCR3, CCR4)
 * @param buffer: Puntero al buffer en memoria
 * @param length: Número de elementos en buffer
 */
void timer_set_dma(TIM_TypeDef *tim,
				   dma_dir_t dir,
                   dma_mode_t mode,
                   timer_dma_event_t event,
                   timer_dma_periph_t periph_reg,
				   void *buffer,
                   uint16_t length);

DMA_Channel_TypeDef* timer_get_dma_channel(TIM_TypeDef *tim, timer_dma_event_t event);






/*
 ***********************************************************************
 * ANALOG TO DIGITAL CONVERTER PERIPHERAL
 ***********************************************************************
 */

/**
 * @brief Modos de operación del ADC.
 *
 * Define cómo se realizan las conversiones analógicas:
 * si se convierten uno o varios canales y si las conversiones
 * se ejecutan una sola vez o de forma continua.
 */
typedef enum {
	ADC_MODE_SINGLE = 0,        ///< Conversión única de un solo canal. El ADC se detiene al terminar.
	ADC_MODE_CONTINUOUS = 1,    ///< Conversión continua de un solo canal. El ADC reinicia automáticamente la conversión.
	ADC_MODE_SCAN_SINGLE = 2,   ///< Escaneo de múltiples canales definido por una secuencia, ejecutado una sola vez.
	ADC_MODE_SCAN_CONTINUOUS = 3///< Escaneo continuo de múltiples canales. La secuencia completa se repite automáticamente.
} adc_mode_t;

/**
 * @brief Canales disponibles del ADC.
 *
 * Cada canal corresponde a una entrada analógica del microcontrolador
 * o a señales internas del sistema.
 */
typedef enum {
	ADC_CHANNEL_0  = 0,   		///< Canal IN0 (PA0)
	ADC_CHANNEL_1  = 1,   		///< Canal IN1 (PA1)
	ADC_CHANNEL_2  = 2,   		///< Canal IN2 (PA2)
	ADC_CHANNEL_3  = 3,   		///< Canal IN3 (PA3)
	ADC_CHANNEL_4  = 4,   		///< Canal IN4 (PA4)
	ADC_CHANNEL_5  = 5,   		///< Canal IN5 (PA5)
	ADC_CHANNEL_6  = 6,   		///< Canal IN6 (PA6)
	ADC_CHANNEL_7  = 7,   		///< Canal IN7 (PA7)
	ADC_CHANNEL_8  = 8,   		///< Canal IN8 (PB0)
	ADC_CHANNEL_9  = 9,   		///< Canal IN9 (PB1)
	ADC_CHANNEL_10 = 10,  		///< Canal IN10 (PC0)
	ADC_CHANNEL_11 = 11,  		///< Canal IN11 (PC1)
	ADC_CHANNEL_12 = 12,  		///< Canal IN12 (PC2)
	ADC_CHANNEL_13 = 13,  		///< Canal IN13 (PC3)
	ADC_CHANNEL_14 = 14,  		///< Canal IN14 (PC4)
	ADC_CHANNEL_15 = 15,  		///< Canal IN15 (PC5)
	ADC_CHANNEL_TSENSE = 16,	///< Sensor de temperatura interno
	ADC_CHANNEL_VREFINT = 17 	///< Referencia interna de voltaje (VREFINT)
} adc_channel_t;


/**
 * @brief Fuentes de disparo (trigger) del ADC.
 *
 * Define qué evento inicia una conversión ADC.
 * Puede ser generado por timers, eventos externos o software.
 */
typedef enum {
	ADC_TRIG_TIM1_CC1 = 0, ///< Evento Compare Channel 1 del Timer 1
	ADC_TRIG_TIM1_CC2 = 1, ///< Evento Compare Channel 2 del Timer 1
	ADC_TRIG_TIM1_CC3 = 2, ///< Evento Compare Channel 3 del Timer 1
	ADC_TRIG_TIM2_CC2 = 3, ///< Evento Compare Channel 2 del Timer 2
	ADC_TRIG_TIM3_TRGO = 4, ///< Trigger Output (TRGO) del Timer 3
	ADC_TRIG_TIM4_CC4 = 5, ///< Evento Compare Channel 4 del Timer 4
	ADC_TRIG_EXTI11 = 6, ///< Evento externo en la línea EXTI11
	ADC_TRIG_SOFTWARE = 7 ///< Conversión iniciada por software
} adc_trigger_t;

/**
 * @brief Tiempo de muestreo del ADC.
 *
 * Define la cantidad de ciclos del reloj ADC que se emplean
 * para muestrear la señal analógica antes de iniciar la conversión.
 *
 * Un mayor tiempo de muestreo mejora la precisión para fuentes
 * de alta impedancia.
 */
typedef enum {
	ADC_SMP_1_5   = 0, ///< 1.5 ciclos de reloj ADC
	ADC_SMP_7_5   = 1, ///< 7.5 ciclos
	ADC_SMP_13_5  = 2, ///< 13.5 ciclos
	ADC_SMP_28_5  = 3, ///< 28.5 ciclos
	ADC_SMP_41_5  = 4, ///< 41.5 ciclos
	ADC_SMP_55_5  = 5, ///< 55.5 ciclos
	ADC_SMP_71_5  = 6, ///< 71.5 ciclos
	ADC_SMP_239_5 = 7  ///< 239.5 ciclos (máxima precisión)
} adc_sample_t;

/**
 * @brief Alineación del resultado de conversión ADC.
 *
 * Define cómo se almacenan los 12 bits del resultado
 * dentro del registro de datos del ADC.
 */
typedef enum {
	ADC_ALIGN_RIGHT = 0, ///< Alineación a la derecha (formato típico)
	ADC_ALIGN_LEFT  = 1  ///< Alineación a la izquierda (útil para resolución menor)
} adc_align_t;

/**
 * @brief Fuentes de interrupción del ADC.
 *
 * Permiten generar una interrupción cuando ocurre
 * un evento específico durante la conversión.
 */
typedef enum {
    ADC_IRQ_EOC=0,   ///< End Of Conversion: Conversión ADC completada
    ADC_IRQ_EOS,   ///< End Of Sequence: Secuencia completa terminada
    ADC_IRQ_AWD    ///< Analog Watchdog: Valor fuera de rango
} adc_irq_t;

/**
 * @brief Inicializa y configura el ADC.
 *
 * Esta función prepara el ADC para operar configurando:
 *
 * - Modo de conversión
 * - Tiempo de muestreo
 * - Fuente de trigger
 * - Alineación del resultado
 *
 * También realiza la habilitación básica del ADC necesaria
 * para iniciar conversiones.
 *
 * @param ADCx
 * Instancia del ADC a configurar (ADC1 o ADC2).
 *
 * @param mode
 * Modo de operación definido en @ref adc_mode_t.
 *
 * @param samp
 * Tiempo de muestreo definido en @ref adc_sample_t.
 *
 * @param trig
 * Fuente de disparo definida en @ref adc_trigger_t.
 *
 * @param align
 * Alineación del resultado definida en @ref adc_align_t.
 */
void adc_init(ADC_TypeDef *ADCx, adc_mode_t mode, adc_trigger_t trig, adc_align_t align);

/**
 * @brief Habilita o deshabilita interrupciones del ADC.
 *
 * Permite configurar qué eventos del ADC generarán
 * una interrupción hacia la NVIC.
 *
 * @param ADCx
 * Instancia del ADC.
 *
 * @param irq
 * Tipo de interrupción definido en @ref adc_irq_t.
 *
 * @param state
 * Estado de la interrupción (IRQ_ENABLE o IRQ_DISABLE).
 *
 * @note Recuerde que tiene que habilitar manualmente la
 *		   interrupcion en el NVIC mediante NVIC_IRQEnable(ADC1_2_IRQn);
 */
void adc_set_interrupt(ADC_TypeDef *ADCx, adc_irq_t irq, irq_state_t state);

/**
 * @brief Selecciona un canal ADC para conversión simple.
 *
 * Configura el canal que será convertido cuando el ADC
 * se encuentra en modo de conversión simple.
 *
 * @param ADCx
 * Instancia del ADC.
 *
 * @param channel
 * Canal a convertir definido en @ref adc_channel_t.
 */
void adc_set_channel(ADC_TypeDef *ADCx, adc_channel_t channel, adc_sample_t samp);

/**
 * @brief Configura una secuencia de canales para modo scan.
 *
 * Define el orden de conversión de múltiples canales
 * cuando el ADC opera en modo de escaneo.
 *
 * @param ADCx
 * Instancia del ADC.
 *
 * @param channels
 * Arreglo con los canales a convertir.
 *
 * @param len
 * Número de canales en la secuencia.
 */
void adc_set_sequence(ADC_TypeDef *ADCx, const adc_channel_t *channels, const adc_sample_t *samps, uint8_t len);

/**
 * @brief Lee el resultado de una conversión ADC.
 *
 * Esta función inicia la conversión (si es necesario),
 * espera a que termine y devuelve el valor convertido.
 *
 * @param ADCx
 * Instancia del ADC.
 *
 * @return
 * Valor digital convertido (12 bits).
 */
uint16_t adc_read(ADC_TypeDef *ADCx);

/**
 * @brief Lee los resultados de una secuencia de conversiones ADC.
 *
 * Utilizada cuando el ADC está configurado en modo scan.
 * La función llena un buffer con los resultados de cada canal
 * en el orden configurado en la secuencia.
 *
 * @param ADCx
 * Instancia del ADC.
 *
 * @param buffer
 * Arreglo donde se almacenarán los resultados.
 */
void adc_read_sequence(ADC_TypeDef *ADCx, uint16_t *buffer);

/**
 * @brief Inicia el proceso de conversión ADC.
 *
 * Dependiendo del modo configurado, iniciará una conversión
 * simple o una secuencia completa de conversiones.
 *
 * @param ADCx
 * Instancia del ADC.
 */
void adc_start(ADC_TypeDef *ADCx);

/**
 * @brief Detiene el ADC.
 *
 * Deshabilita el ADC y detiene cualquier conversión
 * en curso.
 *
 * @param ADCx
 * Instancia del ADC.
 */
void adc_stop(ADC_TypeDef *ADCx);


/**
 * @brief Tipos de eventos DMA para el ADC
 */
typedef enum {
    ADC_DMA_REGULAR = 0   /**< DMA para conversiones regulares del ADC */
} adc_dma_event_t;

/**
 * @brief Configura DMA para un ADC
 *
 * Esta función inicializa y habilita el DMA para el ADC especificado.
 * También habilita la solicitud DMA en el ADC y las interrupciones del DMA.
 *
 * @param adc: Puntero al periférico ADC (ADC1, ADC2, ...)
 * @param buffer: Puntero al buffer en memoria donde se almacenarán los datos
 * @param length: Número de elementos a transferir
 * @param mode: Modo de operación del DMA (DMA_MODE_NORMAL o DMA_MODE_CIRCULAR)
 * @param dma_event: Tipo de conversión DMA (ADC_DMA_REGULAR o ADC_DMA_INJECTED)
 */
void adc_set_dma(ADC_TypeDef *adc, uint16_t *buffer, uint16_t length,
                 dma_mode_t mode, adc_dma_event_t dma_event);



/*
 **************************************************************
 * UNIVERSAL SYNCHRONOUS/ASYNCHRONOUS RECEIVER TRANSMITTER
 **************************************************************
 */
typedef enum {
	UART_MODE_TX = 0,
	UART_MODE_RX,
	UART_MODE_TX_RX
} uart_mode_t;

typedef enum {
	UART_PARITY_NONE = 0,
	UART_PARITY_EVEN,
	UART_PARITY_ODD
} uart_parity_t;

typedef enum {
	UART_STOP_1 = 0,
	UART_STOP_0_5,
	UART_STOP_2,
	UART_STOP_1_5
} uart_stopbits_t;

typedef enum {
    UART_IRQ_TX = 0,	///< Se puede enviar nuevo dato
    UART_IRQ_TC,        ///< Tansmisión completa
    UART_IRQ_RX,      	///< Nuevo dato recibido
    UART_IRQ_IDLE,      ///< Detección de línea
    UART_IRQ_PE,        ///< Error en paridad
    UART_IRQ_ERR        ///< Error (ORE, NE, FE)
} uart_irq_t;

typedef enum {
    UART_DMA_TX = 0,  ///< DMA para transmisión (USARTx->DR como destino)
    UART_DMA_RX       ///< DMA para recepción (USARTx->DR como origen)
} uart_dma_event_t;

/**
 * @brief Inicializa el periférico USART.
 *
 * Configura los parámetros básicos de comunicación serie:
 * - Baudrate
 * - Modo (TX, RX o ambos)
 * - Paridad
 * - Bits de parada
 *
 * @param USARTx   Instancia del periférico (USART1, USART2, etc.)
 * @param baudrate Velocidad de comunicación en baudios
 * @param mode     Modo de operación
 * @param parity   Configuración de paridad
 * @param stopbits Bits de parada
 */
void uart_init(USART_TypeDef *USARTx,
			   uint32_t baudrate,
			   uart_mode_t mode,
			   uart_parity_t parity,
			   uart_stopbits_t stopbits);

/**
 * @brief Envía un byte por UART en modo bloqueante.
 *
 * Esta función transmite un solo byte a través del periférico USART.
 * La función espera (bloquea la CPU) hasta que el registro de datos
 * de transmisión esté vacío (TXE) y el dato pueda ser enviado.
 *
 * Internamente:
 * - Espera a que el flag TXE esté activo
 * - Escribe el dato en USARTx->DR
 *
 * @param USARTx
 * Instancia del periférico USART (USART1, USART2, etc.).
 *
 * @param data
 * Byte a transmitir.
 *
 * @note
 * Esta función es bloqueante, por lo que detiene la ejecución hasta
 * que el periférico esté listo para transmitir el dato.
 *
 * @warning
 * Para transmisión eficiente de grandes volúmenes de datos, se recomienda
 * utilizar interrupciones o DMA en lugar de esta función.
 */
void uart_write_byte(USART_TypeDef *USARTx, uint8_t data);


/**
 * @brief Recibe un byte por UART en modo bloqueante.
 *
 * Esta función espera (bloquea la CPU) hasta que se reciba un dato
 * en el periférico USART. Una vez disponible, retorna el byte leído.
 *
 * Internamente:
 * - Espera a que el flag RXNE esté activo
 * - Lee el dato desde USARTx->DR
 *
 * @param USARTx
 * Instancia del periférico USART (USART1, USART2, etc.).
 *
 * @return
 * Byte recibido desde la interfaz UART.
 *
 * @note
 * Esta función es bloqueante, por lo que detiene la ejecución hasta
 * que se reciba un dato.
 *
 * @warning
 * Si no se recibe información, la función permanecerá bloqueada
 * indefinidamente. Para aplicaciones en tiempo real se recomienda
 * usar interrupciones o DMA.
 */
uint8_t uart_read_byte(USART_TypeDef *USARTx);


/**
 * @brief Envía un buffer completo por UART en modo bloqueante.
 *
 * Transmite una secuencia de bytes de longitud especificada a través
 * del periférico USART. La función envía cada byte de manera secuencial
 * utilizando uart_write_byte().
 *
 * @param USARTx
 * Instancia del periférico USART (USART1, USART2, etc.).
 *
 * @param data
 * Puntero al buffer de datos a transmitir.
 *
 * @param len
 * Número de bytes a enviar.
 *
 * @note
 * La función es bloqueante y no retorna hasta que todos los datos
 * han sido transmitidos.
 *
 * @warning
 * Para transferencias largas o continuas, se recomienda utilizar
 * DMA o interrupciones para evitar bloquear la CPU.
 */
void uart_write(USART_TypeDef *USARTx, const uint8_t *data, uint16_t len);


/**
 * @brief Recibe un buffer completo por UART en modo bloqueante.
 *
 * Esta función recibe una cantidad específica de bytes desde el
 * periférico USART y los almacena en el buffer proporcionado.
 *
 * La función espera de forma bloqueante la recepción de cada byte,
 * utilizando uart_read_byte().
 *
 * @param USARTx
 * Instancia del periférico USART (USART1, USART2, etc.).
 *
 * @param data
 * Puntero al buffer donde se almacenarán los datos recibidos.
 *
 * @param len
 * Número de bytes a recibir.
 *
 * @note
 * La función no retorna hasta haber recibido exactamente @p len bytes.
 *
 * @warning
 * Si no se reciben suficientes datos, la función permanecerá bloqueada.
 * Para aplicaciones robustas se recomienda implementar mecanismos
 * de timeout o utilizar interrupciones/DMA.
 */
void uart_read(USART_TypeDef *USARTx, uint8_t *data, uint16_t len);

/**
 * @brief Habilita o deshabilita interrupciones del USART.
 *
 * Permite seleccionar eventos específicos del USART que generarán
 * una interrupción hacia la NVIC.
 *
 * @param USARTx Instancia USART
 * @param irq    Tipo de interrupción (@ref uart_irq_t)
 * @param state  Estado (IRQ_ENABLE / IRQ_DISABLE)
 */
void uart_set_interrupt(USART_TypeDef *USARTx,
                        uart_irq_t irq,
                        irq_state_t state);

DMA_Channel_TypeDef* uart_get_dma_channel(USART_TypeDef *USARTx,
                                                 uart_dma_event_t event);

/**
 * @brief Configura DMA para el periférico USART.
 *
 * Esta función inicializa y habilita una transferencia DMA asociada
 * al periférico USART, permitiendo transmitir o recibir datos sin
 * intervención directa de la CPU.
 *
 * Dependiendo del evento seleccionado:
 *
 * - UART_DMA_TX:
 *   El DMA transfiere datos desde memoria hacia el registro de datos
 *   del USART (USARTx->DR), permitiendo transmisión continua.
 *
 * - UART_DMA_RX:
 *   El DMA transfiere datos desde el registro de datos del USART
 *   hacia memoria, permitiendo recepción continua.
 *
 * Internamente:
 * - Configura el canal DMA correspondiente al USART
 * - Establece direcciones de memoria y periférico
 * - Habilita la solicitud DMA en el USART (bits DMAT / DMAR)
 *
 * @param USARTx	Instancia del periférico USART (USART1, USART2, etc.).
 * @param event		Evento DMA a configurar (@ref uart_dma_event_t).
 * @param buffer	Puntero al buffer en memoria.
 * @param length	Número de elementos a transferir.
 *
 * @note
 * Para recepción continua se recomienda usar DMA en modo circular.
 *
 */
void uart_set_dma(USART_TypeDef *USARTx,
                  uart_dma_event_t event,
                  void *buffer,
                  uint16_t length);

#endif /* __STM32F103_HAL_H */
