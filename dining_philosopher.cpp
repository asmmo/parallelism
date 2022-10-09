#include <chrono>
#include <iostream>
#include <mutex>
#include <random>
#include <semaphore>
#include <thread>

constexpr const size_t N = 5;  // number of philosophers
enum class State {
    THINKING = 0,  // philosopher is THINKING
    HUNGRY = 1,    // philosopher is trying to get forks
    EATING = 2,    // philosopher is EATING
};

size_t inline left(
    size_t i) {  // number of i'both_forks_available left neighbor
    return (i + N - 1) % N;
}

size_t inline right(
    size_t i) {  // number of i'both_forks_available right neighbor
    return (i + 1) % N;
}

State state[N];  // array to keep track of everyone'both_forks_available state
std::mutex critical_region_mtx;  // mutual exclusion for critical regions
std::binary_semaphore both_forks_available[N]{
    std::binary_semaphore{0}, std::binary_semaphore{0},
    std::binary_semaphore{0}, std::binary_semaphore{0},
    std::binary_semaphore{0}};
// one semaphore per philosopher
std::mutex output_mtx;  // for synchronized cout

size_t my_rand(size_t min, size_t max) {
    static std::mt19937 rnd(std::time(nullptr));
    return std::uniform_int_distribution<>(min, max)(rnd);
}

void test(size_t i) {  // i: philosopher number, from 0 to N-1
    if (state[i] == State::HUNGRY && state[left(i)] != State::EATING &&
        state[right(i)] != State::EATING) {
        state[i] = State::EATING;
        both_forks_available[i].release();
    }
}

void think(size_t i) {
    size_t duration = my_rand(400, 800);
    {
        std::lock_guard<std::mutex> lk(output_mtx);
        std::cout << i << " thinks " << duration << "ms\n";
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(duration));
}

void take_forks(size_t i) {  // i: philosopher number, from 0 to N-1
    {
        std::lock_guard<std::mutex> lk{
            critical_region_mtx};  // enter critical region
        state[i] =
            State::HUNGRY;  // record fact that philosopher i is State::HUNGRY
        {
            std::lock_guard<std::mutex> lk(output_mtx);
            std::cout << "\t\t" << i << " is State::HUNGRY\n";
        }
        test(i);                        // try to acquire 2 forks
    }                                   // exit critical region
    both_forks_available[i].acquire();  // block if forks were not acquired
}

void eat(size_t i) {
    size_t duration = my_rand(400, 800);
    {
        std::lock_guard<std::mutex> lk(output_mtx);
        std::cout << "\t\t\t\t" << i << " eats " << duration << "ms\n";
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(duration));
}

void put_forks(size_t i) {  // i: philosopher number, from 0 to N-1
    std::lock_guard<std::mutex> lk{
        critical_region_mtx};    // enter critical region
    state[i] = State::THINKING;  // philosopher has finished State::EATING
    test(left(i));               // see if left neighbor can now eat
    test(right(i));              // see if right neighbor can now eat
                                 // exit critical region
}

void philosopher(size_t i) {  // i: philosopher number, from 0 to N-1
    while (true) {            // repeat forever
        think(i);             // philosopher is State::THINKING
        take_forks(i);        // acquire two forks or block
        eat(i);               // yum-yum, spaghetti
        put_forks(i);         // put both forks back on table
    }
}

int main() {
    std::cout << "dp_14\n";

    std::jthread t0([&] { philosopher(0); });
    std::jthread t1([&] { philosopher(1); });
    std::jthread t2([&] { philosopher(2); });
    std::jthread t3([&] { philosopher(3); });
    std::jthread t4([&] { philosopher(4); });
}
