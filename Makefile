PROG=numerical_integration
PROG2=numerical_integration2
PROG3=numerical_integration3
PROG4=sieve

num_int3: $(PROG3).cpp
	g++ -std=c++11 -Wall -pthread $(PROG3).cpp -o $(PROG3)

sieve: $(PROG4).cpp
	g++ -std=c++11 -Wall -pthread $(PROG4).cpp -o $(PROG4)

all: num_int1 num_int2 sieve bench 

num_int1: $(PROG).cpp
	g++ -std=c++11 -Wall -pthread $(PROG).cpp -o $(PROG)

num_int2: $(PROG2).cpp
	g++ -std=c++11 -Wall -pthread $(PROG2).cpp -o $(PROG2)

bench: bench1 bench2 bench3 bench4 bench5

bench1:
	g++ -std=c++11 -Wall -pthread -O3 benchmark_example1.cpp -o bench1

bench2:
	g++ -std=c++11 -Wall -pthread -O3 benchmark_example2.cpp -o bench2

bench2_d:
	g++ -std=c++11 -Wall -pthread -fsanitize=thread -g benchmark_example2.cpp -o bench2_d

bench3:
	g++ -std=c++11 -Wall -pthread -O3 benchmark_example3.cpp -o bench3

bench4:
	g++ -std=c++11 -Wall -pthread -O3 benchmark_example4.cpp -o bench4

bench5:
	g++ -std=c++11 -Wall -pthread benchmark_example5.cpp -o bench5

clean:
	$(RM) $(PROG) $(PROG2) $(PROG3) $(PROG4) bench1 bench2 bench3 bench4 bench5
