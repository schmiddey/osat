#ifndef OSATCONFIG_H_
#define OSATCONFIG_H_

#include <iostream>
#include <cstdint>
#include <string>
#include <boost/lexical_cast.hpp>

#include "Utility.h"

namespace osat{


struct OsatConfig{
  uint8_t    id         = 0;
  uint16_t   hit_thresh = 0;
  uint16_t   timeout_ms = 0;


  OsatConfig() { }

  /**
   * @brief Construct a new Osat Config object
   * 
   * 
   * @param cfg_str 
   */
  OsatConfig(const std::string cfg_str)
  {
    ok = this->fromString(cfg_str);
  }

  operator bool() const
  {
    return ok;
  }

  static bool isConfig(const std::string& data)
  {
    try
    {
      std::string str = data.substr(0, data.find('#') -1);
      str = Utility::remove_space_nl(str);
      if(str == "OsatConfig")
      {
        return true;
      }
    }
    catch(const std::out_of_range& e)
    {
      return false;
    }
    return false;
  }

  /**
   * @brief 
   * 
   * How String for cfg is build:
   * "OsatConfig #id:value #hit_thresh:vlaue #timout_ms value"
   * type of cfg starts with "#" then name of cfg then ":" then value...   * 
   * example string: "OsatConfig #id:12 #hit_thresh:150 #timeout_ms:1500"
   * 
   * @note dont know if this parser is good... i think not just hacked xD
   * 
   * @param cfg_str param string form an osat
   * @return true if parsing is ok
   * @return false if parsing failed
   */
  bool fromString(const std::string cfg_str)
  {
    //check string if Config
    //get OsatConfig substring
    if(!isConfig(cfg_str))
    {
      return false;
    }
    std::string remain = cfg_str.substr(cfg_str.find('#'));

    // std::cout << "start: " << start << std::endl;
    bool rdy = false;
    while(!rdy)
    {
      // std::cout << "remain: " << remain << std::endl;
      std::string cfg_tag = remain.substr(1, remain.find(':') -1);
      // std::cout << "cfg_tag: " << cfg_tag << std::endl;
      remain = remain.substr(remain.find(':') + 1);
      std::string value_str = remain.substr(0, remain.find('#') -1);
      // std::cout << "value_str: " << value_str << std::endl;
      try
      {
        remain = remain.substr(remain.find('#'));
      }
      catch(const std::out_of_range& e)
      {
        //rdy
        rdy = true;
      }
      
      uint32_t value = 0;
      // std::cout << "value_str: " << value_str << std::endl;

      try{
        value = std::stoi(value_str);
      } catch(std::invalid_argument& e)
      {
        std::cout << "Failed Parsing: " << value_str << "to int... " << e.what() << std::endl;
        return false;
      }

      // check if remove space is needed
      if(cfg_tag == "id")
      {
        id = value;
      }
      else if(cfg_tag == "hit_thresh")
      {
        hit_thresh = value;
      }
      else if(cfg_tag == "timeout_ms")
      {
        timeout_ms = value;
      }
      else
      {
        //fail
        std::cout << "Failed parse config" << std::endl;
        return false;
      }
    }
    return true;
  }

  std::string toString() const
  {
    return "OsatConfig #id: " + std::to_string(id)  + " #hit_thresh: "  + std::to_string(hit_thresh)  + " #timeout_ms: " + std::to_string(timeout_ms);
  }

private:
  bool ok = false;

};

} //namespace osat


inline std::ostream& operator<<(std::ostream &out, const osat::OsatConfig& cfg)
{
  out << cfg.toString();
  return out;
}

#endif  //OSATCONFIG_H_
