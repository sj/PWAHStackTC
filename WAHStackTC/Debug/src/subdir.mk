################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/BitSet.cpp \
../src/DynamicBitSet.cpp \
../src/DynamicStack.cpp \
../src/Graph.cpp \
../src/IntMacros.cpp \
../src/PerformanceTimer.cpp \
../src/StaticBitSet.cpp \
../src/WAHBitSet.cpp \
../src/WAHBitSetIterator.cpp \
../src/WAHBitSetTester.cpp \
../src/WAHStackTC.cpp \
../src/main.cpp 

OBJS += \
./src/BitSet.o \
./src/DynamicBitSet.o \
./src/DynamicStack.o \
./src/Graph.o \
./src/IntMacros.o \
./src/PerformanceTimer.o \
./src/StaticBitSet.o \
./src/WAHBitSet.o \
./src/WAHBitSetIterator.o \
./src/WAHBitSetTester.o \
./src/WAHStackTC.o \
./src/main.o 

CPP_DEPS += \
./src/BitSet.d \
./src/DynamicBitSet.d \
./src/DynamicStack.d \
./src/Graph.d \
./src/IntMacros.d \
./src/PerformanceTimer.d \
./src/StaticBitSet.d \
./src/WAHBitSet.d \
./src/WAHBitSetIterator.d \
./src/WAHBitSetTester.d \
./src/WAHStackTC.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

