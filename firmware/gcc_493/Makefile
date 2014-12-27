RESULT = firmware

#COMPILE GLAGS
CFLAGS = -g -O2 -Wall -mlittle-endian -mthumb -mcpu=cortex-m3 -msoft-float
CFLAGS += -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER -DNDEBUG

#INCLUDES
CFLAGS += -I.
CFLAGS += -L.
CFLAGS += -I./../src
CFLAGS += -I./../STM32_USB-FS-Device_Driver/inc
CFLAGS += -I./../STM32F10x_StdPeriph_Driver/inc
CFLAGS += -I./../CMIS/Device/STM32F10x/Include

#LINKER FLAGS
LFLAGS = -nostartfiles -Tstm32_link.ld

#STM32_10x_LIB
ST_STDLIB_DIR = ./../STM32F10x_StdPeriph_Driver/src/
ST_STDLIB = $(ST_STDLIB_DIR)misc.c
ST_STDLIB += $(ST_STDLIB_DIR)stm32f10x_exti.c
ST_STDLIB += $(ST_STDLIB_DIR)stm32f10x_gpio.c
ST_STDLIB += $(ST_STDLIB_DIR)stm32f10x_pwr.c
ST_STDLIB += $(ST_STDLIB_DIR)stm32f10x_rcc.c
ST_STDLIB += $(ST_STDLIB_DIR)stm32f10x_spi.c
ST_STDLIB += $(ST_STDLIB_DIR)stm32f10x_tim.c

#STM32 USB LIB
ST_USBLIB_DIR = ./../STM32_USB-FS-Device_Driver/src/
ST_USBLIB = $(ST_USBLIB_DIR)usb_core.c
ST_USBLIB += $(ST_USBLIB_DIR)usb_init.c
ST_USBLIB += $(ST_USBLIB_DIR)usb_int.c
ST_USBLIB += $(ST_USBLIB_DIR)usb_mem.c
ST_USBLIB += $(ST_USBLIB_DIR)usb_regs.c
ST_USBLIB += $(ST_USBLIB_DIR)usb_sil.c

#PROJECT FILES
PROJECT_DIR  = ./../src/
STARTUP = startup_stm32f10x_md.s

PROJECT = $(PROJECT_DIR)hw_config.c
PROJECT += $(PROJECT_DIR)main.c
PROJECT += $(PROJECT_DIR)stm32_it.c
PROJECT += $(PROJECT_DIR)system_stm32f10x.c

PROJECT += $(PROJECT_DIR)usb_istr.c
PROJECT += $(PROJECT_DIR)usb_prop.c
PROJECT += $(PROJECT_DIR)usb_pwr.c
#CHANGE LCD DRIVER IF NEEDS ( erc12864_4.c, hd44780.c, lcd_st7735.c )
PROJECT += $(PROJECT_DIR)nokia_lcd.c

PROJECT += $(PROJECT_DIR)io_input.c
PROJECT += $(PROJECT_DIR)kbd_driver.c
PROJECT += $(PROJECT_DIR)spi_master.c
PROJECT += $(PROJECT_DIR)string_utils.c
PROJECT += $(PROJECT_DIR)timer_drv.c

#OBJECTS
OBJ_DIR = obj
OBJ_STARTUP = $(STARTUP:%.s=%.o)
OBJ = $(ST_STDLIB:%.c=%.o)
OBJ += $(ST_USBLIB:%.c=%.o)
OBJ += $(PROJECT:%.c=%.o)

DEP = $(OBJ:%.o=%.d)


GCC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size


all: create_dir build_firmware

build_firmware: $(OBJ_STARTUP) $(OBJ)
	$(GCC) $(LFLAGS) -o $(RESULT).elf $(addprefix $(OBJ_DIR)/, $(notdir $^))
	$(OBJCOPY) -O binary $(RESULT).elf $(RESULT).bin
	$(OBJCOPY) -O ihex $(RESULT).elf $(RESULT).hex
	$(SIZE) $(RESULT).hex
create_dir:
	mkdir -p $(OBJ_DIR)
	
clean:
	rm -f *.elf
	rm -f *.bin
	rm -f *.hex
	rm -f $(OBJ_DIR)/*.*
	
%.o: %.c
	$(GCC) $(CFLAGS) -c $< -o $(OBJ_DIR)/$(notdir $@)

%.o: %.s
	$(GCC) $(CFLAGS) -c $< -o $(OBJ_DIR)/$(notdir $@)
