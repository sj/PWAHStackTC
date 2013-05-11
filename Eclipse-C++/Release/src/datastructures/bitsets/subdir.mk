################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/datastructures/bitsets/BitSet.cpp \
../src/datastructures/bitsets/BitSetIterator.cpp \
../src/datastructures/bitsets/DynamicBitSet.cpp \
../src/datastructures/bitsets/LongBitMacros.cpp \
../src/datastructures/bitsets/StaticBitSet.cpp 

OBJS += \
./src/datastructures/bitsets/BitSet.o \
./src/datastructures/bitsets/BitSetIterator.o \
./src/datastructures/bitsets/DynamicBitSet.o \
./src/datastructures/bitsets/LongBitMacros.o \
./src/datastructures/bitsets/StaticBitSet.o 

CPP_DEPS += \
./src/datastructures/bitsets/BitSet.d \
./src/datastructures/bitsets/BitSetIterator.d \
./src/datastructures/bitsets/DynamicBitSet.d \
./src/datastructures/bitsets/LongBitMacros.d \
./src/datastructures/bitsets/StaticBitSet.d 


# Each subdirectory must supply rules for building sources it contributes
src/datastructures/bitsets/%.o: ../src/datastructures/bitsets/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


