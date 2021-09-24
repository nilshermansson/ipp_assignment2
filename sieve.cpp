#include <iostream>
#include <pthread.h>
#include <string>
#include <cstring>
#include <atomic>
#include <math.h>
#include <chrono>

int nr_of_threads = 0;
int n; // 'Max'
int *marked; //array of marked numbers

void calculate_primes(int number_list[], int max)
{
    int k = 2;
    while(k*k <= max)
    {
        // mark numbers between k^2 and n
        for (int i = k*k; i <= max; i++) 
        {
            if(i % k == 0)
                marked[i] = 1;
        }

        // find smallest number greater than k that is unmarked
        for (int i = k; i <= max; i++) 
        {
            if(marked[i] == 0 && i > k)
            {
                k = i;
                break;
            }
        }
    }
}

void *thread_func(void *args) 
{
    int thread_id = *(int *)args;
    
    for(int i = sqrt(n) + thread_id; i <= n; i += nr_of_threads)
    {
        for(int j = 2; j <= (int)sqrt(n); j++) 
        {
            if(marked[j] == 0 && i % j == 0)
                marked[i] = 1;
        }
    }

    return NULL;
}

void usage(char *program_name)
{
    std::cout << "Usage:" << std::endl;
    std::cout << program_name << " <T> <N>" << std::endl;
    std::cout << "where T is the number of threads, N is the maximum possive integer for which the program will find primes under." << std::endl;
    std::cout << "To print the found prime numbers, use the -p flag as the final argument." << std::endl;
    std::cout << "Example: " << program_name << " 4 10 -p" << std::endl;
    exit(1);
}

int main(int argc, char *argv[])
{
    int should_print_primes = 0;
    if(argc == 2 && std::strcmp(argv[1], "-h") == 0)
    {
        usage(argv[0]);
    }
    else if(argc == 4)
    {
        should_print_primes = std::strcmp(argv[3], "-p") == 0; //print the primes if true
    }
    else if(argc != 3)
    {
        std::cout << "Invalid arguments! Use " << argv[0] << " -h for help." << std::endl;
        return 0;
    }
    try
    {
        nr_of_threads = std::stoi(argv[1]);
        n = std::stoi(argv[2]);
    }
    catch(std::exception e)
    {
        std::cout << e.what() << std::endl;
        return 1;
    }

    //setup
    int numbers[n+1]; //index 0 left empty
    marked = new int[n+1];
    for(int i = 1; i <= n; i++)
    {
        numbers[i] = i;
        marked[i] = 0;
    }

    pthread_t thread_handle[nr_of_threads];
    int thread_id[nr_of_threads];

    //start counting here
    auto start_time = std::chrono::high_resolution_clock::now();

    // sequentially compute primes up to sqrt(n)
    calculate_primes(numbers, (int)sqrt(n));

    // dispatch threads
    for (int i = 0; i < nr_of_threads; i++) 
    {
        thread_id[i] = i + 1;
        pthread_create(&thread_handle[i], NULL, thread_func, (void*) &thread_id[i]);
    }

    // wait for threads to finish
    for (int i = 0; i < nr_of_threads; i++) 
    {
        pthread_join(thread_handle[i], NULL);
    }

    auto end_time = std::chrono::high_resolution_clock().now();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);

    if(should_print_primes)
    {
        std::cout << "Primes: ";
        for(int i = 1; i <= n; i++)
        {
            if(marked[i] == 0)
                std::cout << i << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "Execution time(s): " << elapsed.count() * 1e-9 << std::endl;

    return 0;
}