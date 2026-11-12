#include <iostream>
#include <sstream>
#include <vector>
#include <omp.h>

using namespace std;

vector<vector<int>> generate_random_graph(int V , int e)
{
    vector<vector<int>> adj(V);

    int e_count = 0;
    srand(time(NULL));

    while(e_count < e)
    {
        int u = rand() % V;
        int v = rand() % V;

        if(u != v)
        {
            bool duplicate = false;
            for(int i = 0; i < adj[u].size(); i++)
            {
                if(adj[u][i] == v)
                {
                    duplicate = true;
                    break;
                }
            }
            if(!duplicate)
            {
                adj[u].push_back(v);
                e_count++;
            }
        }
    }
    return adj;
}


double convert_serail(const vector<vector<int>>& list , vector<int>& row_ptr , vector<int>& col_idx)
{
    int n = list.size();
    for(int i = 0; i <= n; i++)
    {
        row_ptr.push_back(-1);
    }

    double start = omp_get_wtime();
    row_ptr[0] = 0;
    for(int i = 0; i < n; i++)
    {
        for(int j = 0; j < list[i].size(); j++)
        {
            col_idx.push_back(list[i][j]);
        }
        row_ptr[i + 1] = col_idx.size();
    }
    double end = omp_get_wtime();
    return (end - start);
}

double convert_parallel(const vector<vector<int>>& list , vector<int>& row_ptr , vector<int>& col_idx)
{
    int n = list.size();
    for(int i = 0; i <= n; i++)
    {
        row_ptr.push_back(-1);
    }
    row_ptr[0] = 0;
    for(int i = 0; i < n; i++)
    {
        for(int j = 0; j < list[i].size(); j++)
        {
            col_idx.push_back(-1);
        }
        row_ptr[i + 1] = col_idx.size();
    }

    //Parallel calculation of vertex degree
    vector<int> d_arr(n , -1);

    #pragma omp parallel for
    for(int i = 0; i < n; i++)
    {
        d_arr[i] = list[i].size();
    }

    //degree calculation ends here
    
    //Parallel prefix sum

    vector<int> p_sum(n , -1);

    #pragma omp parallel for
    for(int i = 0; i < n; i++)
    {
        int sum = 0;
        #pragma omp parallel for reduction(+ : sum)
        for(int j = 0; j <= i; j++)
        {
            sum += d_arr[j];
        }

        p_sum[i] = sum;
    }

    //prefix sum end

    //Population of col_idx
    double start = omp_get_wtime();
    #pragma omp parallel for
    for(int i = 0; i < n; i++)
    {
        for(int j = 0; j < list[i].size(); j++)
        {
            col_idx[row_ptr[i] + j] = list[i][j];
        }
    }
    double end = omp_get_wtime();
    //col_idx filling end here

    return (end - start);
}

bool ver_res(const vector<int> srow_ptr , const vector<int> scol_idx , const vector<int> prow_ptr , const vector<int> pcol_idx)
{
    if(srow_ptr.size() != prow_ptr.size()) return false;
    if(scol_idx.size() != pcol_idx.size()) return false;

    for(int i = 0; i < srow_ptr.size(); i++)
    {
        if(srow_ptr[i] != prow_ptr[i]) return false;
    }

    for(int i = 0; i < scol_idx.size(); i++)
    {
        if(scol_idx[i] != pcol_idx[i]) return false;
    }
    return true;
}

void runCSR(int v , int e)
{
    vector<vector<int>> list = generate_random_graph(v , e);

    vector<int> srow_ptr;
    vector<int> scol_idx;
    vector<int> prow_ptr;
    vector<int> pcol_idx;

    double stime = convert_serail(list , srow_ptr , scol_idx);

    cout << "Serail Execution time : " << stime * 1000 << " ms" << endl;

    double ptime = convert_parallel(list , prow_ptr , pcol_idx);

    cout << "Parallel Execution time : " << ptime * 1000 << " ms" << endl;
    cout << "Speed up achived : " << ((stime / ptime) * 100) << "%" << endl;
    cout << "Is res of serail and parallel same : " << (ver_res(srow_ptr , scol_idx , prow_ptr , pcol_idx) ? "Yes" : "No") << endl;
}

int main()
{
    cout << "Type 0 for test cases and 1 for custom" << endl;
    int a;
    cin >> a;
    if(a == 0)
    {
        int test_case[4] = {1000 , 10000 , 15000 , 50000};
        for(int test : test_case)
        {
            runCSR(test , test);
            cout << endl;
            cout << "======================" << endl;
            cout << endl;
        }
    }
    else
    {
        int v , e;
        cout << "Enter vertex count : ";
        cin >> v;
        cout << endl;

        cout << "Enter edge count  : " ;
        cin >> e;
        cout << endl;
        runCSR(v , e);
    }
    
}