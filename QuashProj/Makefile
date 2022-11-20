CXX := g++
CPPS := $(wildcard *.cpp) 
OBJS := $(CPPS:.cpp=.o)
FLAGS := -DDEBUG

ifdef debug
Quash: $(OBJS)
	$(CXX) -O0 -g -std=c++17 -DDEBUG -o Quash $(OBJS) -lpthread

%.o: %.cpp 
	$(CXX) -O0 -g -std=c++17 -DDEBUG -MMD -MP -c -o $@ $<
else
Quash: $(OBJS)
	$(CXX) -O0 -g -std=c++17 -o Quash $(OBJS) -lpthread

%.o: %.cpp 
	$(CXX) -O0 -g -std=c++17 -MMD -MP -c -o $@ $<
endif

clean:
	rm *.o Quash
