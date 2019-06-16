//
// Created by Administrator on 2019/6/7.
//
#include "IMComm.h"
#include "Logging.h"

int DJBHash(std::string str)
{
    int hash = 5381;
    for(int i = 0; i < str.size(); i++)
    {
        hash = ((hash << 5) + hash) + str[i]; //hash = hash * 32 + c
    }
    return (hash & 0x7FFFFFFF);
}

void replaceFromZero(string &value,int byte_size)
{
    int i = 0;
    for( i = 0; i < byte_size;i++)
    {
        if(value[i] == '~')
        {
            LOG_ERROR << "replace error : '~' has exist";
        }
        if(value[i] == '\0')
        {
            value[i] = '~';
        }
    }
}
void replaceFromZero(char *value,int byte_size)
{
    int i = 0;
    for( i = 0; i < byte_size;i++)
    {
        if(value[i] == '~')
        {
            LOG_ERROR << "replace error : '~' has exist";
        }
        if(value[i] == '\0')
        {
            value[i] = '~';
        }
    }
}
void replaceToZero(string &value)
{
    int i ;
    for( i = 0; i < value.size();i++)
    {
        if(value[i] == '~')
        {
            value[i] = '\0';
        }
    }
    //value[i] = '\0';
}