#include "main.h"
#include "smfi_waveform.h"

DAC_HandleTypeDef hdac;
TIM_HandleTypeDef htim6;

static const SmfiWaveformConfig waveform = {
    .period_us = 90000000U,
    .offset_mv = 144U,
    .amplitude_mv = 115U
};

static SmfiWaveformState waveform_state;

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void DAC_Init(void);
static void TIM6_Init(void);
static void SetNextTimerInterval(void);
static void Error_Handler(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    DAC_Init();
    TIM6_Init();

    if (!smfi_waveform_config_is_valid(&waveform)) {
        Error_Handler();
    }

    smfi_waveform_start(&waveform, &waveform_state);

    if (HAL_DAC_Start(&hdac, DAC_CHANNEL_1) != HAL_OK) {
        Error_Handler();
    }

    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R,
                     smfi_waveform_sample_dac(&waveform, 0U));
    SetNextTimerInterval();

    if (HAL_TIM_Base_Start_IT(&htim6) != HAL_OK) {
        Error_Handler();
    }

    while (1) {
        __WFI();
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *timer)
{
    bool completed_cycle;

    if (timer->Instance != TIM6) {
        return;
    }

    completed_cycle = smfi_waveform_advance(&waveform_state);

    HAL_DAC_SetValue(
        &hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R,
        smfi_waveform_sample_dac(&waveform, waveform_state.sample_index));

    SetNextTimerInterval();

    if (completed_cycle) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    }
}

static void SetNextTimerInterval(void)
{
    uint32_t interval_us = smfi_waveform_next_interval_us(&waveform_state);

    /* TIM6 counts at 1 MHz: one counter tick equals one microsecond. */
    __HAL_TIM_SET_AUTORELOAD(&htim6, interval_us - 1U);
}

static void DAC_Init(void)
{
    DAC_ChannelConfTypeDef channel = {0};

    hdac.Instance = DAC;
    if (HAL_DAC_Init(&hdac) != HAL_OK) {
        Error_Handler();
    }

    channel.DAC_Trigger = DAC_TRIGGER_NONE;
    channel.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
    if (HAL_DAC_ConfigChannel(&hdac, &channel, DAC_CHANNEL_1) != HAL_OK) {
        Error_Handler();
    }
}

static void TIM6_Init(void)
{
    htim6.Instance = TIM6;
    htim6.Init.Prescaler = 89U;       /* 90 MHz / 90 = 1 MHz. */
    htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim6.Init.Period = 8181U - 1U;  /* Replaced dynamically. */
    htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&htim6) != HAL_OK) {
        Error_Handler();
    }
}

static void GPIO_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

    gpio.Pin = GPIO_PIN_5;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &gpio);
}

static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef oscillator = {0};
    RCC_ClkInitTypeDef clocks = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    oscillator.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    oscillator.HSIState = RCC_HSI_ON;
    oscillator.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    oscillator.PLL.PLLState = RCC_PLL_ON;
    oscillator.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    oscillator.PLL.PLLM = 8U;
    oscillator.PLL.PLLN = 180U;
    oscillator.PLL.PLLP = RCC_PLLP_DIV2;
    oscillator.PLL.PLLQ = 2U;

    if (HAL_RCC_OscConfig(&oscillator) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
        Error_Handler();
    }

    clocks.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                       RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clocks.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clocks.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clocks.APB1CLKDivider = RCC_HCLK_DIV4;
    clocks.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&clocks, FLASH_LATENCY_5) != HAL_OK) {
        Error_Handler();
    }
}

static void Error_Handler(void)
{
    __disable_irq();
    while (1) {
    }
}
