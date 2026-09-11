#include "smfi_waveform.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    const SmfiWaveformConfig config = {
        .period_us = 90000000U,
        .offset_mv = 144U,
        .amplitude_mv = 115U
    };
    SmfiWaveformState state;
    uint16_t minimum_mv = UINT16_MAX;
    uint16_t maximum_mv = 0U;
    uint64_t total_period_us = 0U;
    uint32_t sample;

    assert(smfi_waveform_config_is_valid(&config));
    smfi_waveform_start(&config, &state);

    for (sample = 0U; sample < SMFI_WAVEFORM_POINTS; sample++) {
        uint16_t voltage_mv = smfi_waveform_sample_mv(&config, sample);

        if (voltage_mv < minimum_mv) {
            minimum_mv = voltage_mv;
        }
        if (voltage_mv > maximum_mv) {
            maximum_mv = voltage_mv;
        }

        total_period_us += smfi_waveform_next_interval_us(&state);
    }

    assert(minimum_mv == 29U);
    assert(maximum_mv == 259U);
    assert(total_period_us == config.period_us);
    assert(smfi_waveform_sample_dac(&config, 8250U) == 36U);
    assert(smfi_waveform_sample_dac(&config, 2750U) == 321U);

    printf("PASS: %u samples, %u to %u mV, %llu us period\n",
           SMFI_WAVEFORM_POINTS,
           minimum_mv,
           maximum_mv,
           (unsigned long long)total_period_us);
    return 0;
}
