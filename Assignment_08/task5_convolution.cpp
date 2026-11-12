#include <iostream>
#include <vector>
#include <cmath>
#include <omp.h>

using namespace std;

void conv_serial(const vector<double>& input, vector<double>& output, int H, int W, const double kernel[3][3])
{
    int out_H = H - 2;
    int out_W = W - 2;

    for(int i = 0; i < out_H; i++)
    {
        for(int j = 0; j < out_W; j++)
        {
            double sum = 0.0;
            for(int ki = 0; ki < 3; ki++)
            {
                for(int kj = 0; kj < 3; kj++)
                {
                    sum += input[(i + ki) * W + (j + kj)] * kernel[ki][kj];
                }
            }
            output[i * out_W + j] = sum;
        }
    }
}

void conv_parallel(const vector<double>& input, vector<double>& output, int H, int W, const double kernel[3][3])
{
    int out_H = H - 2;
    int out_W = W - 2;

    #pragma omp parallel for
    for(int i = 0; i < out_H; i++)
    {
        for(int j = 0; j < out_W; j++)
        {
            double sum = 0.0;
            for(int ki = 0; ki < 3; ki++)
            {
                for(int kj = 0; kj < 3; kj++)
                {
                    sum += input[(i + ki) * W + (j + kj)] * kernel[ki][kj];
                }
            }
            output[i * out_W + j] = sum;
        }
    }
}

bool verify_res(const vector<double>& out1, const vector<double>& out2, unsigned long size)
{
    for(unsigned long i = 0; i < size; i++)
    {
        if(fabs(out1[i] - out2[i]) > 1e-6) return false;
    }
    return true;
}

void runconv()
{
    int H = 1000;
    int W = 1000;
    int out_H = H - 2;
    int out_W = W - 2;
    unsigned long in_size = (unsigned long)H * W;
    unsigned long out_size = (unsigned long)out_H * out_W;

    vector<double> input(in_size);
    for(unsigned long i = 0; i < in_size; i++)
    {
        input[i] = (i % 256);
    }

    double kernel[3][3] = {
        {1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0},
        {1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0},
        {1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0}
    };

    vector<double> out_serial(out_size, 0.0);
    vector<double> out_parallel(out_size, 0.0);

    double s_start = omp_get_wtime();
    conv_serial(input, out_serial, H, W, kernel);
    double s_end = omp_get_wtime();
    cout << "Serial Time : " << (s_end - s_start) * 1000 << " ms" << endl;

    double p_start = omp_get_wtime();
    conv_parallel(input, out_parallel, H, W, kernel);
    double p_end = omp_get_wtime();
    cout << "omp time taken : " << (p_end - p_start) * 1000 << " ms" << endl;

    cout << "Are answer same : " << (verify_res(out_serial, out_parallel, out_size) ? "Yes" : "No") << endl;
}

int main()
{
    runconv();
    return 0;
}
