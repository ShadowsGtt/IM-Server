//
// Created by lonnyliu(刘卓) on 2019-05-19.
//

#ifndef IM2_BUSINESS_H
#define IM2_BUSINESS_H

#include <cstdint>
#include <string>
#include "IM.pb.h"

using namespace std;
namespace Business
{
    void InitBusiness();

    void StopRobHongbao(string key,string sender);

    int32_t Register(const IM::Register &info_register, IM::Response &response);

    int32_t CheckDuplication(const IM::Register &info_registe);

    int32_t sendCacheMessage(string username, google::protobuf::Message& message);

    int32_t cacheMessage(string username,const google::protobuf::Message& message);

    int32_t Login(const IM::Login &login, IM::Response &response);

    int32_t SetSession(IM::UserInfo user);

    int32_t GetFriendInfo(const IM::GetFriendInfo &reqinfo,IM::FriendInfo &respinfo);

    int32_t SendHongbao(const IM::Hongbaoreq &hongbao,IM::HongbaoResp &resp);

    int32_t RobHongbao(const IM::ReqRobHongbao &req, IM::RespRobHongbao &resp);

    int RAND(int n,int m);

    int getRandomMoney(const int &remainMoney, const int &left_MoneyPacksize);//获取随机金额红包

};


#endif //IM2_BUSINESS_H
