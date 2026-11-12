#include <iostream>
#include <vector>
#include <omp.h>

using namespace std;

void mat_transpose_serial(const vector<double>& A, vector<double>& B, int rows, int cols)
{
    for(int i = 0; i < rows; i++)
    {
        for(int j = 0; j < cols; j++)
        {
            B[j * rows + i] = A[i * cols + j];
        }
    }
}

void mat_transpose_parallel(const vector<double>& A, vector<double>& B, int rows, int cols)
{
    #pragma omp parallel for
    for(int i = 0; i < rows; i++)
    {
        for(int j = 0; j < cols; j++)
        {
            B[j * rows + i] = A[i * cols + j];
        }
    }
}

void mat_transpose_collapse(const vector<double>& A, vector<double>& B, int rows, int cols)
{
    #pragma omp parallel for collapse(2)
    for(int i = 0; i < rows; i++)
    {
        for(int j = 0; j < cols; j++)
        {
            B[j * rows + i] = A[i * cols + j];
        }
    }
}

bool verify_res(const vector<double>& B1, const vector<double>& B2, unsigned long size)
{
    for(unsigned long i = 0; i < size; i++)
    {
        if(B1[i] != B2[i]) return false;
    }
    return true;
}

void runtranspose()
{
    int rows = 2000;
    int cols = 2000;
    unsigned long total = (unsigned long)rows * cols;

    vector<double> A(total);
    for(unsigned long i = 0; i < total; i++)
    {
        A[i] = i * 0.5;
    }

    vector<double> B_serial(total, 0.0);
    vector<double> B_parallel(total, 0.0);
    vector<double> B_collapse(total, 0.0);

    double s_start = omp_get_wtime();
    mat_transpose_serial(A, B_serial, rows, cols);
    double s_end = omp_get_wtime();
    cout << "Serial Time : " << (s_end - s_start) * 1000 << " ms" << endl;

    double p_start1 = omp_get_wtime();
    mat_transpose_parallel(A, B_parallel, rows, cols);
    double p_end1 = omp_get_wtime();
    cout << "Parallel Outer Time : " << (p_end1 - p_start1) * 1000 << " ms" << endl;

    double p_start2 = omp_get_wtime();
    mat_transpose_collapse(A, B_collapse, rows, cols);
    double p_end2 = omp_get_wtime();
    cout << "Parallel Collapse Time : " << (p_end2 - p_start2) * 1000 << " ms" << endl;

    cout << "Are outer answers same : " << (verify_res(B_serial, B_parallel, total) ? "Yes" : "No") << endl;
    cout << "Are collapse answers same : " << (verify_res(B_serial, B_collapse, total) ? "Yes" : "No") << endl;
}

int main()
{
    runtranspose();
    return 0;
}
