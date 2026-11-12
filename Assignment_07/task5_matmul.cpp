#include <iostream>
#include <vector>
#include <cmath>
#include <omp.h>

using namespace std;

void matmul_serial(const vector<double>& A, const vector<double>& B, vector<double>& C, int n)
{
    for(int i = 0; i < n; i++)
    {
        for(int j = 0; j < n; j++)
        {
            double sum = 0.0;
            for(int k = 0; k < n; k++)
            {
                sum += A[i * n + k] * B[k * n + j];
            }
            C[i * n + j] = sum;
        }
    }
}

void matmul_parallel(const vector<double>& A, const vector<double>& B, vector<double>& C, int n)
{
    #pragma omp parallel for
    for(int i = 0; i < n; i++)
    {
        for(int j = 0; j < n; j++)
        {
            double sum = 0.0;
            for(int k = 0; k < n; k++)
            {
                sum += A[i * n + k] * B[k * n + j];
            }
            C[i * n + j] = sum;
        }
    }
}

bool verify_res(const vector<double>& C1, const vector<double>& C2, unsigned long size)
{
    for(unsigned long i = 0; i < size; i++)
    {
        if(fabs(C1[i] - C2[i]) > 1e-6) return false;
    }
    return true;
}

void runmatmul()
{
    int n = 500;
    unsigned long total = (unsigned long)n * n;

    vector<double> A(total, 1.0);
    vector<double> B(total, 2.0);
    vector<double> C_serial(total, 0.0);
    vector<double> C_parallel(total, 0.0);

    double s_start = omp_get_wtime();
    matmul_serial(A, B, C_serial, n);
    double s_end = omp_get_wtime();
    cout << "Serial Time : " << (s_end - s_start) * 1000 << " ms" << endl;

    double p_start = omp_get_wtime();
    matmul_parallel(A, B, C_parallel, n);
    double p_end = omp_get_wtime();
    cout << "omp time taken : " << (p_end - p_start) * 1000 << " ms" << endl;

    cout << "Are answer same : " << (verify_res(C_serial, C_parallel, total) ? "Yes" : "No") << endl;
}

int main()
{
    runmatmul();
    return 0;
}
