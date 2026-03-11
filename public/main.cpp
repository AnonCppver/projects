#include "include/timeUtil.h"
#include "include/strUtil.h"
#include "include/LogStream.h"
#include <iostream>
using namespace std;
using namespace prj;
// g++ -std=c++17 -g main.cpp -L./lib -lstrUtil -ltimeUtil -lLogStream -Wl,-rpath=./lib -o main
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

    LogStream log;
    log<<"hello "<<123<<" "<<123.456;
    cout<<log.buffer().toString()<<endl;

    return 0;
}