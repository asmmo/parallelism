#include<vector>
#include<iostream>
#include<future>
#include<iomanip>
template <typename T>
void parallelForColsTrans(std::vector<std::vector<T>>& A, size_t fCol, size_t lCol, size_t i, size_t n)
{
    if (fCol == lCol)
    {
        T temp = A[i][fCol+n];
        A[i][fCol+n] = A[i+n][fCol];
        A[i+n][fCol] = temp;

        return;
    }

    std::async(parallelForColsTrans<T>, std::ref(A), fCol, (fCol + lCol) / 2, i, n);
    std::async(parallelForColsTrans<T>, std::ref(A), (fCol + lCol) / 2 + 1, lCol, i, n);

}


template <typename T>
void parallelForRowsTrans(std::vector<std::vector<T>>& A, size_t fRow, size_t lRow, size_t fCol, size_t lCol, size_t n)
{
    if (fRow == lRow) {
        parallelForColsTrans(A, fCol, lCol, fRow, n);
        return;
    }

    std::async(parallelForRowsTrans<T>, std::ref(A), fRow, (fRow+lRow) / 2, fCol, lCol, n);
    std::async(parallelForRowsTrans<T>, std::ref(A), (fRow + lRow) / 2 + 1, lRow, fCol, lCol, n);
}

template <typename T>
void pMatTransposeRecursive(std::vector<std::vector<T>>& A, size_t firstRow, size_t lastRow, size_t firstColumn, size_t lastColumn)
{
    if (firstRow == lastRow)    return;

    auto t1 =std::async(pMatTransposeRecursive<T>, std::ref(A), firstRow, (firstRow +lastRow)/2, firstColumn, (firstColumn+lastColumn)/2);
    auto t2 =std::async(pMatTransposeRecursive<T>, std::ref(A), (firstRow +lastRow)/2+1, lastRow, firstColumn, (firstColumn+lastColumn)/2);
    auto t3 =std::async(pMatTransposeRecursive<T>, std::ref(A), firstRow, (firstRow +lastRow)/2, (firstColumn+lastColumn)/2+1, lastColumn);
    pMatTransposeRecursive<T>(std::ref(A), (firstRow +lastRow)/2+1, lastRow, (firstColumn+lastColumn)/2+1, lastColumn);
    t1.get();
    t2.get();
    t3.get();
    size_t n = (lastColumn-firstColumn+1)/2;
    parallelForRowsTrans<T>(std::ref(A), firstRow, firstRow+n-1, firstColumn, firstColumn+n-1, n);

}

template <typename T>
void transpose(std::vector<std::vector<T>>& A){
    pMatTransposeRecursive( A, 0, A.size()-1, 0, A[0].size()-1);
}
int main(){

    std::vector<std::vector<int>> A = {{1,2,3,4,7,7,7,8}, {5,6,7,8,4,5,1,1}, {9,10,5,5,11,12,4,79}, {7,8,13,14,15,16,44,6}, {13,-14,7,-7,15,-16,-44,6}, {13,-14,105,106,404,6,9,9}, {13,-14,7,-7,15,-16,-44,6}, {13,-14,105,106,404,6,9,9}};
    transpose(A);
    for(auto & el:A){
        for(auto& ele:el) std::cout << ele << std::setw(4) ;
        std::cout << "\n";

    }
}
