#include <iostream>
#include <vector>
#include <omp.h>

using namespace std;

void mat_add_serial(const vector<double>& A, const vector<double>& B, vector<double>& C, int n)
{
    for(int i = 0; i < n; i++)
    {
        for(int j = 0; j < n; j++)
        {
            C[i * n + j] = A[i * n + j] + B[i * n + j];
        }
    }
}

void mat_add_parallel(const vector<double>& A, const vector<double>& B, vector<double>& C, int n)
{
    #pragma omp parallel for
    for(int i = 0; i < n; i++)
    {
        for(int j = 0; j < n; j++)
        {
            C[i * n + j] = A[i * n + j] + B[i * n + j];
        }
    }
}

void mat_add_collapse(const vector<double>& A, const vector<double>& B, vector<double>& C, int n)
{
    #pragma omp parallel for collapse(2)
    for(int i = 0; i < n; i++)
    {
        for(int j = 0; j < n; j++)
        {
            C[i * n + j] = A[i * n + j] + B[i * n + j];
        }
    }
}

bool verify_res(const vector<double>& C1, const vector<double>& C2, unsigned long size)
{
    for(unsigned long i = 0; i < size; i++)
    {
        if(C1[i] != C2[i]) return false;
    }
    return true;
}

void runmatadd()
{
    int n = 2000;
    unsigned long total = (unsigned long)n * n;

    vector<double> A(total, 1.5);
    vector<double> B(total, 2.5);
    vector<double> C_serial(total, 0.0);
    vector<double> C_outer(total, 0.0);
    vector<double> C_collapse(total, 0.0);

    double s_start = omp_get_wtime();
    mat_add_serial(A, B, C_serial, n);
    double s_end = omp_get_wtime();
    cout << "Serial Time : " << (s_end - s_start) * 1000 << " ms" << endl;

    double p_start1 = omp_get_wtime();
    mat_add_parallel(A, B, C_outer, n);
    double p_end1 = omp_get_wtime();
    cout << "Parallel Outer Time : " << (p_end1 - p_start1) * 1000 << " ms" << endl;

    double p_start2 = omp_get_wtime();
    mat_add_collapse(A, B, C_collapse, n);
    double p_end2 = omp_get_wtime();
    cout << "Parallel Collapse Time : " << (p_end2 - p_start2) * 1000 << " ms" << endl;

    cout << "Are outer answers same : " << (verify_res(C_serial, C_outer, total) ? "Yes" : "No") << endl;
    cout << "Are collapse answers same : " << (verify_res(C_serial, C_collapse, total) ? "Yes" : "No") << endl;
}

int main()
{
    runmatadd();
    return 0;
}
