################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../tests/BitSetTester.cpp \
../tests/GraphTestInfo.cpp \
../tests/PWAHStackTCTest.cpp 

OBJS += \
./tests/BitSetTester.o \
./tests/GraphTestInfo.o \
./tests/PWAHStackTCTest.o 

CPP_DEPS += \
./tests/BitSetTester.d \
./tests/GraphTestInfo.d \
./tests/PWAHStackTCTest.d 


# Each subdirectory must supply rules for building sources it contributes
tests/%.o: ../tests/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DDEBUGGING_STATEMENT=$(DEBUGGING_STATEMENT) -O0 -g3 -p -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


