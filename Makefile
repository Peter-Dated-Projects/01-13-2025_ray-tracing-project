# Makefile that, on every `make` call:
# 1. Builds the project (compiling main.cpp into an executable in build/).
# 2. Cleans up the .o files immediately after linking.
# 3. Runs the resulting program.

##############################################################################
# VARIABLES
##############################################################################

CXX      := g++
CXXFLAGS := -std=c++11 -Isource

TARGET   := result
SRCS     := main.cpp
OBJS     := $(SRCS:.cpp=.o)

##############################################################################
# TARGETS
##############################################################################

# "all" target: builds, removes .o files, and runs the program in one shot.
all: $(TARGET)
	@rm -f $(OBJS)

# Create the build directory if needed, then link object files into the executable.
$(TARGET): $(OBJS)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files into object files.
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# `clean` target: removes both object files and the final executable.
.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)