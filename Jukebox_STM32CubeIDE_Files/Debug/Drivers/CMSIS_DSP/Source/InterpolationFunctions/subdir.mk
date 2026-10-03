################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/CMSIS_DSP/Source/InterpolationFunctions/InterpolationFunctions.c \
../Drivers/CMSIS_DSP/Source/InterpolationFunctions/InterpolationFunctionsF16.c \
../Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_f16.c \
../Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_f32.c \
../Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_q15.c \
../Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_q31.c \
../Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_q7.c \
../Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_f16.c \
../Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_f32.c \
../Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_q15.c \
../Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_q31.c \
../Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_q7.c \
../Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_spline_interp_f32.c \
../Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_spline_interp_init_f32.c 

OBJS += \
./Drivers/CMSIS_DSP/Source/InterpolationFunctions/InterpolationFunctions.o \
./Drivers/CMSIS_DSP/Source/InterpolationFunctions/InterpolationFunctionsF16.o \
./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_f16.o \
./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_f32.o \
./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_q15.o \
./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_q31.o \
./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_q7.o \
./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_f16.o \
./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_f32.o \
./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_q15.o \
./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_q31.o \
./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_q7.o \
./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_spline_interp_f32.o \
./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_spline_interp_init_f32.o 

C_DEPS += \
./Drivers/CMSIS_DSP/Source/InterpolationFunctions/InterpolationFunctions.d \
./Drivers/CMSIS_DSP/Source/InterpolationFunctions/InterpolationFunctionsF16.d \
./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_f16.d \
./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_f32.d \
./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_q15.d \
./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_q31.d \
./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_q7.d \
./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_f16.d \
./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_f32.d \
./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_q15.d \
./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_q31.d \
./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_q7.d \
./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_spline_interp_f32.d \
./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_spline_interp_init_f32.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/CMSIS_DSP/Source/InterpolationFunctions/%.o Drivers/CMSIS_DSP/Source/InterpolationFunctions/%.su Drivers/CMSIS_DSP/Source/InterpolationFunctions/%.cyclo: ../Drivers/CMSIS_DSP/Source/InterpolationFunctions/%.c Drivers/CMSIS_DSP/Source/InterpolationFunctions/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../FATFS/Target -I"C:/Users/Johnathan Guzman/Documents/GitHub/STM32-RFID-Based-Minecraft-Jukebox/Jukebox_STM32CubeIDE_Files/Drivers/CMSIS_DSP/Include" -I"C:/Users/Johnathan Guzman/Documents/GitHub/STM32-RFID-Based-Minecraft-Jukebox/Jukebox_STM32CubeIDE_Files/Drivers/CMSIS_DSP/PrivateInclude" -I../FATFS/App -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FatFs/src -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-CMSIS_DSP-2f-Source-2f-InterpolationFunctions

clean-Drivers-2f-CMSIS_DSP-2f-Source-2f-InterpolationFunctions:
	-$(RM) ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/InterpolationFunctions.cyclo ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/InterpolationFunctions.d ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/InterpolationFunctions.o ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/InterpolationFunctions.su ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/InterpolationFunctionsF16.cyclo ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/InterpolationFunctionsF16.d ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/InterpolationFunctionsF16.o ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/InterpolationFunctionsF16.su ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_f16.cyclo ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_f16.d ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_f16.o ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_f16.su ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_f32.cyclo ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_f32.d ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_f32.o ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_f32.su ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_q15.cyclo ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_q15.d ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_q15.o ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_q15.su ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_q31.cyclo ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_q31.d ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_q31.o ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_q31.su ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_q7.cyclo ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_q7.d ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_q7.o ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_bilinear_interp_q7.su ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_f16.cyclo ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_f16.d ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_f16.o ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_f16.su ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_f32.cyclo ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_f32.d ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_f32.o ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_f32.su ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_q15.cyclo ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_q15.d ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_q15.o ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_q15.su ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_q31.cyclo ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_q31.d ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_q31.o ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_q31.su ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_q7.cyclo ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_q7.d ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_q7.o ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_linear_interp_q7.su ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_spline_interp_f32.cyclo ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_spline_interp_f32.d ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_spline_interp_f32.o ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_spline_interp_f32.su ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_spline_interp_init_f32.cyclo ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_spline_interp_init_f32.d ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_spline_interp_init_f32.o ./Drivers/CMSIS_DSP/Source/InterpolationFunctions/arm_spline_interp_init_f32.su

.PHONY: clean-Drivers-2f-CMSIS_DSP-2f-Source-2f-InterpolationFunctions

