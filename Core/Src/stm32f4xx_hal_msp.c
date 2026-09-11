#include "main.h"

void HAL_MspInit(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
}

void HAL_DAC_MspInit(DAC_HandleTypeDef *dac)
{
    GPIO_InitTypeDef gpio = {0};

    if (dac->Instance != DAC) {
        return;
    }

    __HAL_RCC_DAC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* PA4 is DAC_OUT1 and Arduino connector A2 on the NUCLEO-F446RE. */
    gpio.Pin = GPIO_PIN_4;
    gpio.Mode = GPIO_MODE_ANALOG;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &gpio);
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *timer)
{
    if (timer->Instance != TIM6) {
        return;
    }

    __HAL_RCC_TIM6_CLK_ENABLE();
    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 1U, 0U);
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
}
