# 交叉编译器前缀
PREFIX ?= arm-none-eabi-

CC = $(PREFIX)gcc
LD = $(PREFIX)ld
OBJCOPY = $(PREFIX)objcopy
SIZE = $(PREFIX)size

# 目标文件
TARGET = app

# 源文件
SRCS = app.c \
       flash_storage.c \
       stm32_hal_msp.c \
       stm32_it.c \
       main.c \
       freertos.c \
       $(wildcard *.c)

# 头文件目录
INCLUDES = -I./Inc \
           -I./ \
           -I./Middlewares/Third_Party/FreeRTOS/Source/include \
           -I./Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS \
           -I./Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 \
           -I./Drivers/CMSIS/Include \
           -I./Drivers/CMSIS/Device/ST/STM32F1xx/Include \
           -I./Drivers/STM32F1xx_HAL_Driver/Inc

# 预处理器定义
DEFS = -DUSE_HAL_DRIVER -DSTM32F103xB

# 编译选项
CFLAGS = -mcpu=cortex-m3 -mthumb -mlittle-endian -mfix-cortex-m3-ldrd
CFLAGS += -ffunction-sections -fdata-sections -fsingle-precision-constant -Wdouble-promotion
CFLAGS += -Os -flto
CFLAGS += $(INCLUDES) $(DEFS)

# 链接选项
LDFLAGS = -mcpu=cortex-m3 -mthumb -Wl,-Map=$(TARGET).map,--cref -Wl,--gc-sections

# 库文件
LIBS = -lc -lm -lnosys

# 目标
.PHONY: all clean size

all: $(TARGET).bin size

# 编译规则
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 链接
$(TARGET).elf: $(SRCS:.c=.o)
	$(CC) $(LDFLAGS) $^ -o $@ $(LIBS)

# 生成二进制文件
%.bin: %.elf
	$(OBJCOPY) -O binary $< $@
	@echo "生成 $@ 完成!"

# 显示尺寸信息
size: $(TARGET).elf
	$(SIZE) $<

# 清理
clean:
	rm -f *.o *.elf *.bin *.map

# 测试目标
test:
	@echo "运行单元测试..."
	gcc -o test_runner test_app.c -DTESTING
	./test_runner
	@echo "测试结果保存到 test_results.log"

# 集成测试
integration_test:
	@echo "运行集成测试..."
	gcc -o integration_test_runner test_integration.c -DTESTING
	./integration_test_runner
	@echo "集成测试完成!"