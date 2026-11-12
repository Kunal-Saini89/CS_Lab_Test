#include <iostream>
#include <vector>
#include <omp.h>

using namespace std;

void bubble_sort_serial(vector<int>& a, int n)
{
    for(int i = 0; i < n - 1; i++)
    {
        for(int j = 0; j < n - i - 1; j++)
        {
            if(a[j] > a[j + 1])
            {
                int temp = a[j];
                a[j] = a[j + 1];
                a[j + 1] = temp;
            }
        }
    }
}

void odd_even_sort_parallel(vector<int>& a, int n)
{
    #pragma omp parallel
    {
        for(int phase = 0; phase < n; phase++)
        {
            if(phase % 2 == 0)
            {
                #pragma omp for
                for(int i = 0; i < n - 1; i += 2)
                {
                    if(a[i] > a[i + 1])
                    {
                        int temp = a[i];
                        a[i] = a[i + 1];
                        a[i + 1] = temp;
                    }
                }
            }
            else
            {
                #pragma omp for
                for(int i = 1; i < n - 1; i += 2)
                {
                    if(a[i] > a[i + 1])
                    {
                        int temp = a[i];
                        a[i] = a[i + 1];
                        a[i + 1] = temp;
                    }
                }
            }
        }
    }
}

bool verify_res(const vector<int>& a, const vector<int>& b, int n)
{
    for(int i = 0; i < n; i++)
    {
        if(a[i] != b[i]) return false;
        if(i > 0 && b[i] < b[i - 1]) return false;
    }
    return true;
}

void runsort()
{
    int n = 8000;
    vector<int> a(n);
    vector<int> b(n);

    for(int i = 0; i < n; i++)
    {
        a[i] = n - i;
        b[i] = n - i;
    }

    double s_start = omp_get_wtime();
    bubble_sort_serial(a, n);
    double s_end = omp_get_wtime();
    cout << "Serial Time : " << (s_end - s_start) * 1000 << " ms" << endl;

    double p_start = omp_get_wtime();
    odd_even_sort_parallel(b, n);
    double p_end = omp_get_wtime();
    cout << "omp time taken : " << (p_end - p_start) * 1000 << " ms" << endl;

    cout << "Are answer same : " << (verify_res(a, b, n) ? "Yes" : "No") << endl;
}

int main()
{
    runsort();
    return 0;
}
