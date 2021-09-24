#include <iostream>
#include <thread>
#include <cstring>
#include <pthread.h>
#include <chrono>
#include <string>

int nr_of_threads, nr_of_total_trapezes = 0;
double total_area = 0.0;

struct arg_t {
    int thread_id;
    double *output;
};

double delta_x = 0.0;

double the_function(double x)
{
    return 4.0 / (1.0 + x*x);
}

void usage(char *program_name)
{
    std::cout << "Usage:" << std::endl;
    std::cout << program_name << " <T> <N>" << std::endl;
    std::cout << "where T is the number of threads, N is the number of trapezes." << std::endl;
    exit(1);
}

double calculate_trapetzoid_area(double x1, double x2)
{
    return ((the_function(x1) + the_function(x2)) / 2.0) * (x2 - x1);
}

void *calculate_partial_area(void *args)
{   
    arg_t *thread_args = (arg_t*) args;
    int thread_id = thread_args->thread_id;

    double summed_area = 0.0;

    for(int i = thread_id; i < nr_of_total_trapezes; i += nr_of_threads)
    {
        summed_area += calculate_trapetzoid_area(i * delta_x, (i+1) * delta_x);
    }

    //while(assigned_trapeze < nr_of_total_trapezes)
    //{
    //    summed_area += calculate_trapetzoid_area(assigned_trapeze * delta_x, (assigned_trapeze+1) * delta_x);
    //    assigned_trapeze = current_trapeze.fetch_add(1);
    //}
    
    *(thread_args->output) = summed_area;

    return NULL;
}

int main(int argc, char *argv[])
{
    if(argc == 2 && std::strcmp(argv[1], "-h") == 0)
    {
        usage(argv[0]);
    }
    else if(argc != 3)
    {
        std::cout << "Invalid arguments! Use " << argv[0] << " -h for help." << std::endl;
        return 0;
    }
    try
    {
        nr_of_threads = std::stoi(argv[1]);
        nr_of_total_trapezes = std::stoi(argv[2]);
    }
    catch(std::exception e)
    {
        std::cout << e.what() << std::endl;
        return 1;
    }

    // thread setup
    pthread_t thread_handle[nr_of_threads];
    delta_x = 1.0 / nr_of_total_trapezes;
    double thread_outputs[nr_of_threads]; // each thread owns one index
    arg_t thread_args[nr_of_threads];

    auto start_time = std::chrono::high_resolution_clock::now();

    // initial load assignment and thread creation
    for(int i = 0; i < nr_of_threads; i++)
    {
        thread_args[i].thread_id = i;
        thread_args[i].output = &thread_outputs[i];
        pthread_create(&thread_handle[i], NULL, calculate_partial_area, (void*)&thread_args[i]);
    }

    // wait for all threads to finish
    for(int i = 0; i < nr_of_threads; i++)
    {
        pthread_join(thread_handle[i], NULL);
        total_area += thread_outputs[i];
    }

    auto end_time = std::chrono::high_resolution_clock().now();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);

    std::cout << "Result: " << total_area << std::endl;
    std::cout << "Execution time(seconds): " << elapsed.count() * 1e-9 << std::endl;

    return 0;
}