################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/datastructures/bitsets/wah/IntBitMacros.cpp \
../src/datastructures/bitsets/wah/WAHBitSet.cpp \
../src/datastructures/bitsets/wah/WAHBitSetIterator.cpp 

OBJS += \
./src/datastructures/bitsets/wah/IntBitMacros.o \
./src/datastructures/bitsets/wah/WAHBitSet.o \
./src/datastructures/bitsets/wah/WAHBitSetIterator.o 

CPP_DEPS += \
./src/datastructures/bitsets/wah/IntBitMacros.d \
./src/datastructures/bitsets/wah/WAHBitSet.d \
./src/datastructures/bitsets/wah/WAHBitSetIterator.d 


# Each subdirectory must supply rules for building sources it contributes
src/datastructures/bitsets/wah/%.o: ../src/datastructures/bitsets/wah/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -p -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


