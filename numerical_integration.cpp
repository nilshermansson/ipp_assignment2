#include <iostream>
#include <thread>
#include <cstring>
#include <pthread.h>
#include <chrono>

pthread_mutex_t area_sum_mutex;
int nr_of_threads, nr_of_total_trapezes = 0;
double total_area = 0.0;

struct arg_t 
{
    double x_min;
    double x_max;
    int nr_of_trapezes;
};

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
    arg_t *arguments = (arg_t*)args;
    if(nr_of_threads <= 0 || nr_of_total_trapezes <= 0)
        return NULL;
    
    double total_local_area = 0.0;
    double delta_x = (arguments->x_max - arguments->x_min) / arguments->nr_of_trapezes;
    for(int i = 0; i < arguments->nr_of_trapezes; i++)
    {
        total_local_area += calculate_trapetzoid_area(arguments->x_min + i*delta_x, arguments->x_min + (i+1)*delta_x);
    }

    //add to global sum
    pthread_mutex_lock(&area_sum_mutex);
    total_area += total_local_area;
    pthread_mutex_unlock(&area_sum_mutex);

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
        usage(argv[0]);
    }
    //mutex
    pthread_mutex_init(&area_sum_mutex, NULL);

    //threads
    pthread_t thread_handle[nr_of_threads];
    arg_t *thread_args = new arg_t[nr_of_threads]; //each thread owns one argument index
    int leftover_trapezes = nr_of_total_trapezes % nr_of_threads;
    double delta_x = (1.0 - 0.0) / nr_of_total_trapezes;
    int nr_of_assigned_trapezes = 0;


    auto begin = std::chrono::high_resolution_clock::now();
    //auto start_time = std::chrono::system_clock::now();

    // initial load assignment and thread creation
    for(int i = 0; i < nr_of_threads; i++)
    {
        thread_args[i].nr_of_trapezes = nr_of_total_trapezes / nr_of_threads;
        if(i < leftover_trapezes)
            thread_args[i].nr_of_trapezes++;

        thread_args[i].x_min = nr_of_assigned_trapezes * delta_x;
        thread_args[i].x_max = thread_args[i].x_min + thread_args[i].nr_of_trapezes * delta_x;

        pthread_create(&thread_handle[i], NULL, calculate_partial_area, &thread_args[i]);
        nr_of_assigned_trapezes += thread_args[i].nr_of_trapezes;
    }

    //wait for all threads to finish
    for(int i = 0; i < nr_of_threads; i++)
    {
        pthread_join(thread_handle[i], NULL);
    }
    auto end = std::chrono::high_resolution_clock().now();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);

    pthread_mutex_destroy(&area_sum_mutex);
    std::cout << "Result: " << total_area << std::endl;
    std::cout << "Execution time(seconds): " << elapsed.count() * 1e-9 << std::endl;

    return 0;
}