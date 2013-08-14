################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/algorithms/PWAHStackTC.cpp \
../src/algorithms/TransitiveClosureAlgorithm.cpp 

OBJS += \
./src/algorithms/PWAHStackTC.o \
./src/algorithms/TransitiveClosureAlgorithm.o 

CPP_DEPS += \
./src/algorithms/PWAHStackTC.d \
./src/algorithms/TransitiveClosureAlgorithm.d 


# Each subdirectory must supply rules for building sources it contributes
src/algorithms/%.o: ../src/algorithms/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	$(GPP) -DDEBUGGING_STATEMENT=$(DEBUGGING_STATEMENT) -O0 -g3 -p -Wall -c -fmessage-length=0 $(GPP_EXTRA_DEBUG_OPTIONS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


