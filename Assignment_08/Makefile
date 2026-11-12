CXX = g++
CXXFLAGS = -fopenmp -O2 -Wall

TARGETS = task1 task2 task3 task4 task5

all: $(TARGETS)

task1: task1_search.cpp
	$(CXX) $(CXXFLAGS) task1_search.cpp -o task1

task2: task2_sort.cpp
	$(CXX) $(CXXFLAGS) task2_sort.cpp -o task2

task3: task3_histogram.cpp
	$(CXX) $(CXXFLAGS) task3_histogram.cpp -o task3

task4: task4_transpose.cpp
	$(CXX) $(CXXFLAGS) task4_transpose.cpp -o task4

task5: task5_convolution.cpp
	$(CXX) $(CXXFLAGS) task5_convolution.cpp -o task5

1: task1
	./task1

2: task2
	./task2

3: task3
	./task3

4: task4
	./task4

5: task5
	./task5

clean:
	rm -f $(TARGETS)
