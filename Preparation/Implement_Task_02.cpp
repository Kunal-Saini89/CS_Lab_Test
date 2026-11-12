#include <iostream>
#include <vector>
#include <chrono>
#include <omp.h>

using namespace std;

long vec_sum_serail(const vector<double>& a , unsigned long size)
{
    long sum = 0;
    for(unsigned long i = 0; i < size; i++)
    {
        sum += a[i];
    }
    return sum;
}

long vec_sum_parallel(const vector<double>& a , unsigned long size)
{
    long sum = 0;
    #pragma omp parallel for reduction(+ : sum)
    for(unsigned long i = 0; i < size; i++)
    {
        sum += a[i];
    }
    return sum;
}

bool verify_res(unsigned long a , unsigned long b)
{
    return a == b;
}

void runvectadd()
{
    unsigned long n = 1e5;
    vector<double> a(n , 2.0);
    vector<double> b(n , 3.0);
    vector<double> pc(n , 0);
    vector<double> sc(n , 0);

    double osstart = omp_get_wtime();

    long aa = vec_sum_serail(a , n);

    double osend = omp_get_wtime();
    cout << "Serial Time : " << (osend - osstart) * 1000 << " ms" << endl;

    auto start = chrono::high_resolution_clock::now();
    double ostart = omp_get_wtime();

    long bb = vec_sum_parallel(a , n);

    double oend = omp_get_wtime();
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration<double , milli>(end - start);
    cout << "Chrono time takesn : " << duration.count() << " ms" << endl;
    cout << "omp time taken : " << (oend - ostart) * 1000 << " ms" << endl;

    cout << "Are answer same : " << (verify_res(aa , bb) ? "Yes" : "No") << endl;
}

int main() 
{
    runvectadd();
    return 0;
}