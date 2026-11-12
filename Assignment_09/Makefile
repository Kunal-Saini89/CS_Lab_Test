CXX = g++
CXXFLAGS = -fopenmp -O2 -Wall

TARGETS = task1 task2 task3

all: $(TARGETS)

task1: task1_mergesort.cpp
	$(CXX) $(CXXFLAGS) task1_mergesort.cpp -o task1

task2: task2_schedule.cpp
	$(CXX) $(CXXFLAGS) task2_schedule.cpp -o task2

task3: task3_evenodd.cpp
	$(CXX) $(CXXFLAGS) task3_evenodd.cpp -o task3

1: task1
	./task1

2: task2
	./task2

3: task3
	./task3

clean:
	rm -f $(TARGETS)

