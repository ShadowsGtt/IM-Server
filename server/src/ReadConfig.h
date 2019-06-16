#ifndef _READCONFIG_H
#define _READCONFIG_H

#include <fstream>
#include <algorithm>
#include <cerrno>
#include <iostream>
#include <string>
#include <cstring>
#include <map>
#include <utility>
#include <vector>
#include <stdio.h>

using namespace std;

namespace
{
    vector<string> knownFields =
    {
        "host"              ,       // 服务器ip
        "port"              ,       // 服务器port
        "ioThreadNumber"    ,       // io线程数目
        "serverName"        ,       // 服务器名字
        "maxConn"           ,       // 客户最大连接数
        "tpNum"             ,       // 线程池线程数
        "daemon"            ,       // 是否设置守护进程
        //"maxFd"             ,       // 最大fd数目
        "logFlushInterval"  ,       // 日志刷磁盘间隔
        "logRollSie"                // 日志回滚尺寸
    };



    /* 去除头尾空格 */
    void removeSpace(string& line)
    {
        if(line.size() == 0)
            return ;

        string::iterator begin;
        while(true)
        {
            begin = line.begin();
            if(*begin == ' ' || *begin == '\t'){
                line.erase(begin);
                //begin++;
                continue;
            }
            break;
        }
        string::iterator end;
        while(true)
        {
            end = line.end()-1;
            if(*end == ' ' || *end == '\t'){
                line.erase(end);
                //end--;
                continue;
            }
            break;
        }
        return;
    }

    /* 将字符串分割成多个参数 */
    /* 分析配置文件中的每一行,提取出key-value-value-... */
    pair<string,vector<string> > splitArgs(const string& line)
    {
        if(line.size() == 0)
            return pair<string,vector<string> >("",vector<string>{});

        vector<string> store;
        pair<string,vector<string> > ret;
        bool flag = true;
        string::const_iterator strBegin;
        string::const_iterator strEnd;
        strBegin = strEnd = line.end();

        for(string::const_iterator it = line.begin();it != line.end() ; it++){
            if( *it != ' ' && strBegin == line.end()){
                    strBegin = it;
            }
            else if( *it == ' ' || it == line.end() ){
                strEnd = it;
                if(strBegin != line.end()){
                    store.push_back(line.substr(strBegin-line.begin(),strEnd-strBegin));
                    strBegin = line.end();
                    strEnd = line.end();
                }
            }
        }
        store.push_back(line.substr(strBegin-line.begin(),strEnd-strBegin));
        ret.first = store[0];
        store.erase(store.begin());
        for(auto &c : store)
            ret.second.push_back(c);

        return ret;

    }

    /* kv 是否合法 */
    bool isValid(const pair<string,vector<string> >& kv , const int& curline , const string& line)
    {
        if(kv.first.size() == 0 || kv.second.size() == 0)
            return false ;

        string key = kv.first;
        vector<string> values = kv.second ;
        string value = kv.second[0];

        if(values.size() > 1){
            cerr << "*** FATAL CONFIG FILE ERROR ***" << endl;
            cerr << "error at line " << curline << ":" << line << endl;
            exit(1);
        }
        if(find(knownFields.begin(),knownFields.end(),key) == knownFields.end() )
        {
            cerr << "*** FATAL CONFIG FILE ERROR ***" << endl;
            cerr << "error at line " << curline << ": unknown \"" << key  << "\"" << endl;
            exit(1);
        }
        if(key == "daemon" )
        {
            if(value != "yes" && value != "no"){
                cerr << "*** FATAL CONFIG FILE ERROR ***" << endl;
                cerr << "error at line " << curline << ": unknow \"" << value << "\"" << endl;
                exit(1);
            }
        }

        return true;
    }

}   // end namespace

/* 接口 */
map<string,string> readConfig(const char* filename)
{
    ifstream fs;
    map<string,string> ret;
    int curLine = 0;

    fs.open(filename, ios::in | ios::binary);
    if(!fs.is_open()){
        cerr << "*** FATAL CONFIG FILE ERROR ***" << endl;
        cerr << "open \"" << filename << "\" error :" ;
        cerr << strerror(errno);
        cerr << endl;
        exit(errno);
    }
    while(fs.peek() != EOF)
    {
        string line;
        curLine++;

        getline(fs,line,'\n');
        line.erase(line.end()-1);
        removeSpace(line);
        if(line[0] == '#' || line[0] == '\n')
            continue;
        pair<string,vector<string> > kv = splitArgs(line);
        if(isValid(kv,curLine,line))
        {
            ret.insert(make_pair(kv.first,kv.second[0]));
        }
    }

    return ret;
}

int toByte(string str)
{
    string unit;
    string number;
    for (auto &c : str) {
        if(c <= '9' && c >= '0'){
            number.append(1,c);
        } else if(c <= 'Z' && c >= 'A'){
            unit.append(1,c);
        }
    }
    if(unit == "B"){
        return std::stoi(number);
    } else if(unit == "KB"){
        return std::stoi(number)*1024;
    } else if(unit == "MB"){
        return std::stoi(number)*1024*1024;
    } else if(unit == "GB"){
        return std::stoi(number)*1024*1024*1024;
    }
    return 0;
}

#endif
