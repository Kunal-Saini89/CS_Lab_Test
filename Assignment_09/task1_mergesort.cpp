#include <iostream>
#include <vector>
#include <omp.h>

using namespace std;

void merge(vector<int>& a, vector<int>& tmp, int left, int mid, int right)
{
    int i = left;
    int j = mid + 1;
    int k = left;

    while(i <= mid && j <= right)
    {
        if(a[i] <= a[j])
        {
            tmp[k++] = a[i++];
        }
        else
        {
            tmp[k++] = a[j++];
        }
    }

    while(i <= mid)
    {
        tmp[k++] = a[i++];
    }

    while(j <= right)
    {
        tmp[k++] = a[j++];
    }

    for(i = left; i <= right; i++)
    {
        a[i] = tmp[i];
    }
}

void merge_sort_serial(vector<int>& a, vector<int>& tmp, int left, int right)
{
    if(left >= right) return;

    int mid = left + (right - left) / 2;
    merge_sort_serial(a, tmp, left, mid);
    merge_sort_serial(a, tmp, mid + 1, right);
    merge(a, tmp, left, mid, right);
}

void merge_sort_parallel(vector<int>& a, vector<int>& tmp, int left, int right)
{
    if(left >= right) return;

    if(right - left < 2048)
    {
        merge_sort_serial(a, tmp, left, right);
        return;
    }

    int mid = left + (right - left) / 2;

    #pragma omp task shared(a, tmp) firstprivate(left, mid)
    merge_sort_parallel(a, tmp, left, mid);

    #pragma omp task shared(a, tmp) firstprivate(mid, right)
    merge_sort_parallel(a, tmp, mid + 1, right);

    #pragma omp taskwait
    merge(a, tmp, left, mid, right);
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

void runmergesort()
{
    int n = 1000000;
    vector<int> a(n);
    vector<int> b(n);
    vector<int> tmp_s(n);
    vector<int> tmp_p(n);

    for(int i = 0; i < n; i++)
    {
        a[i] = (n - i) + (i % 7);
        b[i] = a[i];
    }

    double s_start = omp_get_wtime();
    merge_sort_serial(a, tmp_s, 0, n - 1);
    double s_end = omp_get_wtime();
    cout << "Serial Time : " << (s_end - s_start) * 1000 << " ms" << endl;

    double p_start = omp_get_wtime();
    #pragma omp parallel
    {
        #pragma omp single
        merge_sort_parallel(b, tmp_p, 0, n - 1);
    }
    double p_end = omp_get_wtime();
    cout << "omp time taken : " << (p_end - p_start) * 1000 << " ms" << endl;

    cout << "Are answer same : " << (verify_res(a, b, n) ? "Yes" : "No") << endl;
}

int main()
{
    runmergesort();
    return 0;
}
