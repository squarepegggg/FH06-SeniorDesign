# Makefile for Edge Impulse classification demo

CXX = clang++
CXXFLAGS = -std=c++11 -I. -O2
LDFLAGS = 

# Directories
SDK_DIR = edge-impulse-sdk
PORTING_DIR = $(SDK_DIR)/porting/posix
MODEL_DIR = tflite-model

# Main sources
MAIN = main.cpp
PORTING = $(PORTING_DIR)/ei_classifier_porting.cpp $(PORTING_DIR)/debug_log.cpp
MODEL = $(MODEL_DIR)/tflite_learn_810907_3_compiled.cpp

# Find all other sources
EI_SOURCES = $(shell find $(SDK_DIR) -name "*.cpp" ! -path "*/porting/*" -o -name "*.cc" ! -path "*/porting/*" 2>/dev/null)
TFLITE_COMMON = $(SDK_DIR)/tensorflow/lite/c/common.c

# All sources
SOURCES = $(MAIN) $(PORTING) $(MODEL) $(EI_SOURCES) $(TFLITE_COMMON)

# Generate object file names (in build/ directory)
BUILD_DIR = build
OBJS = $(SOURCES:%=$(BUILD_DIR)/%.o)

# Target
TARGET = app

# Default target
all: $(TARGET)
	@echo "Build complete: ./$(TARGET)"

# Link
$(TARGET): $(OBJS)
	@echo "Linking..."
	@mkdir -p $(dir $@)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

# Compile C++ files
$(BUILD_DIR)/%.cpp.o: %.cpp
	@echo "Compiling $<..."
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile .cc files
$(BUILD_DIR)/%.cc.o: %.cc
	@echo "Compiling $<..."
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile C files
$(BUILD_DIR)/%.c.o: %.c
	@echo "Compiling $<..."
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Nordic flashing configuration
NRFJPROG = nrfjprog
FLASH_FILE ?= $(TARGET).hex
DEVICE_SNR ?= 

# Flash commands for nRF52840 DK
flash-nrf52840: $(FLASH_FILE)
	@echo "Flashing nRF52840 DK..."
	@which $(NRFJPROG) > /dev/null || (echo "Error: nrfjprog not found. Install nRF Command Line Tools." && exit 1)
	$(NRFJPROG) --family NRF52 $(if $(DEVICE_SNR),--snr $(DEVICE_SNR),) --program $(FLASH_FILE) --sectorerase
	$(NRFJPROG) --family NRF52 $(if $(DEVICE_SNR),--snr $(DEVICE_SNR),) --reset
	@echo "Flash complete!"

# Flash commands for nRF52805
flash-nrf52805: $(FLASH_FILE)
	@echo "Flashing nRF52805..."
	@which $(NRFJPROG) > /dev/null || (echo "Error: nrfjprog not found. Install nRF Command Line Tools." && exit 1)
	$(NRFJPROG) --family NRF52 $(if $(DEVICE_SNR),--snr $(DEVICE_SNR),) --program $(FLASH_FILE) --sectorerase
	$(NRFJPROG) --family NRF52 $(if $(DEVICE_SNR),--snr $(DEVICE_SNR),) --reset
	@echo "Flash complete!"

# Generic flash target (uses NRFJPROG)
flash: $(FLASH_FILE)
	@echo "Flashing device..."
	@which $(NRFJPROG) > /dev/null || (echo "Error: nrfjprog not found. Install nRF Command Line Tools." && exit 1)
	$(NRFJPROG) --family NRF52 $(if $(DEVICE_SNR),--snr $(DEVICE_SNR),) --program $(FLASH_FILE) --sectorerase
	$(NRFJPROG) --family NRF52 $(if $(DEVICE_SNR),--snr $(DEVICE_SNR),) --reset
	@echo "Flash complete!"

# Erase device
erase:
	@which $(NRFJPROG) > /dev/null || (echo "Error: nrfjprog not found. Install nRF Command Line Tools." && exit 1)
	@echo "Erasing device..."
	$(NRFJPROG) --family NRF52 $(if $(DEVICE_SNR),--snr $(DEVICE_SNR),) --eraseall
	@echo "Device erased!"

# Recovery (recover device if flash is corrupted)
recover:
	@which $(NRFJPROG) > /dev/null || (echo "Error: nrfjprog not found. Install nRF Command Line Tools." && exit 1)
	@echo "Recovering device..."
	$(NRFJPROG) --family NRF52 $(if $(DEVICE_SNR),--snr $(DEVICE_SNR),) --recover
	@echo "Device recovered!"

# Clean
clean:
	rm -rf $(BUILD_DIR) $(TARGET)
	@echo "Clean complete"

# Help target
help:
	@echo "Available targets:"
	@echo "  all              - Build the application (default)"
	@echo "  clean            - Remove build artifacts"
	@echo "  flash            - Flash to connected nRF52 device (generic)"
	@echo "  flash-nrf52840   - Flash to nRF52840 DK"
	@echo "  flash-nrf52805   - Flash to nRF52805"
	@echo "  erase            - Erase the device"
	@echo "  recover          - Recover a bricked device"
	@echo ""
	@echo "Flash options:"
	@echo "  FLASH_FILE=<file> - Specify hex/bin file to flash (default: $(TARGET).hex)"
	@echo "  DEVICE_SNR=<snr>  - Specify device serial number if multiple devices connected"
	@echo ""
	@echo "Example:"
	@echo "  make flash-nrf52840 FLASH_FILE=app.hex DEVICE_SNR=12345678"

.PHONY: all clean flash flash-nrf52840 flash-nrf52805 erase recover help
