#include "smfi_waveform.h"

#include <math.h>

#define TWO_PI_F 6.28318530717958647692f

bool smfi_waveform_config_is_valid(const SmfiWaveformConfig *config)
{
    if (config == 0 || config->period_us < SMFI_WAVEFORM_POINTS) {
        return false;
    }
    if (config->amplitude_mv > config->offset_mv) {
        return false;
    }
    if ((uint32_t)config->offset_mv + config->amplitude_mv >
        SMFI_DAC_REFERENCE_MV) {
        return false;
    }
    return true;
}

void smfi_waveform_start(const SmfiWaveformConfig *config,
                         SmfiWaveformState *state)
{
    state->sample_index = 0U;
    state->base_interval_us = config->period_us / SMFI_WAVEFORM_POINTS;
    state->interval_remainder = config->period_us % SMFI_WAVEFORM_POINTS;
    state->timing_error = 0U;
}

uint16_t smfi_waveform_sample_mv(const SmfiWaveformConfig *config,
                                 uint32_t sample_index)
{
    float phase;
    float voltage_mv;

    sample_index %= SMFI_WAVEFORM_POINTS;
    phase = TWO_PI_F * (float)sample_index / (float)SMFI_WAVEFORM_POINTS;
    voltage_mv = (float)config->offset_mv +
                 (float)config->amplitude_mv * sinf(phase);

    if (voltage_mv <= 0.0f) {
        return 0U;
    }
    if (voltage_mv >= (float)SMFI_DAC_REFERENCE_MV) {
        return SMFI_DAC_REFERENCE_MV;
    }
    return (uint16_t)(voltage_mv + 0.5f);
}

uint16_t smfi_waveform_sample_dac(const SmfiWaveformConfig *config,
                                  uint32_t sample_index)
{
    uint32_t voltage_mv = smfi_waveform_sample_mv(config, sample_index);
    uint32_t numerator = voltage_mv * SMFI_DAC_MAX_CODE;

    return (uint16_t)((numerator + SMFI_DAC_REFERENCE_MV / 2U) /
                      SMFI_DAC_REFERENCE_MV);
}

uint32_t smfi_waveform_next_interval_us(SmfiWaveformState *state)
{
    uint32_t interval_us = state->base_interval_us;

    state->timing_error += state->interval_remainder;
    if (state->timing_error >= SMFI_WAVEFORM_POINTS) {
        state->timing_error -= SMFI_WAVEFORM_POINTS;
        interval_us++;
    }
    return interval_us;
}

bool smfi_waveform_advance(SmfiWaveformState *state)
{
    state->sample_index++;
    if (state->sample_index == SMFI_WAVEFORM_POINTS) {
        state->sample_index = 0U;
        return true;
    }
    return false;
}
