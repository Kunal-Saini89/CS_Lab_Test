#include <iostream>
#include <vector>
#include <omp.h>

using namespace std;

void vec_minmax_serial(const vector<int>& a, unsigned long size, int& min_val, unsigned long& min_idx, int& max_val, unsigned long& max_idx)
{
    min_val = a[0];
    min_idx = 0;
    max_val = a[0];
    max_idx = 0;

    for(unsigned long i = 1; i < size; i++)
    {
        if(a[i] < min_val)
        {
            min_val = a[i];
            min_idx = i;
        }
        if(a[i] > max_val)
        {
            max_val = a[i];
            max_idx = i;
        }
    }
}

void vec_minmax_parallel(const vector<int>& a, unsigned long size, int& min_val, unsigned long& min_idx, int& max_val, unsigned long& max_idx)
{
    int local_min[64];
    unsigned long local_min_idx[64];
    int local_max[64];
    unsigned long local_max_idx[64];
    int num_threads = 1;

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        int nthreads = omp_get_num_threads();
        if(tid == 0) num_threads = nthreads;

        unsigned long start = (tid * size) / nthreads;
        unsigned long end = ((tid + 1) * size) / nthreads;

        int cur_min = a[start];
        unsigned long cur_min_idx = start;
        int cur_max = a[start];
        unsigned long cur_max_idx = start;

        for(unsigned long i = start + 1; i < end; i++)
        {
            if(a[i] < cur_min)
            {
                cur_min = a[i];
                cur_min_idx = i;
            }
            if(a[i] > cur_max)
            {
                cur_max = a[i];
                cur_max_idx = i;
            }
        }

        local_min[tid] = cur_min;
        local_min_idx[tid] = cur_min_idx;
        local_max[tid] = cur_max;
        local_max_idx[tid] = cur_max_idx;
    }

    min_val = local_min[0];
    min_idx = local_min_idx[0];
    max_val = local_max[0];
    max_idx = local_max_idx[0];

    for(int t = 1; t < num_threads; t++)
    {
        if(local_min[t] < min_val)
        {
            min_val = local_min[t];
            min_idx = local_min_idx[t];
        }
        if(local_max[t] > max_val)
        {
            max_val = local_max[t];
            max_idx = local_max_idx[t];
        }
    }
}

bool verify_res(int s_min, int p_min, unsigned long s_min_idx, unsigned long p_min_idx,
                int s_max, int p_max, unsigned long s_max_idx, unsigned long p_max_idx)
{
    return (s_min == p_min && s_min_idx == p_min_idx && s_max == p_max && s_max_idx == p_max_idx);
}

void runminmax()
{
    unsigned long n = 1e7;
    vector<int> a(n);

    for(unsigned long i = 0; i < n; i++)
    {
        a[i] = 100 + (i % 1000);
    }
    a[123456] = -50;
    a[789012] = 50000;

    int s_min, s_max;
    unsigned long s_min_idx, s_max_idx;

    double s_start = omp_get_wtime();
    vec_minmax_serial(a, n, s_min, s_min_idx, s_max, s_max_idx);
    double s_end = omp_get_wtime();
    cout << "Serial Time : " << (s_end - s_start) * 1000 << " ms" << endl;

    int p_min, p_max;
    unsigned long p_min_idx, p_max_idx;

    double p_start = omp_get_wtime();
    vec_minmax_parallel(a, n, p_min, p_min_idx, p_max, p_max_idx);
    double p_end = omp_get_wtime();
    cout << "omp time taken : " << (p_end - p_start) * 1000 << " ms" << endl;

    cout << "Serial Min : " << s_min << " at index " << s_min_idx << endl;
    cout << "Parallel Min : " << p_min << " at index " << p_min_idx << endl;
    cout << "Serial Max : " << s_max << " at index " << s_max_idx << endl;
    cout << "Parallel Max : " << p_max << " at index " << p_max_idx << endl;

    cout << "Are answer same : " << (verify_res(s_min, p_min, s_min_idx, p_min_idx, s_max, p_max, s_max_idx, p_max_idx) ? "Yes" : "No") << endl;
}

int main()
{
    runminmax();
    return 0;
}
