#include <iostream>
#include <vector>
#include <chrono>
#include <omp.h>

using namespace std;

void vec_min_serail(const vector<double>& a , unsigned long size , double& min , unsigned int& min_index)
{
    double min_val = a[0];
    unsigned long index = 0;
    for(unsigned long i = 0; i < size; i++)
    {
        if(min_val < a[i])
        {
            min_val = a[i];
            index = i;
        }
    }
    min = min_val;
    min_index = index;
}

void vec_min_parallel(const vector<double>& a , unsigned long size , double& min , unsigned int& min_index)
{
    double min_val = a[0];
    unsigned long index = 0;
    #pragma omp parallel for reduction(min : min_val)
    for(unsigned long i = 0; i < size; i++)
    {
        if(min_val > a[i])
        {
            min_val = a[i];
            index = i;
        }
    }
    min = min_val;
    min_index = index;
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

    long aa = vec_min_serail(a , n);

    double osend = omp_get_wtime();
    cout << "Serial Time : " << (osend - osstart) * 1000 << " ms" << endl;

    auto start = chrono::high_resolution_clock::now();
    double ostart = omp_get_wtime();

    long bb = vec_min_parallel(a , n);

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