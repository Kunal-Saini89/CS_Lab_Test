#include <iostream>
#include <sstream>
#include <vector>
#include <omp.h>

using namespace std;

void giveinput(vector<int>& grid , int& i , int& j)
{
    grid = vector<int>(i * j , -1);
    for(int a = 0 ; i < a; a++)
    {
        for(int b = 0; b < j; b++)
        {
            grid[a * i + b] = rand() % 2;
        }
    }
}

void serail(const vector<int>& org , int a , int b , vector<int>& res , int iterations)
{
    vector<int> grid = org;
    res = vector<int>(org.size() , -1);
    double start = omp_get_wtime();
    double send = 0 , pend = 0;
    for(int x = 0; x < iterations; x++)
    {
        // static 
        for(int i = 0; i < a; i++)
        {
            for(int j = 0; j < b; j++)
            {
                int me = grid[i * a + j];
                vector<int> n(4 , 0);
                // For n0
                {
                    int index = i * a + (j - 1);
                    if(index >= 0 && index <= grid.size())
                    {
                        n[0] = grid[index];
                    }
                }

                // For n1
                {
                    int index = (i - 1) * a + j;
                    if(index >= 0 && index <= grid.size())
                    {
                        n[1] = grid[index];
                    }
                }

                // For n2
                {
                    int index = i * a + (j + 1);
                    if(index >= 0 && index <= grid.size())
                    {
                        n[2] = grid[index];
                    }
                }

                // For n3
                {
                    int index = (i + 1) * a + j;
                    if(index >= 0 && index <= grid.size())
                    {
                        n[3] = grid[index];
                    }
                }

                int total = 0;
                for(int i : n)
                {
                    if(i == 1) total++;
                }

                if(me == 1 && total == 2 || total == 3) res[i * a + j] = 1;
                else if (me == 0 && total == 3) res[i * a + j] = 1;
                else res[i * a + j] = 0;
            }
        }
        if(x + 1 == iterations)
        {
            send = omp_get_wtime();
        }
        grid = res;
        res.clear();

        //Parallel
        vector<int> pgrid = org;
        res = vector<int>(org.size() , -1);
        for(int kk = 0; kk < 2; kk++)
        {
            if(kk == 0)
            {
                #pragma omp parallel for collapse(2)
                for(int i = 0; i < a; i += 2)
                {
                    for(int j = 0; j < b; j += 2)
                    {
                        int me = pgrid[i * a + j];
                        vector<int> n(4 , 0);
                        // For n0
                        {
                            int index = i * a + (j - 1);
                            if(index >= 0 && index <= pgrid.size())
                            {
                                n[0] = pgrid[index];
                            }
                        }

                        // For n1
                        {
                            int index = (i - 1) * a + j;
                            if(index >= 0 && index <= pgrid.size())
                            {
                                n[1] = pgrid[index];
                            }
                        }

                        // For n2
                        {
                            int index = i * a + (j + 1);
                            if(index >= 0 && index <= pgrid.size())
                            {
                                n[2] = pgrid[index];
                            }
                        }

                        // For n3
                        {
                            int index = (i + 1) * a + j;
                            if(index >= 0 && index <= pgrid.size())
                            {
                                n[3] = pgrid[index];
                            }
                        }

                        int total = 0;
                        for(int i : n)
                        {
                            if(i == 1) total++;
                        }

                        if(me == 1 && total == 2 || total == 3) res[i * a + j] = 1;
                        else if (me == 0 && total == 3) res[i * a + j] = 1;
                        else res[i * a + j] = 0;
                    }
                }
            }
            else
            {
                #pragma omp parallel for collapse(2)
                for(int i = 1; i < a; i += 2)
                {
                    for(int j = 1; j < b; j += 2)
                    {
                        int me = pgrid[i * a + j];
                        vector<int> n(4 , 0);
                        // For n0
                        {
                            int index = i * a + (j - 1);
                            if(index >= 0 && index <= pgrid.size())
                            {
                                n[0] = pgrid[index];
                            }
                        }

                        // For n1
                        {
                            int index = (i - 1) * a + j;
                            if(index >= 0 && index <= pgrid.size())
                            {
                                n[1] = pgrid[index];
                            }
                        }

                        // For n2
                        {
                            int index = i * a + (j + 1);
                            if(index >= 0 && index <= pgrid.size())
                            {
                                n[2] = pgrid[index];
                            }
                        }

                        // For n3
                        {
                            int index = (i + 1) * a + j;
                            if(index >= 0 && index <= pgrid.size())
                            {
                                n[3] = pgrid[index];
                            }
                        }

                        int total = 0;
                        for(int i : n)
                        {
                            if(i == 1) total++;
                        }

                        if(me == 1 && total == 2 || total == 3) res[i * a + j] = 1;
                        else if (me == 0 && total == 3) res[i * a + j] = 1;
                        else res[i * a + j] = 0;
                    }
                }
            }
        }
        if(x + 1 == iterations)
        {
            pend = omp_get_wtime();
        }
        pgrid = res;
        res.clear();

        bool issame = true;
        for(int i = 0; i < org.size(); i++)
        {
            if(grid[i] != pgrid[i])
            {
                issame = false;
                break;
            }
        }
        cout << "For iteration " << x << " serial and parallel are sam : " << (issame ? "Yes" : "No") << endl; 
    }
}

int main()
{
    vector<int> test;
    int i = 256 , j = 256;
    giveinput(test, &i , &j);
}