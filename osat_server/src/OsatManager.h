#ifndef OSATMANAGER_H_
#define OSATMANAGER_H_




#include <iostream>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>

#include <boost/lexical_cast.hpp>

#include "osat_types.h"
#include "OsatConfig.h"
#include "Mode.h"
#include "ClientList.h"


#include "ip/Tcp.h"

namespace osat{




/**
 * @brief 
 * Comm protocoll for receiving... (All cmds are sended as string...)
 * Commands:
 * "CMD-ACTIVE"    -> Activates OSAT -> waiting for hit... no timeout
 * "CMD-ACTIVE_TO" -> Activates Osat -> waiting for hit, but sets it inactive after timout (from cfg)
 * "CMD-INACTIVE"  -> Deactivates OSAT
 * "CMD-FLASH"     -> Activates Flash Mode -> Flashes when hit
 * 
 * Configs: (All configs are permanently saved in EEPROM)
 * "CFG-ID-xxx"       -> Sets ID of this Module (1..255) 255 default
 * "CFG-THRESH-xxx"   -> Sets hit-threshold for this Module
 * "CFG-TIMEOUT-xxx"  -> Sets timeout in [ms] for ACTIVE_TO mode
 * "CFG-REQUEST"      -> Sends current cfg
 * 
 * 
 */
class OsatCommand{
public:
  static std::string create_set_active_cmd()
  {
    return std::string("CMD-ACTIVE");
  }

  static std::string create_set_active_to_cmd()
  {
    return std::string("CMD-ACTIVE_TO");
  }

  static std::string create_set_inactive_cmd()
  {
    return std::string("CMD-INACTIVE");
  }

  static std::string create_set_flash_cmd()
  {
    return std::string("CMD-FLASH");
  }

  static std::string create_set_state_cmd(const state::OsatState state)
  {
    switch(state) {
      case state::ACTIVE:
        return create_set_active_cmd();
      case state::ACTIVE_TO:
        return create_set_active_to_cmd();
      case state::INACTIVE:
        return create_set_inactive_cmd();
      case state::FLASH:
        return create_set_flash_cmd();
      default:
        return std::string("invalid_cmd");
    }
  }

  static std::string create_cfg_id_cmd(const uint8_t id)
  {
    return std::string("CFG-ID-") + std::to_string(id);
  }

  static std::string create_cfg_thresh_cmd(const uint16_t thresh)
  {
    return std::string("CFG-THRESH-") + std::to_string(thresh);
  }

  static std::string create_cfg_timeout_cmd(const uint16_t timeout)
  {
    return std::string("CFG-TIMEOUT-") + std::to_string(timeout);
  }

  static std::string create_cfg_request()
  {
    return std::string("CFG-REQUEST");
  }

  static bool write_to(const std::string& ip, const std::string& msg)
  {
    std::cout << "++++++++++++++++++++++++++++++++++++++++ sending to: " << ip << ", ->" << msg << std::endl;
    return client::TcpClient::writeOnce(ip, 1335, msg);
  }
};


class OsatManager{
public:
  OsatManager();
  ~OsatManager();
  
  void start();

  void stop();

  void spin();

  void spinOnce();

  void startMode(std::weak_ptr<Mode> mode);

  void stopMode(const std::string& mode_name);

  void modeRdy_callback(const std::string& mode_name, const Report& rep);

  ClientList& getClientList();

  void setOsatState(const uint8_t id, const state::OsatState);

  void setOsatTimeout(const uint8_t id, const uint32_t timeout_ms);

  void setConfig(const uint8_t id, const OsatConfig& cfg);

  std::optional<OsatConfig> getConfig(const uint8_t id);

  std::size_t getNumClients() const { return _clients.getNumClients(); }

  void findClients();

private: //functions

  void tcp_in_callback(const osat::server::ClientEndpoint& cl, const std::string& data);

private:
  server::TcpServer _tcp_server;

  ClientList _clients;

  std::shared_ptr<Mode> _mode; //for now, only one mode at a time possible

  std::atomic<bool> _run = true;
};


} //namespace osat


#endif  //OSATMANAGER_H_
