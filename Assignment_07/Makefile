CXX = g++
CXXFLAGS = -fopenmp -O2 -Wall

TARGETS = task1 task2 task3 task4 task5

all: $(TARGETS)
main: $(TARGETS)

task1: task1_vecadd.cpp
	$(CXX) $(CXXFLAGS) task1_vecadd.cpp -o task1

task2: task2_sum.cpp
	$(CXX) $(CXXFLAGS) task2_sum.cpp -o task2

task3: task3_minmax.cpp
	$(CXX) $(CXXFLAGS) task3_minmax.cpp -o task3

task4: task4_matadd.cpp
	$(CXX) $(CXXFLAGS) task4_matadd.cpp -o task4

task5: task5_matmul.cpp
	$(CXX) $(CXXFLAGS) task5_matmul.cpp -o task5

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
