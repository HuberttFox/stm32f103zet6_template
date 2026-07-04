#include "SYSTEM/sys/sys.h"

void sys_nvic_set_vector_table(uint32_t baseaddr, uint32_t offset)
{
    SCB->VTOR = baseaddr | (offset & (uint32_t)0xFFFFFE00);
}

void sys_wfi_set(void)
{
    __ASM volatile("wfi");
}

void sys_intx_disable(void)
{
    __ASM volatile("cpsid i");
}

void sys_intx_enable(void)
{
    __ASM volatile("cpsie i");
}

void sys_msr_msp(uint32_t addr)
{
    __set_MSP(addr);
}

void sys_standby(void)
{
    __HAL_RCC_PWR_CLK_ENABLE();
    SET_BIT(PWR->CR, PWR_CR_PDDS);
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    __WFI();
}

void sys_soft_reset(void)
{
    NVIC_SystemReset();
}

void sys_stm32_clock_init(uint32_t plln)
{
    HAL_StatusTypeDef ret = HAL_ERROR;
    RCC_OscInitTypeDef rcc_oscinitstructure;
    RCC_ClkInitTypeDef rcc_clkinitstructure;

    rcc_oscinitstructure.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    rcc_oscinitstructure.HSEState = RCC_HSE_ON;
    rcc_oscinitstructure.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    rcc_oscinitstructure.PLL.PLLState = RCC_PLL_ON;
    rcc_oscinitstructure.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    rcc_oscinitstructure.PLL.PLLMUL = plln;
    ret = HAL_RCC_OscConfig(&rcc_oscinitstructure);

    if (ret != HAL_OK)
    {
        while (1);
    }

    rcc_clkinitstructure.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    rcc_clkinitstructure.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    rcc_clkinitstructure.AHBCLKDivider = RCC_SYSCLK_DIV1;
    rcc_clkinitstructure.APB1CLKDivider = RCC_HCLK_DIV2;
    rcc_clkinitstructure.APB2CLKDivider = RCC_HCLK_DIV1;
    ret = HAL_RCC_ClockConfig(&rcc_clkinitstructure, FLASH_LATENCY_2);

    if (ret != HAL_OK)
    {
        while (1);
    }
}
