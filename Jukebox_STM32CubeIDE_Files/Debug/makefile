################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Middlewares/Third_Party/FatFs/src/option/subdir.mk
-include Middlewares/Third_Party/FatFs/src/subdir.mk
-include FATFS/Target/subdir.mk
-include FATFS/App/subdir.mk
-include Drivers/STM32F4xx_HAL_Driver/Src/subdir.mk
-include Drivers/CMSIS_DSP/Source/TransformFunctions/subdir.mk
-include Drivers/CMSIS_DSP/Source/SupportFunctions/subdir.mk
-include Drivers/CMSIS_DSP/Source/StatisticsFunctions/subdir.mk
-include Drivers/CMSIS_DSP/Source/InterpolationFunctions/subdir.mk
-include Drivers/CMSIS_DSP/Source/FastMathFunctions/subdir.mk
-include Drivers/CMSIS_DSP/Source/DistanceFunctions/subdir.mk
-include Drivers/CMSIS_DSP/Source/ControllerFunctions/subdir.mk
-include Drivers/CMSIS_DSP/Source/ComplexMathFunctions/subdir.mk
-include Drivers/CMSIS_DSP/Source/CommonTables/subdir.mk
-include Drivers/CMSIS_DSP/Source/BasicMathFunctions/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := JukeboxProject
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
JukeboxProject.elf \

MAP_FILES += \
JukeboxProject.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
JukeboxProject.list \


# All Target
all: main-build

# Main-build Target
main-build: JukeboxProject.elf secondary-outputs

# Tool invocations
JukeboxProject.elf JukeboxProject.map: $(OBJS) $(USER_OBJS) C:\Users\Johnathan\ Guzman\Documents\GitHub\STM32-RFID-Based-Minecraft-Jukebox\Jukebox_STM32CubeIDE_Files\STM32F411CEUX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "JukeboxProject.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"C:\Users\Johnathan Guzman\Documents\GitHub\STM32-RFID-Based-Minecraft-Jukebox\Jukebox_STM32CubeIDE_Files\STM32F411CEUX_FLASH.ld" --specs=nosys.specs -Wl,-Map="JukeboxProject.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

JukeboxProject.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "JukeboxProject.list"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) JukeboxProject.elf JukeboxProject.list JukeboxProject.map default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
