#include "include/strUtil.h"
#include <iostream>
using namespace std;
using namespace strUtil;

int main()
{
    string s = "   ";
    string ss=strUtil::ltrim(s);
    cout<<"s:"<<s<<endl;
    cout<<"ss:"<<ss<<endl;

    return 0;
}