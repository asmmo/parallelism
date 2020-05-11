//parallelizing data addition
#include <iostream>
#include<thread>
#include <chrono>
#include <vector>
#include <numeric>

using namespace std::chrono_literals;
using namespace std::string_literals;

template<typename It, typename Callable>
void accumulate_block(It first, It last, typename It::value_type & result, Callable binaryOp)
{
    result = std::accumulate(first, last, result, binaryOp);
}

template<typename It, typename Callable>
typename It::value_type  parallel_accumulate(It first, It last, typename It::value_type init, Callable binaryOp)
{
    unsigned long const length = std::distance(first, last);
    if(!length) return init;
    unsigned long const min_per_thread = 25;
    unsigned long const max_threads =  1 + (length - 1)/min_per_thread;
    unsigned long const hardware_parallel_threads = std::thread::hardware_concurrency();
    unsigned long const num_threads =
            std::min(hardware_parallel_threads != 0 ? hardware_parallel_threads : 2, max_threads);
    unsigned long const block_size=length / num_threads;
    std::vector< typename It::value_type > results(num_threads);
    std::vector<std::thread> threads(num_threads-1);//the original thread takes one
    It block_start = first;
    for(unsigned long i{}; i < (num_threads - 1); ++i)
    {
        It block_end = block_start;
        std::advance(block_end, block_size);
        threads[i]=std::thread(
                accumulate_block<It, Callable>,
                block_start, block_end, std::ref(results[i]), binaryOp);
        block_start=block_end;
    }

    accumulate_block<It>(block_start, last, results[num_threads-1], binaryOp);//the rest

    for(auto& entry: threads) entry.join();

    return std::accumulate(results.begin(), results.end(), init, binaryOp);
}

int main(){
    std::vector<int> vec(1000000000);
    for(int i{}; i < 1000000000; ++i) vec[0] = i;
    std::cout<<"The size of the vector processed " << sizeof(vec)+sizeof(vec[0])*vec.capacity() << " Bytes\n";

    auto start1 = std::chrono::system_clock::now();
    std::cout << "Ordinary one result is " << accumulate(vec.begin(), vec.end(), 50) << "\t";
    auto duration1 = std::chrono::system_clock::now() - start1;
    std::cout<< "It took " << std::chrono::duration_cast<std::chrono::duration<double>>(duration1).count() << " seconds\n";

    auto start2 = std::chrono::system_clock::now();
    std::cout <<"\nparallelized one result is " << parallel_accumulate(vec.begin(), vec.end(), 50, [](const int & i, const int & j){return i+j;}) << "\t";
    auto duration2 = std::chrono::system_clock::now() - start2;
    std::cout<< "It took " << std::chrono::duration_cast<std::chrono::duration<double>>(duration2).count() << " seconds";


}
