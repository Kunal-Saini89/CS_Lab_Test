#include <iostream>
#include <vector>
#include <omp.h>

using namespace std;

void hist_serial(const vector<int>& a, unsigned long size, vector<long>& hist)
{
    for(int k = 0; k < 10; k++)
    {
        hist[k] = 0;
    }

    for(unsigned long i = 0; i < size; i++)
    {
        hist[a[i]]++;
    }
}

void hist_parallel(const vector<int>& a, unsigned long size, vector<long>& hist)
{
    for(int k = 0; k < 10; k++)
    {
        hist[k] = 0;
    }

    #pragma omp parallel
    {
        vector<long> local_hist(10, 0);

        #pragma omp for
        for(unsigned long i = 0; i < size; i++)
        {
            local_hist[a[i]]++;
        }

        #pragma omp critical
        {
            for(int k = 0; k < 10; k++)
            {
                hist[k] += local_hist[k];
            }
        }
    }
}

bool verify_res(const vector<long>& h1, const vector<long>& h2)
{
    for(int k = 0; k < 10; k++)
    {
        if(h1[k] != h2[k]) return false;
    }
    return true;
}

void runhist()
{
    unsigned long n = 1e7;
    vector<int> a(n);

    for(unsigned long i = 0; i < n; i++)
    {
        a[i] = i % 10;
    }

    vector<long> s_hist(10, 0);
    vector<long> p_hist(10, 0);

    double s_start = omp_get_wtime();
    hist_serial(a, n, s_hist);
    double s_end = omp_get_wtime();
    cout << "Serial Time : " << (s_end - s_start) * 1000 << " ms" << endl;

    double p_start = omp_get_wtime();
    hist_parallel(a, n, p_hist);
    double p_end = omp_get_wtime();
    cout << "omp time taken : " << (p_end - p_start) * 1000 << " ms" << endl;

    cout << "Are answer same : " << (verify_res(s_hist, p_hist) ? "Yes" : "No") << endl;
}

int main()
{
    runhist();
    return 0;
}
