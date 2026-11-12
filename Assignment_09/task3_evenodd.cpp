#include <iostream>
#include <vector>
#include <omp.h>

using namespace std;

void count_serial(const vector<int>& a, unsigned long size, long& even_count, long& odd_count)
{
    long even = 0;
    long odd = 0;

    for(unsigned long i = 0; i < size; i++)
    {
        if(a[i] % 2 == 0)
        {
            even++;
        }
        else
        {
            odd++;
        }
    }

    even_count = even;
    odd_count = odd;
}

void count_parallel_reduction(const vector<int>& a, unsigned long size, long& even_count, long& odd_count)
{
    long even = 0;
    long odd = 0;

    #pragma omp parallel for reduction(+ : even, odd)
    for(unsigned long i = 0; i < size; i++)
    {
        if(a[i] % 2 == 0)
        {
            even++;
        }
        else
        {
            odd++;
        }
    }

    even_count = even;
    odd_count = odd;
}

void count_parallel_atomic(const vector<int>& a, unsigned long size, long& even_count, long& odd_count)
{
    long even = 0;
    long odd = 0;

    #pragma omp parallel for
    for(unsigned long i = 0; i < size; i++)
    {
        if(a[i] % 2 == 0)
        {
            #pragma omp atomic
            even++;
        }
        else
        {
            #pragma omp atomic
            odd++;
        }
    }

    even_count = even;
    odd_count = odd;
}

bool verify_res(long s_even, long p_even, long s_odd, long p_odd, unsigned long n)
{
    return (s_even == p_even && s_odd == p_odd && (s_even + s_odd) == (long)n);
}

void runevenodd()
{
    unsigned long n = 1e7;
    vector<int> a(n);

    for(unsigned long i = 0; i < n; i++)
    {
        a[i] = i;
    }

    long s_even = 0, s_odd = 0;
    double s_start = omp_get_wtime();
    count_serial(a, n, s_even, s_odd);
    double s_end = omp_get_wtime();
    cout << "Serial Time : " << (s_end - s_start) * 1000 << " ms" << endl;

    long r_even = 0, r_odd = 0;
    double t_start_r = omp_get_wtime();
    count_parallel_reduction(a, n, r_even, r_odd);
    double t_end_r = omp_get_wtime();
    cout << "Reduction Time : " << (t_end_r - t_start_r) * 1000 << " ms" << endl;

    long a_even = 0, a_odd = 0;
    double t_start_a = omp_get_wtime();
    count_parallel_atomic(a, n, a_even, a_odd);
    double t_end_a = omp_get_wtime();
    cout << "Atomic Time : " << (t_end_a - t_start_a) * 1000 << " ms" << endl;

    cout << "Serial Even : " << s_even << " , Odd : " << s_odd << endl;
    cout << "Reduction Even : " << r_even << " , Odd : " << r_odd << endl;
    cout << "Atomic Even : " << a_even << " , Odd : " << a_odd << endl;

    cout << "Does reduction match : " << (verify_res(s_even, r_even, s_odd, r_odd, n) ? "Yes" : "No") << endl;
    cout << "Does atomic match : " << (verify_res(s_even, a_even, s_odd, a_odd, n) ? "Yes" : "No") << endl;
}

int main()
{
    runevenodd();
    return 0;
}
