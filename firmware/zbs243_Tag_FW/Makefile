
BUILD		?= zbs154_ssd1619

#file containing main() must be first!
SOURCES		+= src/main.c
SOURCES		+= src/powermgt.c


FLAGS += -Iboard/$(BUILD)
FLAGS += -Isoc/$(SOC)
FLAGS += -Icpu/$(CPU)
FLAGS += -Isrc/
FLAGS += -Iboard/$(BUILD)

FLAGS += -Icpu/8051

FLAGS += -mmcs51 --std-c2x --opt-code-size --peep-file cpu/8051/peep.def --fomit-frame-pointer
SOURCES += cpu/8051/asmUtil.c
CC = sdcc
FLAGS += -Isoc/zbs243

FLAGS += -DSOC_ZBS243 --xram-loc 0xe000 --xram-size 0x2000 --model-large

SOURCES += soc/zbs243/soc.c
SOURCES += soc/zbs243/wdt.c
SOURCES += soc/zbs243/sleep.c
SOURCES += soc/zbs243/spi.c
SOURCES += soc/zbs243/uart.c
SOURCES += soc/zbs243/timer.c
# SOURCES += soc/zbs243/radio.c
SOURCES += soc/zbs243/flash.c
SOURCES += soc/zbs243/temperature.c
# SOURCES += cpu/8051/random.c
# SOURCES += cpu/8051/aes.c
SOURCES += cpu/8051/printf.c
SOURCES += soc/zbs243/i2c.c
CPU = 8051
FLAGS += --code-size 0xfc00

SOC = zbs243

BARCODE = datamatrix

# 0xfc00 and not 0x10000 to leave some space for update header and updater in flash

SOURCES += cpu/$(CPU)/cpu.c
SOURCES += board/$(BUILD)/board.c
# SOURCES += board/$(BUILD)/screen.c
# SOURCES += md5.c

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
