################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/util/PerformanceTimer.cpp 

OBJS += \
./src/util/PerformanceTimer.o 

CPP_DEPS += \
./src/util/PerformanceTimer.d 


# Each subdirectory must supply rules for building sources it contributes
src/util/%.o: ../src/util/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	$(GPP) -DDEBUGGING_STATEMENT=$(DEBUGGING_STATEMENT) -O0 -g3 -p -Wall -c -fmessage-length=0 $(GPP_EXTRA_DEBUG_OPTIONS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


