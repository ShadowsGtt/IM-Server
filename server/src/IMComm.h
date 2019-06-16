//
// Created by lonnyliu(刘卓) on 2019-05-19.
//

#ifndef IM2_IMCOMM_H
#define IM2_IMCOMM_H
#include <string>


int DJBHash(std::string str);


void replaceFromZero(std::string &value,int byte_size);

void replaceFromZero(char *value,int byte_size);

void replaceToZero(std::string &value);


#endif //IM2_IMCOMM_H
