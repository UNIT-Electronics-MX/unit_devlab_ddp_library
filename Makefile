.PHONY: check

check:
	cmp src/DevLabDDPProtocol.h src/devlab_protocol.h
	printf '%s\n' '#include "devlab_protocol.h"' '_Static_assert(DDP_DEVICE_NEOPIXEL_PA2 != DDP_DEVICE_BUZZER_PA2, "duplicate device IDs");' '_Static_assert(CMD_BUZZER_PLAY == 0xA9U && CMD_NEO_SEQUENCE == 0xBCU, "unexpected actuator command");' | $(CC) -std=c11 -Wall -Wextra -Werror -fsyntax-only -x c -Isrc -
