# SMFI Nucleo-F446RE single-LED DAC test

This starter firmware generates a slow unipolar sinusoidal voltage on
`PA4 / DAC_OUT1 / Arduino A2` of the NUCLEO-F446RE. It is intended for testing
one LED through the existing op-amp/MOSFET current driver.

## Default waveform

The configuration uses the physical voltage represented by the retired SMFI
settings when its 12-bit DAC used a 4.096 V reference:

```text
Legacy offset code:    144
Legacy amplitude code: 115

Nucleo output offset:    144 mV
Nucleo output amplitude: 115 mV peak
Period:                  90 seconds
DAC updates:              11,000 per period
```

The output is:

```text
V_DAC(t) = 144 mV + 115 mV * sin(2*pi*t/90 s)
V_DAC minimum = 29 mV
V_DAC maximum = 259 mV
```

The values 144 and 115 were originally dimensionless DAC codes. They were
also numerically equal to millivolts because 4.096 V / 4096 = 1 mV. This
firmware specifies physical millivolts and converts them to the Nucleo's
approximately 3.3 V DAC scale.

## First-test wiring

```text
NUCLEO PA4/A2 --- 1.1 kOhm ---+--- current-driver command input
                               |
                             0.22 uF
                               |
NUCLEO GND -------------------+--- current-driver signal ground

Current-limited supply --- LED --- 30 Ohm --- current-driver power path
```

Before connecting the LED driver:

1. Connect an oscilloscope between PA4 and Nucleo GND.
2. Confirm a 90-second sine ranging approximately from 29 to 259 mV.
3. Set the LED bench supply current limit to 30 mA or lower.
4. Connect only one LED and its 30-ohm resistor.
5. Measure the current-driver sense voltage as well as the LED current.

If the current driver has a 10-ohm sense resistor and unity command scaling,
the requested current will range approximately from 2.9 to 25.9 mA.

## STM32CubeIDE project setup

Create a NUCLEO-F446RE project with STM32CubeIDE. Retain the generated startup
code, linker script and STM32 HAL drivers. Configure or verify:

- system clock: 180 MHz;
- APB1 peripheral clock: 45 MHz;
- APB1 timer clock: 90 MHz;
- PA4: `DAC_OUT1` with output buffer enabled;
- TIM6 update interrupt enabled;
- PA5: GPIO output for the onboard status LED.

Copy the repository's `Core/Inc` and `Core/Src` files into the corresponding
generated project directories. Link the ARM math library; STM32CubeIDE normally
does this through `-lm`.

## Exact 11,000-point timing

Ninety seconds cannot be divided into 11,000 equal integer-microsecond
intervals. The timer alternates between 8,181 and 8,182 microseconds with an
error accumulator. Across 11,000 intervals, the total is exactly 90,000,000
microseconds.

This implementation calculates each sine value when required. At only about
122 updates per second, the F446RE has ample processing time. A future version
can use DMA after the basic electrical behaviour is confirmed.

## Test the waveform calculations on a PC

```bash
make test
```

Expected result:

```text
PASS: 11000 samples, 29 to 259 mV, 90000000 us period
```

## Planned next stages

1. Verify the first LED and current-driver response.
2. Add serial commands for offset, amplitude and period.
3. Add `DAC_OUT2` for the second bank.
4. Add Raspberry Pi start and synchronisation messages.
5. Add camera timestamps and phase reconstruction.
