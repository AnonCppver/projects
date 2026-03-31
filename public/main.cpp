#include "base/timeUtil.h"
#include "base/strUtil.h"

#include <iostream>
using namespace std;
using namespace leef;

int main()
{


    Timestamp t = Timestamp::now();
    time_t time=(time_t)t.secondsSinceEpoch();

    Timer timer;
    timer.reset();

    for(int i=0;i<1000000;i++)
    {
        t.toString();
    }

    cout<<"t.toString 1M times elapsedMS:"<<timer.elapsedMS()<<endl;
    timer.reset();

    for(int i=0;i<1000000;i++)
    {
        time2str(time);
    }

    cout<<"time2str 1M times elapsedMS:"<<timer.elapsedMS()<<endl;
    timer.reset();

    for(int i=0;i<1000000;i++)
    {
        t.toFormattedString();
    }

    cout<<"t.toFormattedString 1M times elapsedMS:"<<timer.elapsedMS()<<endl;

    return 0;
}