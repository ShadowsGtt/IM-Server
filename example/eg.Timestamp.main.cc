#include <iostream>
#include "../server/src/Timestamp.h"

using namespace std;


int main(void)
{
    Timestamp time1 = Timestamp::now();
    cout <<"Now :  " <<  time1.toFormattedString(true) << endl;

    Timestamp time2(1);
    cout <<"1微秒时间 :  " <<  time2.toFormattedString(true) << endl;

    Timestamp time3 = Timestamp::fromUnixTime(0,1);
    cout <<"fromUnixTime(time_t,int) :  " <<  time3.toFormattedString(true) << endl;

    Timestamp time4 = Timestamp::fromUnixTime(1);
    cout <<"fromUnixTime(time_t) :  " <<  time4.toFormattedString(true) << endl;

    return 0;
}
