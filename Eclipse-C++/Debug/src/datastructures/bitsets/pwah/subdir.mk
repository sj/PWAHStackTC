################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/datastructures/bitsets/pwah/PWAHBitSet.cpp \
../src/datastructures/bitsets/pwah/PWAHBitSetIterator.cpp 

OBJS += \
./src/datastructures/bitsets/pwah/PWAHBitSet.o \
./src/datastructures/bitsets/pwah/PWAHBitSetIterator.o 

CPP_DEPS += \
./src/datastructures/bitsets/pwah/PWAHBitSet.d \
./src/datastructures/bitsets/pwah/PWAHBitSetIterator.d 


# Each subdirectory must supply rules for building sources it contributes
src/datastructures/bitsets/pwah/%.o: ../src/datastructures/bitsets/pwah/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	$(GPP) -DDEBUGGING_STATEMENT=$(DEBUGGING_STATEMENT) -O0 -g3 -p -Wall -c -fmessage-length=0 $(GPP_EXTRA_DEBUG_OPTIONS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


