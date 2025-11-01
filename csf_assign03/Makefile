CXX = g++
CXXFLAGS = -g -Wall -Wextra -pedantic -std=c++17

# Source files
SRCS = main.cpp csim.cpp
OBJS = $(SRCS:.cpp=.o)

# Header files
HEADERS = csim.h

# When submitting to Gradescope, submit all .cpp and .h files,
# as well as README.txt
FILES_TO_SUBMIT = $(SRCS) $(HEADERS) README.txt

# Executable target
csim : $(OBJS)
	$(CXX) -o $@ $^

# Rule for compiling .cpp to .o
%.o : %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Target to create a solution.zip file you can upload to Gradescope
.PHONY: solution.zip
solution.zip :
	zip -9r $@ $(FILES_TO_SUBMIT) Makefile

# Generate header file dependencies automatically
depend :
	$(CXX) $(CXXFLAGS) -MM $(SRCS) > depend.mak

depend.mak :
	touch $@

clean :
	rm -f csim *.o depend.mak solution.zip

.PHONY: clean depend

include depend.mak