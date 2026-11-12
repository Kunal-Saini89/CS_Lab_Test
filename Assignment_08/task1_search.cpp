#include <iostream>
#include <vector>
#include <omp.h>

using namespace std;

long search_serial(const vector<int>& a, unsigned long size, int target)
{
    for(unsigned long i = 0; i < size; i++)
    {
        if(a[i] == target)
        {
            return i;
        }
    }
    return -1;
}

long search_parallel(const vector<int>& a, unsigned long size, int target)
{
    long found_idx = -1;

    #pragma omp parallel for
    for(unsigned long i = 0; i < size; i++)
    {
        if(found_idx != -1 && (long)i > found_idx)
        {
            continue;
        }

        if(a[i] == target)
        {
            #pragma omp critical
            {
                if(found_idx == -1 || (long)i < found_idx)
                {
                    found_idx = i;
                }
            }
        }
    }

    return found_idx;
}

bool verify_res(long a, long b)
{
    return a == b;
}

void runsearch()
{
    unsigned long n = 1e7;
    vector<int> a(n, 10);
    int target = 999;
    a[n - 500] = target;

    double s_start = omp_get_wtime();
    long s_idx = search_serial(a, n, target);
    double s_end = omp_get_wtime();
    cout << "Serial Time : " << (s_end - s_start) * 1000 << " ms" << endl;

    double p_start = omp_get_wtime();
    long p_idx = search_parallel(a, n, target);
    double p_end = omp_get_wtime();
    cout << "omp time taken : " << (p_end - p_start) * 1000 << " ms" << endl;

    cout << "Serial Found Index : " << s_idx << endl;
    cout << "Parallel Found Index : " << p_idx << endl;
    cout << "Are answer same : " << (verify_res(s_idx, p_idx) ? "Yes" : "No") << endl;
}

int main()
{
    runsearch();
    return 0;
}
