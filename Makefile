.PHONY: check

check:
	cmp -s protocol/include/devlab_protocol.h libraries/arduino/DevLabDDP/src/DevLabDDPProtocol.h
	@echo "DDP firmware contract and Arduino library are synchronized"
