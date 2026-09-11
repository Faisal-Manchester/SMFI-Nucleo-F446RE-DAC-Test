#ifndef SMFI_WAVEFORM_H
#define SMFI_WAVEFORM_H

#include <stdbool.h>
#include <stdint.h>

#define SMFI_WAVEFORM_POINTS 11000U
#define SMFI_DAC_MAX_CODE     4095U
#define SMFI_DAC_REFERENCE_MV 3300U

typedef struct {
    uint32_t period_us;
    uint16_t offset_mv;
    uint16_t amplitude_mv;
} SmfiWaveformConfig;

typedef struct {
    uint32_t sample_index;
    uint32_t base_interval_us;
    uint32_t interval_remainder;
    uint32_t timing_error;
} SmfiWaveformState;

bool smfi_waveform_config_is_valid(const SmfiWaveformConfig *config);
void smfi_waveform_start(const SmfiWaveformConfig *config,
                         SmfiWaveformState *state);
uint16_t smfi_waveform_sample_mv(const SmfiWaveformConfig *config,
                                 uint32_t sample_index);
uint16_t smfi_waveform_sample_dac(const SmfiWaveformConfig *config,
                                  uint32_t sample_index);
uint32_t smfi_waveform_next_interval_us(SmfiWaveformState *state);
bool smfi_waveform_advance(SmfiWaveformState *state);

#endif
