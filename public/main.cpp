#include "include/timeUtil.h"
#include <iostream>
using namespace std;
using namespace prj;

int main()
{
    Timer timer;
    timer.reset();

    for(int i=0;i<1000000;i++)
    {
        Timestamp t = Timestamp::now();
        string s = t.toString();
        if(i%100000==0)
        {
            cout<<"t:"<<t.toString()<<endl;
        }
    }

    cout<<"elapsedMS:"<<timer.elapsedMS()<<endl;

    return 0;
}