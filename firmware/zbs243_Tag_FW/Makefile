
BUILD		?= zbs154_ssd1619

SOC = zbs243
CPU = 8051

FLAGS += -Iboard/$(BUILD)
FLAGS += -Isoc/$(SOC)
FLAGS += -Icpu/$(CPU)
FLAGS += -Isrc/
FLAGS += -mmcs51 --std-c2x --opt-code-size --peep-file cpu/$(CPU)/peep.def --fomit-frame-pointer
FLAGS += -DSOC_ZBS243 --xram-loc 0xe000 --xram-size 0x2000 --model-large
FLAGS += --code-size 0xfc00
# 0xfc00 and not 0x10000 to leave some space for update header and updater in flash

#file containing main() must be first!
# App source
SOURCES	+= src/main.c
SOURCES	+= src/powermgt.c
# SOURCES += src/md5.c

# CPU source
# SOURCES += cpu/$(CPU)/asmUtil.c
SOURCES += cpu/$(CPU)/cpu.c
SOURCES += cpu/$(CPU)/printf.c

# SOC source
SOURCES += soc/zbs243/soc.c
SOURCES += soc/zbs243/wdt.c
SOURCES += soc/zbs243/sleep.c
# SOURCES += soc/zbs243/spi.c
SOURCES += soc/zbs243/uart.c
SOURCES += soc/zbs243/timer.c
# SOURCES += soc/zbs243/radio.c
# SOURCES += soc/zbs243/flash.c
# SOURCES += soc/zbs243/temperature.c
# SOURCES += cpu/8051/random.c
# SOURCES += cpu/8051/aes.c
# SOURCES += soc/zbs243/i2c.c

# Board source
# SOURCES += board/$(BUILD)/board.c
# SOURCES += board/$(BUILD)/screen.c

CC = sdcc

OUTPUT_NAME = EPD_Thermometer_$(BUILD)
BUILD_DIR = .build
OUTPUT_DIR = .output
OBJECTS = $(patsubst %.c,$(BUILD_DIR)/%.rel,$(SOURCES))
OUTPUT = $(OUTPUT_DIR)/$(OUTPUT_NAME)

all: size #make sure this is the first target

# Put all build output in a separate directory

$(BUILD_DIR)/%.rel: %.c
	@mkdir -p $(dir $@)
	$(CC) $(FLAGS) -c $^ -o $@

$(OUTPUT).ihx: $(OBJECTS)
	@mkdir -p $(dir $(OUTPUT))
	$(CC) $(FLAGS) --out-fmt-ihx -o $(OUTPUT).ihx $(OBJECTS)

$(OUTPUT).bin: $(OUTPUT).ihx
# 	makebin -p $(OUTPUT).ihx $(OUTPUT).bin
	objcopy $^ $@ -I ihex -O binary

build: $(OUTPUT).bin

size: build
	@echo '---------- Segments ----------'
	@egrep '(ABS,CON)|(REL,CON)' $(OUTPUT).map | gawk --non-decimal-data '{dec = sprintf("%d","0x" $$2); print dec " " $$0}' | /usr/bin/sort -n -k1 | cut -f2- -d' '
	@echo '------------------------------'
	@stat -L --printf "Size of $(OUTPUT).bin: %s bytes\n" $(OUTPUT).bin

.PHONY: clean

clean:
	rm -r -f $(BUILD_DIR) $(OUTPUT_DIR)
