#include <iostream>
#include <vector>
#include <chrono>
#include <omp.h>

using namespace std;

void vec_add_serial(const vector<double>& a, const vector<double>& b, vector<double>& c, unsigned long size)
{
    for(unsigned long i = 0; i < size; i++)
    {
        c[i] = a[i] + b[i];
    }
}

void vec_add_parallel(const vector<double>& a, const vector<double>& b, vector<double>& c, unsigned long size)
{
    #pragma omp parallel for
    for(unsigned long i = 0; i < size; i++)
    {
        c[i] = a[i] + b[i];
    }
}

void vec_sum_parallel(const vector<double>& a , unsigned long size)
{
    long sum = 0
    #pragma omp for reduction(+ : sum);
    for(unsigned long i = 0; i < size; i++)
    {
        sum += a[i];
    }
}

void vec_sum_serail(const vector<double>& a , unsigned long size)
{
    long sum = 0
    for(unsigned long i = 0; i < size; i++)
    {
        sum += a[i];
    }
}

bool verify_res(const vector<double>& a , const vector<double>& b , unsigned long size)
{
    for(unsigned long i = 0; i < size; i++)
    {
        if(a[i] != b[i]) return false;
    }
    return true;
}

void runvectadd()
{
    unsigned long n = 1e8;
    vector<double> a(n , 2.0);
    vector<double> b(n , 3.0);
    vector<double> pc(n , 0);
    vector<double> sc(n , 0);

    double sstart = omp_get_wtime();

    vec_add_serial(a , b , sc , n);

    double send = omp_get_wtime();
    cout << "Serial Time : " << (osend - osstart) * 1000 << " ms" << endl;

    double sstart = omp_get_wtime();

    vec_add_parallel(a , b , pc , n);

    double end = omp_get_wtime();
    
    cout << "Chrono time takesn : " << duration.count() << " ms" << endl;
    cout << "omp time taken : " << (end - start) * 1000 << " ms" << endl;

    cout << "Are answer same : " << (verify_res(pc , sc , n) ? "Yes" : "No") << endl;
}

int main() 
{
    runvectadd();
    return 0;
}