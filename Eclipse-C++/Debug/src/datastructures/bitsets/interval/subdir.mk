################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/datastructures/bitsets/interval/IntervalBitSet.cpp \
../src/datastructures/bitsets/interval/IntervalBitSetIterator.cpp 

OBJS += \
./src/datastructures/bitsets/interval/IntervalBitSet.o \
./src/datastructures/bitsets/interval/IntervalBitSetIterator.o 

CPP_DEPS += \
./src/datastructures/bitsets/interval/IntervalBitSet.d \
./src/datastructures/bitsets/interval/IntervalBitSetIterator.d 


# Each subdirectory must supply rules for building sources it contributes
src/datastructures/bitsets/interval/%.o: ../src/datastructures/bitsets/interval/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	$(GPP) -DDEBUGGING_STATEMENT=$(DEBUGGING_STATEMENT) -O0 -g3 -p -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


