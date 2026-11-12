#include <iostream>
#include <vector>
#include <cmath>
#include <omp.h>

using namespace std;

void compute_serial(vector<double>& a, int n)
{
    for(int i = 0; i < n; i++)
    {
        double x = a[i];
        int work = (i % 1000) * 10;
        for(int j = 0; j < work; j++)
        {
            x = x * 1.0000001 + 0.0000001;
        }
        a[i] = x;
    }
}

void compute_static(vector<double>& a, int n)
{
    #pragma omp parallel for schedule(static)
    for(int i = 0; i < n; i++)
    {
        double x = a[i];
        int work = (i % 1000) * 10;
        for(int j = 0; j < work; j++)
        {
            x = x * 1.0000001 + 0.0000001;
        }
        a[i] = x;
    }
}

void compute_dynamic(vector<double>& a, int n)
{
    #pragma omp parallel for schedule(dynamic, 100)
    for(int i = 0; i < n; i++)
    {
        double x = a[i];
        int work = (i % 1000) * 10;
        for(int j = 0; j < work; j++)
        {
            x = x * 1.0000001 + 0.0000001;
        }
        a[i] = x;
    }
}

void compute_guided(vector<double>& a, int n)
{
    #pragma omp parallel for schedule(guided)
    for(int i = 0; i < n; i++)
    {
        double x = a[i];
        int work = (i % 1000) * 10;
        for(int j = 0; j < work; j++)
        {
            x = x * 1.0000001 + 0.0000001;
        }
        a[i] = x;
    }
}

bool verify_res(const vector<double>& a, const vector<double>& b, int n)
{
    for(int i = 0; i < n; i++)
    {
        if(fabs(a[i] - b[i]) > 1e-6) return false;
    }
    return true;
}

void runschedule()
{
    int n = 50000;
    vector<double> a_serial(n);
    vector<double> a_static(n);
    vector<double> a_dynamic(n);
    vector<double> a_guided(n);

    for(int i = 0; i < n; i++)
    {
        double val = i * 0.001;
        a_serial[i] = val;
        a_static[i] = val;
        a_dynamic[i] = val;
        a_guided[i] = val;
    }

    double s_start = omp_get_wtime();
    compute_serial(a_serial, n);
    double s_end = omp_get_wtime();
    cout << "Serial Time : " << (s_end - s_start) * 1000 << " ms" << endl;

    double t_start1 = omp_get_wtime();
    compute_static(a_static, n);
    double t_end1 = omp_get_wtime();
    cout << "Static Schedule Time : " << (t_end1 - t_start1) * 1000 << " ms" << endl;

    double t_start2 = omp_get_wtime();
    compute_dynamic(a_dynamic, n);
    double t_end2 = omp_get_wtime();
    cout << "Dynamic Schedule Time : " << (t_end2 - t_start2) * 1000 << " ms" << endl;

    double t_start3 = omp_get_wtime();
    compute_guided(a_guided, n);
    double t_end3 = omp_get_wtime();
    cout << "Guided Schedule Time : " << (t_end3 - t_start3) * 1000 << " ms" << endl;

    cout << "Static matches serial : " << (verify_res(a_serial, a_static, n) ? "Yes" : "No") << endl;
    cout << "Dynamic matches serial : " << (verify_res(a_serial, a_dynamic, n) ? "Yes" : "No") << endl;
    cout << "Guided matches serial : " << (verify_res(a_serial, a_guided, n) ? "Yes" : "No") << endl;
}

int main()
{
    runschedule();
    return 0;
}
