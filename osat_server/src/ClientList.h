#ifndef CLIENTLIST_H_
#define CLIENTLIST_H_




#include <iostream>
#include <cstdint>
#include <string>
#include <map>
#include <vector>
#include <optional>

#include "ip/Tcp.h"
#include "OsatConfig.h"

namespace osat{

/**
 * @brief 
 * 
 */
struct ClientInfo {
  std::string ip;
  OsatConfig cfg;

  ClientInfo(const std::string& ce, OsatConfig cfg) :
    ip(ce),
    cfg(cfg)
  { }
};

//vector only for multiple clients with same id (handles as one osat)
using client_list_container = std::map<uint8_t, std::map<std::string, ClientInfo>>;

class ClientList{
public:
  ClientList()  = default;
  ~ClientList() = default;

  ClientList(const ClientList&) = default;
  ClientList(ClientList&&)      = default;

  ClientList& operator=(const ClientList&) = default;
  ClientList& operator=(ClientList&&)      = default;
  
  std::size_t getNumClients() const
  {
    return _clients.size();
  }

  std::map<std::string, ClientInfo> front() const
  {
    return _clients.begin()->second;
  }

  std::vector<uint8_t> getIds() const
  {
    std::vector<uint8_t> ids;
    
    for(auto& e : _clients)
    {
      ids.push_back(e.first);
    }
    
    return ids;
  }

  bool contains(const uint8_t id) const
  {
    try
    {
      _clients.at(id);
    }
    catch(const std::out_of_range& e)
    {
      return false;
    }
    return true;
  }

  bool contains(const uint8_t id, const std::string& ip) const
  {
    try
    {
      _clients.at(id).at(ip);
    }
    catch(const std::out_of_range& e)
    {
      return false;
    }
    return true;
  }

  bool insert(const uint8_t id, const ClientInfo& ci)
  {
    if(this->contains(id, ci.ip))
    {
      return false; //nothing to insert
    }

    //prove is id is already in list
    if(this->contains(id))
    {
      //insert in existing id
      _clients.at(id).insert(std::make_pair(ci.ip, ci));
    }
    else
    {
      //insert new id and new client
      std::map<std::string, ClientInfo> ci_map;
      ci_map.insert(std::make_pair(ci.ip, ci));
      _clients.insert(std::make_pair(id, ci_map));
    }
    return true;
  }

  void erase(const uint8_t id, const std::string& ip)
  {
    try
    {
      if(_clients.at(id).size() > 1)
      {
        //erase only one client
        _clients.at(id).erase(ip);
      }
      else
      {
        //only one client there so erase all
        _clients.erase(id);
      }
      
    }
    catch(const std::out_of_range& e)
    {
      return;
    }
  }

  void erase(const uint8_t id)
  {
    _clients.erase(id);
  }

  std::map<std::string, ClientInfo> at(const uint8_t id)
  {
    if(this->contains(id))
    {
      return _clients.at(id);
    }
    return std::map<std::string, ClientInfo>();
  }

  std::optional<ClientInfo> at(const uint8_t id, const std::string& ip)
  {
    if(this->contains(id, ip))
    {
      return _clients.at(id).at(ip);
    }
    return {};
  }

  client_list_container& list()
  {
    return _clients;
  }

  void update_config(const OsatConfig& cfg, const uint8_t id, const std::string& ip)
  {
    if(!this->contains(id, ip))
    {
      //nothing to do
      return;
    }

    _clients.at(id).at(ip).cfg = cfg;

  }

  std::string to_string() const
  {
    std::string str("ClientList:\n");
    for(auto& e : _clients)
    { 
      std::cout << "id: " << (int)e.first << std::endl;
      for(auto& c : e.second)
      {
        std::cout << "  client: " << c.second.ip << std::endl;
      }
    }
    return str;
  }

  client_list_container& get_container()
  {
    return _clients;
  }

private:
  client_list_container _clients;

};


} //namespace osat

#endif  //CLIENTLIST_H_
