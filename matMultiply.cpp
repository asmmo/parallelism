#include<iostream>
#include<vector>
#include <future>
#include <functional>
#include <iomanip>
using namespace std::chrono_literals;
template <typename T>
T auxiliary(const std::vector<std::vector<T>>& A, const std::vector<std::vector<T>>& B, size_t f, size_t l, size_t i, size_t j)
{
    if (f == l)  return A[i][f] * B[f][j];

    std::future<T> y = std::async(auxiliary<T>, std::ref(A), std::ref(B), f, (f + l) /2, i, j);
    T x = auxiliary<T>(A, B, (f + l) / 2 + 1, l, i, j);

    return x + y.get();
}

template <typename T>
void parallelForCols(const std::vector<std::vector<T>>& A, const std::vector<std::vector<T>>& B, std::vector<std::vector<T>>& C, size_t f, size_t l, size_t i)
{
    if (f == l)
    {
        C[i][f] = auxiliary(A, B, 0, A[0].size()-1, i, f);
        return;
    }


    std::future<void> thread1 = std::async(parallelForCols<T>, std::ref(A), std::ref(B), std::ref(C), f, (f + l) / 2, i);
    parallelForCols<T>(A, B, C, (f + l) / 2 + 1, l, i);

    thread1.get();
}


template <typename T>
void parallelForRows(const std::vector<std::vector<T>>& A, const std::vector<std::vector<T>>& B, std::vector<std::vector<T>>& C, size_t f, size_t l)
{
    if (f == l) {
        parallelForCols(A, B, C, 0, B[0].size()-1, f);
        return;
    }

    std::future<void> thread1 = std::async(parallelForRows<T>, std::ref(A), std::ref(B), std::ref(C), f, (f+l) / 2);
    parallelForRows<T>(A, B, C, (f + l) / 2 + 1, l);

    thread1.get();
}


template <typename T>
void parallelForRowsSetUp(std::vector<std::vector<T>>& C, size_t f, size_t l, size_t colsNo)
{
    if (f == l) {
        C[f] = std::vector<T>( colsNo);
        return;
    }

    std::future<void> thread1 = std::async(parallelForRowsSetUp<T>, std::ref(C), f, (f + l) / 2, colsNo);
    parallelForRowsSetUp<T>(C, (f + l) / 2 + 1, l, colsNo);

    thread1.get();
}


template <typename T>
std::vector<std::vector<T>> matMultiply(const std::vector<std::vector<T>>& A, const std::vector<std::vector<T>>& B)
{
    auto C = std::vector<std::vector<T>>(A.size());

    parallelForRowsSetUp(C, 0, C.size() -1, B[0].size());

    std::future<void> thread1 = std::async(parallelForRows<T>, std::ref(A), std::ref(B), std::ref(C), 0, (A.size()-1) / 2);
    parallelForRows<T>(A, B, C, (A.size() - 1) / 2 + 1, A.size()-1);

    thread1.get();

    return C;
}

int main(){
    std::vector<std::vector<int>> mat1 = {{1,2},{3,4},{1,2},{5,7}};
    std::vector<std::vector<int>> mat2 = {{1,2,3,4},{1,2,5,7}};
    auto res = matMultiply(mat1, mat2);
    for(auto& el:res){
        for(auto& ele:el) std::cout << ele << std::setw(3);
        std::cout << "\n";
    }
}
