CC ?= cc
CFLAGS := -std=c11 -Wall -Wextra -Werror -pedantic -ICore/Inc

.PHONY: test clean

test: build/test_smfi_waveform
	./build/test_smfi_waveform

build/test_smfi_waveform: Core/Src/smfi_waveform.c tests/test_smfi_waveform.c
	mkdir -p build
	$(CC) $(CFLAGS) $^ -lm -o $@

clean:
	rm -f build/test_smfi_waveform
