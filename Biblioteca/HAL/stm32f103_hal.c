#include "stm32f103_hal.h"
#include <stdlib.h>

#define SYSCLK_MAX_FREQ	(72000000UL)
#define ADCCLK_MAX_FREQ	(14000000UL)
#define APB1CLK_MAX_FREQ	(32000000UL)

/*
 **************************************************************
 * RESET AND CLOCK CONTROL PERIPHERAL
 **************************************************************
 */
uint32_t SystemClock = HSI_VALUE;
uint32_t AHBClock = HSI_VALUE;
uint32_t APB1Clock = HSI_VALUE;
uint32_t APB2Clock = HSI_VALUE;
uint32_t ADCClock = HSI_VALUE / 2;
uint32_t USBClock = HSI_VALUE / 1.5;









void nvic_enable_irq(IRQn_Type IRQn)
{
  if ((int32_t)(IRQn) >= 0)
  {
    NVIC->ISER[(((uint32_t)IRQn) >> 5UL)] = (uint32_t)(1UL << (((uint32_t)IRQn) & 0x1FUL));
  }
}

void nvic_enable(void)
{
  __asm volatile ("cpsie i" : : : "memory");
}


/**
  \brief   Disable IRQ Interrupts
  \details Disables IRQ interrupts by setting the I-bit in the CPSR.
           Can only be executed in Privileged modes.
 */
void nvic_disable(void)
{
  __asm volatile ("cpsid i" : : : "memory");
}


void rcc_clock_enable(rcc_periph_clk_t clk)
{
	uint8_t uclk = (uint8_t)clk;
	if(uclk < 32)
	{
		RCC->APB1ENR |= 1 << uclk;
	}
	else if(uclk < 64)
	{
		uclk-=32;
		RCC->APB2ENR |= 1 << uclk;
	}
	else
	{
		uclk-=64;
		RCC->AHBENR |= 1 << uclk;
	}
}


void rcc_set_clock(rcc_sysclk_src_t src, rcc_pll_mul_t pll_mul)
{
	uint32_t pllmul = (uint32_t)pll_mul << 18;	// PLL multiplier
	uint32_t pllsrc = 0;	// HSI as PLL input
	uint32_t sysclk_src = 0;	// HSI
	SystemClock = HSI_VALUE;

	/* Comprobar src */
	uint8_t hse_en = src!=RCC_SYSCLK_HSI && src!=RCC_SYSCLK_PLL_HSI_DIV_2;
	uint8_t pll_en = src!=RCC_SYSCLK_HSI && src!=RCC_SYSCLK_HSE;

	// Apagar PLL
	RCC->CR &= ~RCC_CR_PLLON;
	while (RCC->CR & RCC_CR_PLLRDY);

	// Default
	SystemClock = HSI_VALUE;

	// HSE
	if (hse_en)
	{
			RCC->CR |= RCC_CR_HSEON;
			while (!(RCC->CR & RCC_CR_HSERDY));

			SystemClock = HSE_VALUE;
			pllsrc = RCC_CFGR_PLLSRC;
			sysclk_src = 0x01; 	// HSE

			if (src == RCC_SYSCLK_PLL_HSE_DIV_2)
			{
					SystemClock >>= 1; // FIX
					pllsrc |= RCC_CFGR_PLLXTPRE;
			}
	}

	// PLL
	if (pll_en)
	{
			SystemClock *= (pll_mul + 2);
			if(SystemClock > SYSCLK_MAX_FREQ)
			{
				SystemClock /= (pll_mul + 2);
				pll_mul = (uint32_t)SYSCLK_MAX_FREQ/SystemClock - 2;
				SystemClock *= (pll_mul + 2);
				pllmul = (uint32_t)pll_mul << 18;
			}
			if (!hse_en)
				SystemClock >>= 1;
			sysclk_src = 0x02;	// PLL

			RCC->CFGR &= ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL);
			RCC->CFGR |= pllsrc | pllmul;

			RCC->CR |= RCC_CR_PLLON;
			while (!(RCC->CR & RCC_CR_PLLRDY));
	}

	/* Reset prescalers */
	RCC->CFGR &= ~RCC_CFGR_PPRE2; // APB2Clock = AHB/1
	RCC->CFGR &= ~RCC_CFGR_PPRE1; // APB1Clock = AHB/1
	RCC->CFGR &= ~RCC_CFGR_ADCPRE; // ADCClock = APB2Clock / X
	RCC->CFGR &= ~RCC_CFGR_USBPRE; // USBClock = PLLClock/1.5
	AHBClock = SystemClock;
	APB1Clock = SystemClock;
	APB2Clock = SystemClock;
	ADCClock = APB2Clock >> 1;
	USBClock = SystemClock * 1.5;

	// Check for APB1 limits (<= 32MHz)
	if(SystemClock > APB1CLK_MAX_FREQ)
	{
		RCC->CFGR |= RCC_CFGR_PPRE1_DIV2; // APB1 /2
		APB1Clock = SystemClock >> 1;
	}

	// ADC Clock check
	uint8_t adc_prescaler = 4;
	while(ADCClock > ADCCLK_MAX_FREQ && adc_prescaler <= 8)
	{
		if (APB2Clock / adc_prescaler <= ADCCLK_MAX_FREQ)
		{
			RCC->CFGR &= ~RCC_CFGR_ADCPRE;
			ADCClock = APB2Clock >> (adc_prescaler>>1);
			RCC->CFGR |= (uint32_t)((adc_prescaler>>1)-1) << 14;
			break;
		}
		adc_prescaler+=2;
	}

	// FLASH antes del cambio
	FLASH->ACR &= ~FLASH_ACR_LATENCY;
	if (SystemClock <= 24000000)
			FLASH->ACR |= FLASH_ACR_LATENCY_0;
	else if (SystemClock <= 48000000)
			FLASH->ACR |= FLASH_ACR_LATENCY_1;
	else
			FLASH->ACR |= FLASH_ACR_LATENCY_2;
	FLASH->ACR |= FLASH_ACR_PRFTBE;

	/* Seleccionar fuente SYSCLK */
	RCC->CFGR &= ~0x03;
	RCC->CFGR |= sysclk_src;
	while (((RCC->CFGR >> 2) & 0x3) != sysclk_src);
}

void rcc_set_bus_clk_div(rcc_bus_t bus, rcc_div_t clkdiv)
{
	uint32_t uclkdiv = (uint32_t)clkdiv;
	if(bus == RCC_BUS_AHB)
	{
		uclkdiv += 0x04;
		RCC->CFGR &= ~(0x0F << 4);
		RCC->CFGR |= uclkdiv << 4;
    AHBClock = (uclkdiv < 5 ? SystemClock >> uclkdiv : SystemClock >> (uclkdiv+1));
	}
	else
	{
		if(uclkdiv > (uint32_t)RCC_DIV_16) return;
		uclkdiv += 0x03;
		if(bus == RCC_BUS_APB1)
		{
			RCC->CFGR &= ~(0x07 << 8);
			RCC->CFGR |= uclkdiv << 8;
      APB1Clock = SystemClock >> uclkdiv;
		}
		else
		{
			RCC->CFGR &= ~(0x07 << 11);
			RCC->CFGR |= uclkdiv << 11;
      APB2Clock = SystemClock >> uclkdiv;
		}
	}
}

void rcc_set_adc_clk_div(rcc_adcclk_div_t clkdiv)
{
	RCC->CFGR &= ~(0x03 << 14);
	RCC->CFGR |= (uint32_t)clkdiv << 14;
  ADCClock = (clkdiv != RCC_ADC_DIV_6 ? APB2Clock >> ((uint32_t)clkdiv + 1) : APB2Clock / 6);
}

void rcc_set_usb_clk_div(rcc_usbclk_div_t clkdiv)
{
	RCC->CFGR &= ~(1 << 18);
	RCC->CFGR |= (uint32_t)clkdiv << 18;
  USBClock = ( clkdiv==RCC_USB_DIV_1 ? SystemClock : SystemClock * 1.5);

}

uint32_t rcc_get_bus_clock(rcc_bus_t bus)
{
	switch(bus)
	{
		case RCC_BUS_AHB: return AHBClock; break;
		case RCC_BUS_APB1: return APB1Clock; break;
		case RCC_BUS_APB2: return APB2Clock; break;
	}

	return 0;
}

uint32_t rcc_get_sysclk(void)
{
	return SystemClock;
}

uint32_t rcc_get_adc_clock(void)
{
	return ADCClock;
}

uint32_t rcc_get_usb_clock(void)
{
	return USBClock;
}


/*
 **************************************************************
 * GENERAL PURPOSE INPUT/OUTPUT PERIPHERAL
 **************************************************************
 */
void gpio_set_mode(GPIO_TypeDef *GPIOx, uint32_t pin_num, uint8_t mode)
{
	if(mode > 0x03) return;
	if(pin_num >= GPIO_PIN_MAX) return;

	if(pin_num < 8)
	{
		GPIOx->CRL &= ~(0x03 << (pin_num<<2));
		GPIOx->CRL |= ((uint32_t)mode << (pin_num<<2));
	}
	else
	{
		pin_num -= 8;
		GPIOx->CRH &= ~(0x03 << (pin_num<<2));
		GPIOx->CRH |= ((uint32_t)mode << (pin_num<<2));
	}

}

void gpio_set_cnf(GPIO_TypeDef *GPIOx, uint32_t pin_num, uint8_t cfn)
{
	if(cfn & ~(0x0C)) return;
	if(pin_num >= GPIO_PIN_MAX) return;

	if(pin_num < 8)
	{
		GPIOx->CRL &= ~(0x0C << (pin_num<<2));
		GPIOx->CRL |= ((uint32_t)cfn << (pin_num<<2));
	}
	else
	{
		pin_num -= 8;
		GPIOx->CRH &= ~(0x0C << (pin_num<<2));
		GPIOx->CRH |= ((uint32_t)cfn << (pin_num<<2));
	}
}

void gpio_set_input(GPIO_TypeDef *GPIOx, uint32_t pin_num, gpio_input_t mode)
{
	gpio_set_mode(GPIOx, pin_num, GPIO_MODE_INPUT);
	gpio_set_cnf(GPIOx, pin_num, (uint8_t)mode & 0x0F);

	// Check for pull-up
	if((uint8_t)mode & 0x10) GPIOx->ODR |= (1 << pin_num);
	else GPIOx->ODR &= ~(1 << pin_num);
}

void gpio_set_output(GPIO_TypeDef *GPIOx, uint32_t pin_num, gpio_output_t mode, gpio_speed_t speed)
{
	gpio_set_mode(GPIOx, pin_num, (uint8_t)speed);
	gpio_set_cnf(GPIOx, pin_num, (uint8_t)mode & 0x0F);

	GPIOx->ODR &= ~(1 << pin_num);
}

void gpio_disable_debug_pins(void)
{
	RCC->APB2ENR |= 1 << 0; 	//Enable AFIO clock
	AFIO->MAPR   |= 2UL << 24;	// Disable JTAG (PA15, PB3,PB4 as GPIO)
}



void gpio_bitwise_set(GPIO_TypeDef *GPIOx, uint16_t mask)
{
	GPIOx->BSRR = mask;
}
void gpio_bitwise_clear(GPIO_TypeDef *GPIOx, uint16_t mask)
{
	GPIOx->BRR = mask;
}
void gpio_bitwise_toggle(GPIO_TypeDef *GPIOx, uint16_t mask)
{
	GPIOx->ODR ^= mask;
}

uint8_t gpio_read_pin(GPIO_TypeDef *GPIOx, uint8_t pin_num)
{
	return GPIOx->IDR & (1UL << pin_num) ? 1 : 0;
}


/*
 **************************************************************
 * EXTERNAL INTERRUPT CONTROLLER
 **************************************************************
 */
void exti_set_interrupt(pin_t pin, exti_irq_t irq)
{
	// Get port
	uint8_t port = (uint8_t)pin >> 4;
	uint8_t pin_num = (uint8_t)pin & 0xF;

	// Enable EXTI clock (AFIO)
	RCC->APB2ENR |= 1 << 0;

	// Set exti pin
	uint8_t cr_idx = pin_num >> 2;
	AFIO->EXTICR[cr_idx] &= ~(0xFUL << ((pin_num & 0x03)<<2));
	AFIO->EXTICR[cr_idx] |= ((uint32_t)port << ((pin_num & 0x03)<<2));

	// Set rising and falling as specified
	EXTI->FTSR &= ~(1 << pin_num);
	EXTI->RTSR &= ~(1 << pin_num);
	switch(irq)
	{
		case EXTI_IRQ_FALLING:
			EXTI->FTSR |=  (1 << pin_num);
			EXTI->RTSR &= ~(1 << pin_num);
		break;

		case EXTI_IRQ_RISING:
			EXTI->RTSR |=  (1 << pin_num);
			EXTI->FTSR &= ~(1 << pin_num);
		break;

		case EXTI_IRQ_BOTH:
			EXTI->FTSR |=  (1 << pin_num);
			EXTI->RTSR |=  (1 << pin_num);
		break;
	}

	// Setup EXTIn line as an interrupt source.
	EXTI->IMR  |=  (1 << pin_num);  // Enable Interrupt source
}







/*
 **************************************************************
 * DIRECT MEMORY ACCESS (DMA)
 **************************************************************
 */
void dma_init(DMA_Channel_TypeDef *ch,
              dma_dir_t dir,
              dma_size_t mem_size,
              dma_size_t periph_size,
              bool mem_inc,
              bool periph_inc,
              dma_mode_t mode,
              dma_priority_t priority)
{
	rcc_clock_enable(RCC_DMA1);

    ch->CCR &= ~DMA_CCR_EN; // Deshabilitar canal antes de configurar

    ch->CCR = 0;
    ch->CCR |= (dir == DMA_DIR_MEM_TO_PERIPH) ? DMA_CCR_DIR : 0;
    ch->CCR |= (mem_inc ? DMA_CCR_MINC : 0);
    ch->CCR |= (periph_inc ? DMA_CCR_PINC : 0);
    ch->CCR |= (mode == DMA_MODE_CIRCULAR) ? DMA_CCR_CIRC : 0;

    switch (mem_size) {
        case DMA_SIZE_8BIT:  ch->CCR |= 0; break;
        case DMA_SIZE_16BIT: ch->CCR |= DMA_CCR_MSIZE_0; break;
        case DMA_SIZE_32BIT: ch->CCR |= DMA_CCR_MSIZE_1; break;
    }
    switch (periph_size) {
        case DMA_SIZE_8BIT:  ch->CCR |= 0; break;
        case DMA_SIZE_16BIT: ch->CCR |= DMA_CCR_PSIZE_0; break;
        case DMA_SIZE_32BIT: ch->CCR |= DMA_CCR_PSIZE_1; break;
    }

    switch (priority) {
        case DMA_PRIORITY_LOW:    ch->CCR |= 0; break;
        case DMA_PRIORITY_MEDIUM: ch->CCR |= DMA_CCR_PL_0; break;
        case DMA_PRIORITY_HIGH:   ch->CCR |= DMA_CCR_PL_1; break;
        case DMA_PRIORITY_VERY_HIGH: ch->CCR |= (DMA_CCR_PL_0 | DMA_CCR_PL_1); break;
    }
}

void dma_set_addresses(DMA_Channel_TypeDef *ch, volatile void *periph_addr, void *mem_addr, uint16_t length)
{
    ch->CPAR = (uint32_t)periph_addr;
    ch->CMAR = (uint32_t)mem_addr;
    ch->CNDTR = length;
}

void dma_set_interrupt(DMA_Channel_TypeDef *ch, dma_irq_t irq, irq_state_t state)
{
    if (state) {
        switch (irq) {
            case DMA_IRQ_TC: ch->CCR |= DMA_CCR_TCIE; break;
            case DMA_IRQ_HT: ch->CCR |= DMA_CCR_HTIE; break;
            case DMA_IRQ_TE: ch->CCR |= DMA_CCR_TEIE; break;
        }
    } else {
        switch (irq) {
            case DMA_IRQ_TC: ch->CCR &= ~DMA_CCR_TCIE; break;
            case DMA_IRQ_HT: ch->CCR &= ~DMA_CCR_HTIE; break;
            case DMA_IRQ_TE: ch->CCR &= ~DMA_CCR_TEIE; break;
        }
    }
}

void dma_start(DMA_Channel_TypeDef *ch)
{
    ch->CCR |= DMA_CCR_EN;
}

void dma_stop(DMA_Channel_TypeDef *ch)
{
    ch->CCR &= ~DMA_CCR_EN;
}




/*
 **************************************************************
 * SYSTEM CLOCK TIMER
 **************************************************************
 */
void systick_config(uint32_t period, systick_clksrc_t clksrc)
{
	/*
	 * SysTick is a 24-bit counter with clock derived from the AHBClock
	 *
	 * Steps:
	 * 1. Programming reload value using LOAD register
	 * 2. Clear current value using the VAL variable
	 * 3. Select clock source via CLKSOURCE bit (2) on CTRL register.
	 *		0 - AHB/8
	 * 		1 - Processor Clock (AHB)
	 * 4. Enable Tick interrupt by setting TICKINT bit (1) of CTRL register.
	 * 5. Start SysTick timer using the ENABLE bit (0) of CTRL register
	 */
	SysTick->CTRL = 0;			// Reset Systick
	SysTick->LOAD = period;	// Set reload value
	SysTick->VAL = 0x00000000;	// Reset Systick counter
	SysTick->CTRL |= (uint32_t)clksrc << 2;	// Set clock source

	/* Set interrupt priority via SCB->SHP register.
	 * Although the SCB_SHPx are 32-bit registers (x = 1, 2, 3)
	 * the library define these registers as an array of 8-bit
	 * registers, since SHPR1-SHPR3 are byte accessible. To set the
	 * priority of the core peripherals, an 8-bit field is defined
	 * along each register, denoted to as PRI_n, where n = 4,...,15.
	 * The lower the value, the greater the priority of the
	 * corresponding interrupt.
	 *
	 * -------------------------------------------
	 * 			Handler 			Field Register
	 * -------------------------------------------
	 * Memory management fault 		PRI_4
	 * System handler Bus fault 	PRI_5
	 * Usage fault 					PRI_6
	 * SVCall 						PRI_11
	 * PendSV 						PRI_14
	 * SysTick 						PRI_15
	 * --------------------------------------------
	 *
	 * Each PRI_n field is 8 bits wide, but the processor implements
	 * only bits[7:4] of each field, and bits[3:0] read as zero
	 *
	 * SysTick is found on PRI_15 (MSByte of the SHPR3 register)
	 *
	 * In the library SCB->SHP[0] is related to PRI_4, SCB->SHP[1] to
	 * PRI_5, and so on. Therefore, to located a particular "n" we only
	 * need to subtract 4. Hence, for the Systick (PRI_15) n is 15, and
	 * is located on 15 - 4 = 11, i.e., on SCB->SHP[11].
	 */
	SCB->SHP[11] = (15 << 4); 	// Set the lower priority
	SysTick->CTRL |= 1 << 1;	// Enable Systick interrupt
	SysTick->CTRL |= 1 << 0;	// Enable Systick timer
}

/*
 **************************************************************
 * TIMER PERIPHERAL
 **************************************************************
 */
static void timer_config_gpio(TIM_TypeDef *TIMx, timer_channel_t ch, bool output)
{
    GPIO_TypeDef *port = NULL;
    uint8_t pin = 0;

    /* Determinar pin según timer y canal */
    if(TIMx == TIM1)
    {
        switch(ch)
        {
            case TIMER_CHANNEL_1: port = GPIOA; pin = 8;  break;
            case TIMER_CHANNEL_2: port = GPIOA; pin = 9;  break;
            case TIMER_CHANNEL_3: port = GPIOA; pin = 10; break;
            case TIMER_CHANNEL_4: port = GPIOA; pin = 11; break;
        }
    }
    else if(TIMx == TIM2)
    {
        switch(ch)
        {
            case TIMER_CHANNEL_1: port = GPIOA; pin = 0; break;
            case TIMER_CHANNEL_2: port = GPIOA; pin = 1; break;
            case TIMER_CHANNEL_3: port = GPIOA; pin = 2; break;
            case TIMER_CHANNEL_4: port = GPIOA; pin = 3; break;
        }
    }
    else if(TIMx == TIM3)
    {
        switch(ch)
        {
            case TIMER_CHANNEL_1: port = GPIOA; pin = 6; break;
            case TIMER_CHANNEL_2: port = GPIOA; pin = 7; break;
            case TIMER_CHANNEL_3: port = GPIOB; pin = 0; break;
            case TIMER_CHANNEL_4: port = GPIOB; pin = 1; break;
        }
    }
    else if(TIMx == TIM4)
    {
        switch(ch)
        {
            case TIMER_CHANNEL_1: port = GPIOB; pin = 6; break;
            case TIMER_CHANNEL_2: port = GPIOB; pin = 7; break;
            case TIMER_CHANNEL_3: port = GPIOB; pin = 8; break;
            case TIMER_CHANNEL_4: port = GPIOB; pin = 9; break;
        }
    }

    if(port == NULL) return;

    /* Configurar modo GPIO */
    if(output)
        gpio_set_output(port, pin, GPIO_OUTPUT_AF_PP, GPIO_SPEED_50MHZ);
    else
        gpio_set_input(port, pin, GPIO_INPUT_FLOAT);
}













/* Inicializa un timer general-purpose o advanced */
void timer_init(TIM_TypeDef *TIMx, timer_mode_t mode, uint32_t period, uint16_t prescaler)
{
    /* Habilitar reloj del timer (RCC) según TIMx */
	switch((uint32_t)TIMx)
	{
		case (uint32_t)TIM1: RCC->APB2ENR |= RCC_APB2ENR_TIM1EN; break;
		case (uint32_t)TIM2: RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; break;
		case (uint32_t)TIM3: RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; break;
		case (uint32_t)TIM4: RCC->APB1ENR |= RCC_APB1ENR_TIM4EN; break;
	}

  /* Detener timer antes de configurar */
  TIMx->CR1 = 0x00000000;
  TIMx->CR2 = 0x00000000;

	/* Reloj interno */
	TIMx->SMCR = 0x00000000;

  /* Configurar prescaler */
  TIMx->PSC = prescaler;

  /* Configurar periodo (ARR) */
  TIMx->ARR = period;

  /* Configurar modo */
	switch(mode)
	{
		case TIMER_MODE_UP:
			TIMx->CR1 &= ~TIM_CR1_DIR;    // Cuenta ascendente
		break;

		case TIMER_MODE_DOWN:
			TIMx->CR1 |= TIM_CR1_DIR; // Cuenta descendente
		break;

		case TIMER_MODE_CENTER:
			TIMx->CR1 |= TIM_CR1_CMS_0 | TIM_CR1_CMS_1; // Center-aligned
		break;
	}
}

void timer_set_clock_source(TIM_TypeDef *TIMx, timer_clk_src_t src)
{
    /* Reset SMCR */
    TIMx->SMCR = 0;

		/* Select clock source */
    switch (src)
    {
        case TIMER_CLK_INTERNAL:
            TIMx->SMCR = 0;
            break;

        case TIMER_CLK_EXTERNAL_TI1:
            TIMx->SMCR |= TIM_SMCR_SMS;                 // External clock mode 1
            TIMx->SMCR |= TIM_SMCR_TS_2 | TIM_SMCR_TS_0; // TI1FP1
            break;

        case TIMER_CLK_EXTERNAL_TI2:
            TIMx->SMCR |= TIM_SMCR_SMS;
            TIMx->SMCR |= TIM_SMCR_TS_2 | TIM_SMCR_TS_1; // TI2FP2
            break;

        case TIMER_CLK_EXTERNAL_ETR:
            TIMx->SMCR |= TIM_SMCR_ECE; // Enable ETR clock
            break;

        case TIMER_CLK_TRIGGER:
            TIMx->SMCR |= TIM_SMCR_SMS_2; // Trigger mode
            TIMx->SMCR |= TIM_SMCR_TS_0;  // ITR0 (puedes cambiar)
            break;

        case TIMER_CLK_ENCODER:
            TIMx->SMCR |= TIM_SMCR_SMS_0 | TIM_SMCR_SMS_1; // Encoder mode 3
            break;

        default:
            break;
    }
}

uint32_t timer_get_clock(TIM_TypeDef *TIMx)
{
    uint32_t ppre_bits;
    uint32_t timer_clk;

    // 1. Determinar frecuencia del bus del timer
    if ( TIMx == TIM1 )
    {
    	timer_clk = rcc_get_bus_clock(RCC_BUS_APB2);
        ppre_bits = (RCC->CFGR >> 11) & 0x7;
    }
    else
    {
    	timer_clk = rcc_get_bus_clock(RCC_BUS_APB1);
        ppre_bits = (RCC->CFGR >> 8) & 0x7;
    }

    // 2. Determinar multiplicador basado en si el prescaler es 1 o mayor
    // Según RM0008:
    // 0xx: división por 1
    // >=100: división por 2,4,8,16
    if (ppre_bits & 0x04) timer_clk <<= 1;  // Prescaler != 1, timer clock = bus clock * 2

    return timer_clk;
}

/* Inicia el timer, opcional one-shot */
void timer_start(TIM_TypeDef *TIMx, timer_run_mode_t run_mode)
{
    if (run_mode==TIMER_ONESHOT) TIMx->CR1 |= TIM_CR1_OPM; // One shot mode
    else TIMx->CR1 &= ~TIM_CR1_OPM;

    TIMx->CR1 |= TIM_CR1_CEN; // Activar timer
}

/* Detiene el timer */
void timer_stop(TIM_TypeDef *TIMx)
{
    TIMx->CR1 &= ~TIM_CR1_CEN; // Desactivar timer
}

/* Resetea el contador */
void timer_reset(TIM_TypeDef *TIMx)
{
    TIMx->CNT = 0;
}

/* Configura interrupciones del timer */
void timer_set_interrupt(TIM_TypeDef *TIMx, timer_irq_t irq, irq_state_t state)
{
    switch (irq)
    {
        case TIMER_IRQ_UPDATE:      // Update interrupt
			if (state == IRQ_ENABLE) TIMx->DIER |= TIM_DIER_UIE;
			else TIMx->DIER &= ~TIM_DIER_UIE;
		break;

        case TIMER_IRQ_CC1:          // Capture/Compare interrupt (ej. CC1)
			if (state == IRQ_ENABLE) TIMx->DIER |= TIM_DIER_CC1IE;
			else TIMx->DIER &= ~TIM_DIER_CC1IE;
		break;

		case TIMER_IRQ_CC2:          // Capture/Compare interrupt (ej. CC1)
			if (state == IRQ_ENABLE) TIMx->DIER |= TIM_DIER_CC2IE;
			else TIMx->DIER &= ~TIM_DIER_CC2IE;
		break;

		case TIMER_IRQ_CC3:          // Capture/Compare interrupt (ej. CC1)
			if (state == IRQ_ENABLE) TIMx->DIER |= TIM_DIER_CC3IE;
			else TIMx->DIER &= ~TIM_DIER_CC3IE;
		break;

		case TIMER_IRQ_CC4:          // Capture/Compare interrupt (ej. CC1)
			if (state == IRQ_ENABLE) TIMx->DIER |= TIM_DIER_CC4IE;
			else TIMx->DIER &= ~TIM_DIER_CC4IE;
		break;

        case TIMER_IRQ_TRIGGER:     // Trigger interrupt
					if (state == IRQ_ENABLE) TIMx->DIER |= TIM_DIER_TIE;
					else TIMx->DIER &= ~TIM_DIER_TIE;
        break;

        case TIMER_IRQ_BREAK:       // Break interrupt (solo TIM1/TIM8)
			if(TIMx==TIM1)
			{
				if (state == IRQ_ENABLE) TIMx->DIER |= TIM_DIER_BIE;
				else TIMx->DIER &= ~TIM_DIER_BIE;
			}
        break;

        default:
		break;
    }
}

void timer_set_trgo(TIM_TypeDef *TIMx, timer_trgo_t trgo)
{
	TIMx->CR2 &= ~(0x7 << 4);   			// limpiar MMS
    TIMx->CR2 |=  ((uint32_t)trgo << 4);  	// asignar
}


void timer_set_ic_channel(
        TIM_TypeDef *TIMx,
        timer_channel_t ch,
        timer_ic_src_t src,
        timer_ic_polarity_t polarity,
        timer_ic_div_t prescaler,
        timer_ic_filter_t filter)
{
	switch(ch)
	{
		case TIMER_CHANNEL_1:
			// Clear config
			TIMx->CCER &= ~(0x0F << 0);
			TIMx->CCMR1 &= ~(0xFF << 0);

			// Set IC config
			TIMx->CCMR1 |= ((uint32_t)src << 0);
			TIMx->CCMR1 |= (((uint32_t)prescaler & 0x3) << 2);
			TIMx->CCMR1 |= (((uint32_t)filter & 0xF) << 4);

			// Set detection edge
			if(polarity==TIMER_IC_FALLING) TIMx->CCER |= (1 << 1);

			// Enable channel
			TIMx->CCER |= (1 << 0);
		break;

		case TIMER_CHANNEL_2:
			// Clear config
			TIMx->CCER &= ~(0x0F << 4);
			TIMx->CCMR1 &= ~(0xFF << 8);

			// Set IC config
			TIMx->CCMR1 |= ((uint32_t)src << 8);
			TIMx->CCMR1 |= (((uint32_t)prescaler & 0x3) << 10);
			TIMx->CCMR1 |= (((uint32_t)filter & 0xF) << 12);

			// Set detection edge
			if(polarity==TIMER_IC_FALLING) TIMx->CCER |= (1 << 5);

			// Enable channel
			TIMx->CCER |= (1 << 4);
		break;

		case TIMER_CHANNEL_3:
			// Clear config
			TIMx->CCER &= ~(0x0F << 8);
			TIMx->CCMR2 &= ~(0xFF << 0);

			// Set IC config
			TIMx->CCMR2 |= ((uint32_t)src << 0);
			TIMx->CCMR2 |= (((uint32_t)prescaler & 0x3) << 2);
			TIMx->CCMR2 |= (((uint32_t)filter & 0xF) << 4);

			// Set detection edge
			if(polarity==TIMER_IC_FALLING) TIMx->CCER |= (1 << 9);

			// Enable channel
			TIMx->CCER |= (1 << 8);
		break;

		case TIMER_CHANNEL_4:
			// Clear config
			TIMx->CCER &= ~(0x0F << 12);
			TIMx->CCMR2 &= ~(0xFF << 8);

			// Set IC config
			TIMx->CCMR2 |= ((uint32_t)src << 8);
			TIMx->CCMR2 |= (((uint32_t)prescaler & 0x3) << 10);
			TIMx->CCMR2 |= (((uint32_t)filter & 0xF) << 12);

			// Set detection edge
			if(polarity==TIMER_IC_FALLING) TIMx->CCER |= (1 << 13);

			// Enable channel
			TIMx->CCER |= (1 << 12);
		break;

		default: return;
	}

	/* Configure gpio */
	timer_config_gpio(TIMx, ch, false);
}

void timer_set_ic_polarity(TIM_TypeDef *TIMx, timer_channel_t ch, timer_ic_polarity_t polarity)
{
	TIMx->CCER &= ~(1 << ((uint32_t)ch * 4 + 1));
	TIMx->CCER |= ((uint32_t)polarity << ((uint32_t)ch * 4 + 1));
}

uint32_t timer_get_capture_value(TIM_TypeDef *TIMx, timer_channel_t ch)
{
    switch(ch)
    {
        case TIMER_CHANNEL_1:
            return TIMx->CCR1;

        case TIMER_CHANNEL_2:
            return TIMx->CCR2;

        case TIMER_CHANNEL_3:
            return TIMx->CCR3;

        case TIMER_CHANNEL_4:
            return TIMx->CCR4;

        default:
            return 0;
    }
}

void timer_set_oc_channel(
        TIM_TypeDef *TIMx,
        timer_channel_t ch,
        timer_oc_mode_t mode,
        timer_oc_polarity_t polarity,
        uint32_t compare_value,
        bool output_enabled)
{
	uint32_t shift = ((uint32_t)ch % 2) << 3;
	volatile uint32_t *ccmr;

	/* Select CCMR register */
	if ((uint32_t)ch < 2) ccmr = &TIMx->CCMR1;
	else ccmr = &TIMx->CCMR2;

	/* Disable channel */
	TIMx->CCER &= ~(1 << ((uint32_t)ch * 4));

	/* Clear configuration */
	*ccmr &= ~(0xFF << shift);

	/* Configure Output Compare mode */
	*ccmr |= ((uint32_t)mode << (shift + 4));

	/* Enable preload (recommended for PWM) */
	*ccmr |= (1 << (shift + 3));

	/* Set compare value */
	TIMx->CR1 |= TIM_CR1_ARPE;
	switch(ch)
	{
		case TIMER_CHANNEL_1: TIMx->CCR1 = compare_value; break;
		case TIMER_CHANNEL_2: TIMx->CCR2 = compare_value; break;
		case TIMER_CHANNEL_3: TIMx->CCR3 = compare_value; break;
		case TIMER_CHANNEL_4: TIMx->CCR4 = compare_value; break;
	}

	/* Configure polarity */
	if (polarity == TIMER_OC_POLARITY_LOW)
			TIMx->CCER |= (1 << (ch * 4 + 1));
	else
			TIMx->CCER &= ~(1 << (ch * 4 + 1));

	/* Enable output if requested */
	if (output_enabled)
	{
		TIMx->CCER |= (1 << ((uint32_t)ch * 4));

		/* Configure gpio */
		timer_config_gpio(TIMx, ch, true);
	}
}

void timer_set_compare_value(TIM_TypeDef *TIMx, timer_channel_t ch, uint32_t value)
{
    switch(ch)
    {
        case TIMER_CHANNEL_1:
            TIMx->CCR1 = value;
            break;

        case TIMER_CHANNEL_2:
            TIMx->CCR2 = value;
            break;

        case TIMER_CHANNEL_3:
            TIMx->CCR3 = value;
            break;

        case TIMER_CHANNEL_4:
            TIMx->CCR4 = value;
            break;

        default:
            break;
    }
}


void timer_set_slave(TIM_TypeDef *tim,
                     timer_slave_mode_t mode,
                     timer_trgi_t trgi)
{
    // Limpiar campos SMS[2:0] y TS[2:0]
    tim->SMCR &= ~(TIM_SMCR_SMS_Msk | TIM_SMCR_TS_Msk);

    // Configurar modo esclavo y trigger
    tim->SMCR |= ((uint32_t)mode << TIM_SMCR_SMS_Pos) |
                 ((uint32_t)trgi << TIM_SMCR_TS_Pos);
}


void timer_set_encoder_mode(TIM_TypeDef *TIMx, timer_encoder_mode_t mode)
{
	/* Configurar GPIOs para CH1 y CH2 */
	timer_config_gpio(TIMx, TIMER_CHANNEL_1, false);
	timer_config_gpio(TIMx, TIMER_CHANNEL_2, false);

	/* Limpiar modo encoder */
	TIMx->SMCR &= ~TIM_SMCR_SMS;

	/* Establecer modo encoder */
	TIMx->SMCR |= (uint32_t)mode;

	/* Configurar CH1 y CH2 como entradas */
	TIMx->CCMR1 &= ~(TIM_CCMR1_CC1S | TIM_CCMR1_CC2S);
	TIMx->CCMR1 |= (1 << 0) | (1 << 8);

	/* Habilitar captura en ambos canales */
	TIMx->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E;

	/* Polaridad por defecto: rising */
	TIMx->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC2P);

	/* Resetear contador */
	TIMx->CNT = 0;
}

int32_t timer_get_encoder_count(TIM_TypeDef *TIMx)
{
    return (int32_t)TIMx->CNT;
}

typedef struct {
    TIM_TypeDef *tim;
    DMA_Channel_TypeDef *channels[5]; // UPDATE, CC1..CC4
} timer_dma_channel_map_t;

const timer_dma_channel_map_t timer_dma_table[] = {
    {TIM1, {DMA1_Channel5, DMA1_Channel2, NULL, 		 DMA1_Channel6, DMA1_Channel4}},
    {TIM2, {DMA1_Channel2, DMA1_Channel5, DMA1_Channel7, DMA1_Channel1, DMA1_Channel7}},
    {TIM3, {DMA1_Channel3, DMA1_Channel6, NULL, 		 DMA1_Channel2, DMA1_Channel3}},
    {TIM4, {DMA1_Channel7, DMA1_Channel1, DMA1_Channel4, DMA1_Channel5, NULL}}
};

DMA_Channel_TypeDef* timer_get_dma_channel(TIM_TypeDef *tim, timer_dma_event_t event)
{
    for (int i=0; i < 4; i++)
    {
        if (timer_dma_table[i].tim == tim)
        {
            return timer_dma_table[i].channels[event];
        }
    }
    return NULL;
}

void timer_set_dma(TIM_TypeDef *tim,
				   dma_dir_t dir,
                   dma_mode_t mode,
                   timer_dma_event_t event,
                   timer_dma_periph_t periph_reg,
				   void *buffer,
                   uint16_t length)
{
	// Get DMA channel
    DMA_Channel_TypeDef *dma_ch = timer_get_dma_channel(tim, event);
	if(dma_ch==NULL) return;

	// Get Peripheral's register
	volatile uint32_t *preg=NULL;
	switch(periph_reg)
	{
		case TIMER_DMA_PERIPH_ARR: preg = &tim->ARR; break;

		case TIMER_DMA_PERIPH_CCR1: preg = &tim->CCR1; break;

		case TIMER_DMA_PERIPH_CCR2: preg = &tim->CCR2; break;

		case TIMER_DMA_PERIPH_CCR3: preg = &tim->CCR3; break;

		case TIMER_DMA_PERIPH_CCR4: preg = &tim->CCR4; break;
	}

	// Initializes DMA channel
    dma_init(dma_ch,
             dir,
             DMA_SIZE_16BIT,
             DMA_SIZE_16BIT,
             true,
             false,
             mode,
             DMA_PRIORITY_HIGH);

	// Set DMA addresses
    dma_set_addresses(dma_ch, preg, buffer, length);

	// Enable Timer DMA mode
    tim->DIER |= 1 << (8 + (uint32_t)event);

	// Enable DMA channel
    dma_start(dma_ch);
}


/*
 **************************************************************
 * ANALOG TO DIGITAL CONVERTER PERIPHERAL
 **************************************************************
 */
 void adc_init(ADC_TypeDef *ADCx, adc_mode_t mode, adc_trigger_t trig, adc_align_t align)
{
	/*  Habilitar reloj ADC  */
	if (ADCx == ADC1) RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	else if (ADCx == ADC2) RCC->APB2ENR |= RCC_APB2ENR_ADC2EN;

	/* ADC clock = PCLK2 / 6  (12 MHz máx) */
	//RCC->CFGR &= ~RCC_CFGR_ADCPRE;
	//RCC->CFGR |= RCC_CFGR_ADCPRE_DIV6;

	/*  Encender ADC  */
	ADCx->CR2 |= ADC_CR2_ADON;

	/*  Calibración  */
	ADCx->CR2 |= ADC_CR2_RSTCAL;
	while(ADCx->CR2 & ADC_CR2_RSTCAL);
	ADCx->CR2 |= ADC_CR2_CAL;
	while(ADCx->CR2 & ADC_CR2_CAL);

	/*  Apagar ADC  */
	ADCx->CR2 &= ~ADC_CR2_ADON;

	/*  Reset configuración  */
	ADCx->CR1 = 0;
	ADCx->CR2 = 0;

	/*  Modo  */
	ADCx->CR1 &= ~ADC_CR1_SCAN;
	ADCx->CR2 &= ~ADC_CR2_CONT;
	switch(mode)
	{
		case ADC_MODE_SINGLE:
			/* Nada que activar */
			break;

		case ADC_MODE_CONTINUOUS:
			ADCx->CR2 |= ADC_CR2_CONT;
		break;

		case ADC_MODE_SCAN_SINGLE:
			ADCx->CR1 |= ADC_CR1_SCAN;
		break;

		case ADC_MODE_SCAN_CONTINUOUS:
			ADCx->CR1 |= ADC_CR1_SCAN;
			ADCx->CR2 |= ADC_CR2_CONT;
		break;

		default:
		break;
	}

	/*  Alineación  */
	if (align == ADC_ALIGN_LEFT)
			ADCx->CR2 |= ADC_CR2_ALIGN;
	else
			ADCx->CR2 &= ~ADC_CR2_ALIGN;

	/*  Sampling time (todos los canales)  */
	ADCx->SMPR1 = 0;
	ADCx->SMPR2 = 0;

	/*  Trigger  */
	ADCx->CR2 |= 1 << 20; // Allows external trigger
	ADCx->CR2 &= ~(0x07<<17); // Clear EXTSEL
	ADCx->CR2 |= (uint32_t)trig << 17; // Set EXTSEL
}

void adc_set_channel(ADC_TypeDef *ADCx, adc_channel_t channel, adc_sample_t samp)
{
	// Longitud = 1 conversión
	ADCx->SQR1 &= ~(0xF << 20); // L = 0, es 1 conversión

	// Canal en primera posición
	ADCx->SQR3 = channel;

	// Sample time
	if ((uint32_t)channel <= 9)
	{
		// SMPR2: canales 0–9
		uint32_t shift = (uint32_t)channel * 3;

		ADCx->SMPR2 &= ~(0x7 << shift);      // limpiar
		ADCx->SMPR2 |=  (samp << shift);     // asignar
	}
	else
	{
		// SMPR1: canales 10–17
		uint32_t shift = ((uint32_t)channel - 10) * 3;

		ADCx->SMPR1 &= ~(0x7 << shift);      // limpiar
		ADCx->SMPR1 |=  (samp << shift);     // asignar
	}

	// Enable INT_TEMP_SENS if needed
	ADCx->CR2 &= ~(1 << 23);
	if(channel >= ADC_CHANNEL_TSENSE) ADCx->CR2 |= (1 << 23);
}

void adc_set_sequence(ADC_TypeDef *ADCx, const adc_channel_t *channels, const adc_sample_t *samples, uint8_t len)
{
	if(len == 0 || len > 16) return;

	// Limpiar registros de secuencia
	ADCx->SQR1 = 0;
	ADCx->SQR2 = 0;
	ADCx->SQR3 = 0;

	// Configurar longitud (L = len - 1)
	ADCx->SQR1 |= (len - 1) << 20;

	for(int i = 0; i < len; i++)
	{
		adc_channel_t ch = channels[i];

		// -----------------------------
		// 1. Configurar secuencia (SQRx)
		// -----------------------------
		if(i < 6) ADCx->SQR3 |= ch << (i * 5);
		else if(i < 12) ADCx->SQR2 |= ch << ((i - 6) * 5);
		else ADCx->SQR1 |= ch << ((i - 12) * 5);

		// -----------------------------
		// 2. Configurar sample time (SMPRx)
		// -----------------------------
		adc_sample_t samp = samples[i];
		if(ch <= 9)
		{
			uint32_t shift = ch * 3;

			ADCx->SMPR2 &= ~(0x7 << shift);   // limpiar
			ADCx->SMPR2 |=  (samp << shift);  // asignar
		}
		else
		{
			uint32_t shift = (ch - 10) * 3;

			ADCx->SMPR1 &= ~(0x7 << shift);   // limpiar
			ADCx->SMPR1 |=  (samp << shift);  // asignar
		}

		// Enable INT_TEMP_SENS if needed
		ADCx->CR2 &= ~(1 << 23);
		if((uint32_t)ch >= ADC_CHANNEL_TSENSE) ADCx->CR2 |= (1 << 23);
	}
}

uint16_t adc_read(ADC_TypeDef *ADCx)
{
    // Start conversion
    ADCx->CR2 |= ADC_CR2_SWSTART;

    // Esperar fin de conversion
    while(!(ADCx->SR & ADC_SR_EOC));

    return ADCx->DR;
}

void adc_read_sequence(ADC_TypeDef *ADCx, uint16_t *buffer)
{
    uint8_t len = ((ADCx->SQR1 >> 20) & 0xF) + 1;

    // Start
    ADCx->CR2 |= ADC_CR2_SWSTART;

    for(int i = 0; i < len; i++)
    {
        while(!(ADCx->SR & ADC_SR_EOC));
        buffer[i] = ADCx->DR;
    }
}

/*  Iniciar conversión  */
void adc_start(ADC_TypeDef *ADCx)
{
    ADCx->CR2 |= ADC_CR2_ADON;
}

/*  Detener ADC  */
void adc_stop(ADC_TypeDef *ADCx)
{
    ADCx->CR2 &= ~ADC_CR2_ADON;
}


/*  Configurar interrupciones  */
void adc_set_interrupt(ADC_TypeDef *ADCx, adc_irq_t irq, irq_state_t state)
{
    switch(irq)
    {
        case ADC_IRQ_EOC:   /* End of conversion */
            if (state == IRQ_ENABLE)
                ADCx->CR1 |= ADC_CR1_EOCIE;
            else
                ADCx->CR1 &= ~ADC_CR1_EOCIE;
        break;

        case ADC_IRQ_EOS:   /* End of sequence */
            if (state == IRQ_ENABLE)
                ADCx->CR1 |= ADC_CR1_EOSIE;
            else
                ADCx->CR1 &= ~ADC_CR1_EOSIE;
        break;

        case ADC_IRQ_AWD:   /* Analog watchdog */
            if (state == IRQ_ENABLE)
                ADCx->CR1 |= ADC_CR1_AWDIE;
            else
                ADCx->CR1 &= ~ADC_CR1_AWDIE;
        break;

        default:
        break;
    }
}


void adc_set_dma(ADC_TypeDef *adc, uint16_t *buffer, uint16_t length,
                 dma_mode_t mode, adc_dma_event_t dma_event)
{
    DMA_Channel_TypeDef *dma_ch = (adc == ADC1) ? DMA1_Channel1 : DMA1_Channel1; // ADC2 también DMA1_Channel1 en F103

    dma_init(dma_ch,
             DMA_DIR_PERIPH_TO_MEM,
             DMA_SIZE_16BIT,
             DMA_SIZE_16BIT,
             false,  // Periph no incrementa
             true,   // Memoria incrementa
             mode,
             DMA_PRIORITY_HIGH);

    dma_set_addresses(dma_ch, &adc->DR, buffer, length);
    dma_set_interrupt(dma_ch, DMA_IRQ_TC, true);
    dma_set_interrupt(dma_ch, DMA_IRQ_HT, true);

    adc->CR2 |= ADC_CR2_DMA; // Habilitar solicitud DMA

    dma_start(dma_ch);
}







/*
 **************************************************************
 * UNIVERSAL SYNCHRONOUS/ASYNCHRONOUS RECEIVER TRANSMITTER
 **************************************************************
 */
void uart_init(USART_TypeDef *USARTx,
               uint32_t baudrate,
               uart_mode_t mode,
               uart_parity_t parity,
               uart_stopbits_t stopbits)
{
	/* Habilitar clock y gpio */
	if (USARTx == USART1) {
		rcc_clock_enable(RCC_USART1);
		rcc_clock_enable(RCC_GPIOA);

		// PA9  -> TX (AF PP)
		// PA10 -> RX (Input floating)
		gpio_set_output(GPIOA, GPIO_PIN_9, GPIO_OUTPUT_AF_PP, GPIO_SPEED_50MHZ);
		gpio_set_input(GPIOA, GPIO_PIN_10, GPIO_INPUT_FLOAT);

	} else if (USARTx == USART2) {
		rcc_clock_enable(RCC_USART2);
		rcc_clock_enable(RCC_GPIOA);

		// PA2 -> TX
		// PA3 -> RX
		gpio_set_output(GPIOA, GPIO_PIN_2, GPIO_OUTPUT_AF_PP, GPIO_SPEED_50MHZ);
		gpio_set_input(GPIOA, GPIO_PIN_3, GPIO_INPUT_FLOAT);

	} else if (USARTx == USART3) {
		rcc_clock_enable(RCC_USART3);
		rcc_clock_enable(RCC_GPIOB);

		// PB10 -> TX
		// PB11 -> RX
		gpio_set_output(GPIOB, GPIO_PIN_10, GPIO_OUTPUT_AF_PP, GPIO_SPEED_50MHZ);
		gpio_set_input(GPIOB, GPIO_PIN_11, GPIO_INPUT_FLOAT);
	}
	else {
		return;
	}

    /* Deshabilitar USART */
    USARTx->CR1 &= ~USART_CR1_UE;

    /* Configurar baudrate */
    uint32_t pclk;
    if (USARTx == USART1) pclk = rcc_get_bus_clock(RCC_BUS_APB2);
    else pclk = rcc_get_bus_clock(RCC_BUS_APB1);
    USARTx->BRR = (pclk + (baudrate / 2)) / baudrate;

    /* Configurar modo */
    USARTx->CR1 &= ~(USART_CR1_TE | USART_CR1_RE);
    if (mode == UART_MODE_TX) USARTx->CR1 |= USART_CR1_TE;
    else if (mode == UART_MODE_RX) USARTx->CR1 |= USART_CR1_RE;
    else USARTx->CR1 |= (USART_CR1_TE | USART_CR1_RE);

    /* Paridad */
    USARTx->CR1 &= ~(USART_CR1_PCE | USART_CR1_PS);

    if (parity == UART_PARITY_EVEN)
        USARTx->CR1 |= USART_CR1_PCE;
    else if (parity == UART_PARITY_ODD)
        USARTx->CR1 |= (USART_CR1_PCE | USART_CR1_PS);

    /* Stop bits */
    USARTx->CR2 &= ~(USART_CR2_STOP);

    switch (stopbits)
    {
        case UART_STOP_1:   USARTx->CR2 |= (0x0 << 12); break;
        case UART_STOP_0_5: USARTx->CR2 |= (0x1 << 12); break;
        case UART_STOP_2:   USARTx->CR2 |= (0x2 << 12); break;
        case UART_STOP_1_5: USARTx->CR2 |= (0x3 << 12); break;
    }

    /* Habilitar USART */
    USARTx->CR1 |= USART_CR1_UE;
}


void uart_write_byte(USART_TypeDef *USARTx, uint8_t data)
{
    while (!(USARTx->SR & USART_SR_TXE));
    USARTx->DR = data;
}

uint8_t uart_read_byte(USART_TypeDef *USARTx)
{
    while (!(USARTx->SR & USART_SR_RXNE));
    return (uint8_t)(USARTx->DR);
}

void uart_write(USART_TypeDef *USARTx, const uint8_t *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        uart_write_byte(USARTx, data[i]);
    }
}

void uart_read(USART_TypeDef *USARTx, uint8_t *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        data[i] = uart_read_byte(USARTx);
    }
}

void uart_set_interrupt(USART_TypeDef *USARTx,
                        uart_irq_t irq,
                        irq_state_t state)
{
    uint32_t bit = 0;

    switch (irq)
    {
        case UART_IRQ_TX:  	bit = USART_CR1_TXEIE; break;
        case UART_IRQ_TC:   bit = USART_CR1_TCIE; break;
        case UART_IRQ_RX: 	bit = USART_CR1_RXNEIE; break;
        case UART_IRQ_IDLE: bit = USART_CR1_IDLEIE; break;
        case UART_IRQ_PE:   bit = USART_CR1_PEIE; break;
        case UART_IRQ_ERR:  bit = USART_CR3_EIE; break;
    }

    if (irq == UART_IRQ_ERR)
    {
        if (state == IRQ_ENABLE)
            USARTx->CR3 |= bit;
        else
            USARTx->CR3 &= ~bit;
    }
    else
    {
        if (state == IRQ_ENABLE)
            USARTx->CR1 |= bit;
        else
            USARTx->CR1 &= ~bit;
    }
}


DMA_Channel_TypeDef* uart_get_dma_channel(USART_TypeDef *USARTx,
                                                 uart_dma_event_t event)
{
    if (USARTx == USART1)
        return (event == UART_DMA_TX) ? DMA1_Channel4 : DMA1_Channel5;

    if (USARTx == USART2)
        return (event == UART_DMA_TX) ? DMA1_Channel7 : DMA1_Channel6;

    if (USARTx == USART3)
        return (event == UART_DMA_TX) ? DMA1_Channel2 : DMA1_Channel3;

    return 0;
}

void uart_set_dma(USART_TypeDef *USARTx,
                  uart_dma_event_t event,
                  void *buffer,
                  uint16_t length)
{
    DMA_Channel_TypeDef *ch = uart_get_dma_channel(USARTx, event);

    if (!ch) return;

    /* Configuración básica DMA */
    dma_init(
        ch,
        (event == UART_DMA_TX) ? DMA_DIR_MEM_TO_PERIPH : DMA_DIR_PERIPH_TO_MEM,
        DMA_SIZE_8BIT,
        DMA_SIZE_8BIT,
        true,
        false,
        DMA_MODE_CIRCULAR,
        DMA_PRIORITY_HIGH
    );

    dma_set_addresses(
        ch,
        (void*)&USARTx->DR,
        buffer,
        length
    );

    /* Habilitar DMA en USART */
    if (event == UART_DMA_TX)
        USARTx->CR3 |= USART_CR3_DMAT;
    else
        USARTx->CR3 |= USART_CR3_DMAR;

    dma_start(ch);
}
