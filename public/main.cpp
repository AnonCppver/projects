#include "include/strUtil.h"
#include <iostream>
using namespace std;
using namespace prj;

int main()
{
    string s = " 一二三 ";
    string ss=trim(s);
    ss=toUpper(ss);
    cout<<"s:"<<s<<endl;
    cout<<"ss:"<<ss<<endl;

    return 0;
}