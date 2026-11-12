#include <iostream>
#include <vector>
#include <omp.h>

using namespace std;

long long vec_sum_serial(const vector<int>& a, unsigned long size)
{
    long long sum = 0;
    for(unsigned long i = 0; i < size; i++)
    {
        sum += a[i];
    }
    return sum;
}

long long vec_sum_parallel(const vector<int>& a, unsigned long size)
{
    long long sum = 0;
    #pragma omp parallel for reduction(+ : sum)
    for(unsigned long i = 0; i < size; i++)
    {
        sum += a[i];
    }
    return sum;
}

bool verify_res(long long a, long long b)
{
    return a == b;
}

void runvecsum()
{
    unsigned long n = 1e7;
    vector<int> a(n, 2);

    double s_start = omp_get_wtime();
    long long s_sum = vec_sum_serial(a, n);
    double s_end = omp_get_wtime();
    cout << "Serial Time : " << (s_end - s_start) * 1000 << " ms" << endl;

    double p_start = omp_get_wtime();
    long long p_sum = vec_sum_parallel(a, n);
    double p_end = omp_get_wtime();
    cout << "omp time taken : " << (p_end - p_start) * 1000 << " ms" << endl;

    cout << "Serial Sum : " << s_sum << endl;
    cout << "Parallel Sum : " << p_sum << endl;
    cout << "Are answer same : " << (verify_res(s_sum, p_sum) ? "Yes" : "No") << endl;
}

int main()
{
    runvecsum();
    return 0;
}
