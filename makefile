-include ../makefile.init

RM := rm -rf

CC=gcc
CFLAGS = -Wall -g -O3  -I"./header" -lpthread


CXX = g++
CXXFLAGS = -Wall -g -O3

export CXXFLAGS
export CFLAGS



# All Target
all: froute_test

# Tool invocations
froute_test: 
	@echo 'Building target: $@ And Linking '
		
	gcc  ripd/*.c -o rip_daemon $(CFLAGS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
