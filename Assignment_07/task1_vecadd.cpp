#include <iostream>
#include <vector>
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

bool verify_res(const vector<double>& a, const vector<double>& b, unsigned long size)
{
    for(unsigned long i = 0; i < size; i++)
    {
        if(a[i] != b[i]) return false;
    }
    return true;
}

void runvecadd()
{
    unsigned long n = 1e7;
    vector<double> a(n, 2.0);
    vector<double> b(n, 3.0);
    vector<double> pc(n, 0.0);
    vector<double> sc(n, 0.0);

    double s_start = omp_get_wtime();
    vec_add_serial(a, b, sc, n);
    double s_end = omp_get_wtime();
    cout << "Serial Time : " << (s_end - s_start) * 1000 << " ms" << endl;

    double p_start = omp_get_wtime();
    vec_add_parallel(a, b, pc, n);
    double p_end = omp_get_wtime();
    cout << "omp time taken : " << (p_end - p_start) * 1000 << " ms" << endl;

    cout << "Are answer same : " << (verify_res(pc, sc, n) ? "Yes" : "No") << endl;
}

int main()
{
    runvecadd();
    return 0;
}
