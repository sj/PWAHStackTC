################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/BitSet.cpp \
../src/BitSetIterator.cpp \
../src/BitSetTester.cpp \
../src/DynamicBitSet.cpp \
../src/DynamicStack.cpp \
../src/Graph.cpp \
../src/IntBitMacros.cpp \
../src/IntervalBitSet.cpp \
../src/IntervalBitSetIterator.cpp \
../src/LongBitMacros.cpp \
../src/PWAHBitSet.cpp \
../src/PWAHBitSetIterator.cpp \
../src/PWAHStackTC.cpp \
../src/PerformanceTimer.cpp \
../src/StaticBitSet.cpp \
../src/TransitiveClosureAlgorithm.cpp \
../src/Validator.cpp \
../src/WAHBitSet.cpp \
../src/WAHBitSetIterator.cpp \
../src/WAHBitSetTester.cpp \
../src/main.cpp 

OBJS += \
./src/BitSet.o \
./src/BitSetIterator.o \
./src/BitSetTester.o \
./src/DynamicBitSet.o \
./src/DynamicStack.o \
./src/Graph.o \
./src/IntBitMacros.o \
./src/IntervalBitSet.o \
./src/IntervalBitSetIterator.o \
./src/LongBitMacros.o \
./src/PWAHBitSet.o \
./src/PWAHBitSetIterator.o \
./src/PWAHStackTC.o \
./src/PerformanceTimer.o \
./src/StaticBitSet.o \
./src/TransitiveClosureAlgorithm.o \
./src/Validator.o \
./src/WAHBitSet.o \
./src/WAHBitSetIterator.o \
./src/WAHBitSetTester.o \
./src/main.o 

CPP_DEPS += \
./src/BitSet.d \
./src/BitSetIterator.d \
./src/BitSetTester.d \
./src/DynamicBitSet.d \
./src/DynamicStack.d \
./src/Graph.d \
./src/IntBitMacros.d \
./src/IntervalBitSet.d \
./src/IntervalBitSetIterator.d \
./src/LongBitMacros.d \
./src/PWAHBitSet.d \
./src/PWAHBitSetIterator.d \
./src/PWAHStackTC.d \
./src/PerformanceTimer.d \
./src/StaticBitSet.d \
./src/TransitiveClosureAlgorithm.d \
./src/Validator.d \
./src/WAHBitSet.d \
./src/WAHBitSetIterator.d \
./src/WAHBitSetTester.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

