################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/datastructures/DynamicStack.cpp \
../src/datastructures/Graph.cpp 

OBJS += \
./src/datastructures/DynamicStack.o \
./src/datastructures/Graph.o 

CPP_DEPS += \
./src/datastructures/DynamicStack.d \
./src/datastructures/Graph.d 


# Each subdirectory must supply rules for building sources it contributes
src/datastructures/%.o: ../src/datastructures/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DDEBUGGING_STATEMENT=$(DEBUGGING_STATEMENT) -O0 -g3 -p -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


