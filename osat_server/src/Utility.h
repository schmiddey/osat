#ifndef UTILITY_H_
#define UTILITY_H_

#include <iostream>
#include <string>
#include <algorithm>

class Utility
{
public:
  static std::string check_eol(const std::string& data)
  {
    if(data.back() != '\n')
    {
      return data + '\n';
    }
    return data;
  }

  static std::string remove_nl(const std::string& data)
  {
    std::string ret(data);
    ret.erase(std::remove_if(ret.begin(), ret.end(), 
                             [](char c){
                               return c == '\n';
                             }),
                             ret.end());
    return ret;
  }

  static std::string remove_space(const std::string& data)
  {
    std::string ret(data);
    ret.erase(std::remove_if(ret.begin(), ret.end(), 
                             [](char c){
                               return c == ' ';
                             }),
                             ret.end());
    return ret;
  }

  static std::string remove_space_nl(const std::string& data)
  {
    std::string ret(data);
    ret.erase(std::remove_if(ret.begin(), ret.end(), 
                             [](char c){
                               return (c == ' ' || c == '\n');
                             }),
                             ret.end());
    return ret;
  }
};

#endif  //UTILITY_H_