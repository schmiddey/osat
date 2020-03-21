#ifndef MODE_H_
#define MODE_H_

#include <iostream>
#include <cstdint>
#include <vector>
#include <map>

#include <functional>

#include <fstream>
#include <iomanip>

#include "Timer.h"
#include "osat_types.h"

#include "thrdparty/json.hpp"
#include "thrdparty/random.hpp"


namespace cppjson = nlohmann;

namespace osat{



namespace cfg{

struct Mode
{
  std::string           mode_type               = "random_n";
  std::string           mode_name               = "hans";
  std::vector<uint32_t> ids                     = { 1u, 2u, 3u, 4u };
  uint32_t              timeout_ms              = 1000u;
  uint32_t              interval_ms             = 1500u;
  uint32_t              interval_reduce_step_ms = 0u;
  uint32_t              max_timeout_count       = 10u;
  uint32_t              activate_count          = 1u;

  //write config //json
  bool writeConfig(const std::string& cfg_file)
  {
    cppjson::json j_cfg;
    j_cfg["mode_type"]               = mode_type;
    j_cfg["mode_name"]               = mode_name;
    j_cfg["ids"]                     = ids;
    j_cfg["timout_ms"]               = timeout_ms;
    j_cfg["interval_ms"]             = interval_ms;
    j_cfg["interval_reduce_step_ms"] = interval_reduce_step_ms;
    j_cfg["max_timeout_count"]       = max_timeout_count;
    j_cfg["activate_count"]          = activate_count;

    std::ofstream os;
    os.open(cfg_file);
    if(!os)
    {
      std::cout << "Unable to write Config" << std::endl;
      return false;
    }

    os << std::setw(2) << j_cfg;
    os.close();
    return true;
  }
  //read config  //json
  bool readConfig(const std::string& cfg_file)
  { 
    std::ifstream is;
    is.open(cfg_file);
    if(!is)
    {
      std::cout << "Unable to read Config" << std::endl;
      return false;
    }
    cppjson::json j_cfg;
    is >> j_cfg;
    mode_type               = j_cfg["mode_type"];
    mode_name               = j_cfg["mode_name"];
    ids                     = j_cfg["ids"].get<std::vector<uint32_t>>();
    timeout_ms              = j_cfg["timout_ms"];
    interval_ms             = j_cfg["interval_ms"];
    interval_reduce_step_ms = j_cfg["interval_reduce_step_ms"];
    max_timeout_count       = j_cfg["max_timeout_count"];
    activate_count          = j_cfg["activate_count"];

    return true;
  }
};

} //namespace cfg




namespace action{
  enum OsatAction{

    HIT = 0,
    TIMEOUT
  };
} //namespace action


struct Report{
  uint32_t hit_cnt;
  uint32_t timeout_cnt;
  Duration duration_total;

  Report(const uint32_t hit_cnt_, const uint32_t timeout_cnt_, const Duration& dur_total_) :
    hit_cnt(hit_cnt_),
    timeout_cnt(timeout_cnt_),
    duration_total(dur_total_)
  { }
};

/**
 * @brief 
 * 
 */
class Mode{
public:
  Mode()
  {
    //set default cb
    _state_change_cb   = std::bind(&Mode::default_state_change_callback, this, std::placeholders::_1, std::placeholders::_2);
    _mode_rdy_cb       = std::bind(&Mode::default_ready_callback, this, std::placeholders::_1, std::placeholders::_2);
    _change_timeout_cb = std::bind(&Mode::default_change_timeout_callback, this, std::placeholders::_1, std::placeholders::_2);
  }

  Mode(const cfg::Mode& cfg) : Mode()
  {
    //init states:
    this->updateConfig(cfg);
  }

  Mode(const std::string& path_to_cfg) : Mode()
  {
    if(!_cfg.readConfig(path_to_cfg))
    {
      std::cout << "Warning: unable to read Config: " << path_to_cfg << ", will use default config" << std::endl;
    }
    this->updateConfig(_cfg);
  }
  


  virtual ~Mode() {}

  void attach_state_change_callback(std::function<void (const uint8_t id, const state::OsatState state)> f)
  {
    _state_change_cb = f;
  }

  void attach_mode_ready_callback(std::function<void (const std::string&, const Report&)> f)
  {
    _mode_rdy_cb = f;
  }

  void attach_change_timeout_callback(std::function<void (const uint8_t, const uint32_t)> f)
  {
    _change_timeout_cb = f;
  }

  cfg::Mode getConfig() const
  {
    return _cfg;
  }
  
  void updateConfig(const cfg::Mode cfg)
  {
    _cfg = cfg;
    //just reinit all states
    _states.clear();
    for(auto& e : _cfg.ids)
    {
      _states.insert(std::make_pair(e, state::INACTIVE));
    }

    // this->reset();
  }

  std::vector<uint32_t> getRandomIds(const uint8_t num, const state::OsatState state_reqired = state::ANY)
  {
    std::vector<uint32_t> rnd_ids;
    uint32_t cnt = 0;
    uint32_t cnt_max = _cfg.ids.size() * num; //just as backup for wrong configuration
    while(rnd_ids.size() < num)
    {
      uint32_t rnd = *(effolkronium::random_static::get(_cfg.ids.begin(), _cfg.ids.end()));
      bool rnd_ok = true;
      for(auto& e : rnd_ids)
      {
        if(e == rnd)
        {
          rnd_ok = false;
          break;
        }
      }

      //check state
      if(_states.at(rnd) == state_reqired || state_reqired == state::ANY)
      {
        if(rnd_ok)
        {
          rnd_ids.push_back(rnd);
        }
      }

      //just as backup for wrong configuration
      if(cnt++ > cnt_max)
      {
        break;
      }
    }
    return rnd_ids;
  }

  bool isRunning() const
  {
    return _running;
  }

  virtual void setAction(const uint32_t id, const action::OsatAction action) = 0;

  virtual void tick() = 0;

  virtual void start() = 0;

  virtual void reset() = 0;


private: //functions


  void default_state_change_callback(const uint8_t id, const state::OsatState state)
  {
    std::cout << "Warning: no state_change_callback attached..." << std::endl;
  }

  void default_ready_callback(const std::string& mode_name, const Report& report)
  {
    // std::cout << "report: " << report << std::endl;
    std::cout << "Warning no mode_rdy_callback attached..." << std::endl;
  }

  void default_change_timeout_callback(const uint8_t id, const uint32_t to)
  {
    std::cout << "Warning no change timeout callback attachted" << std::endl;
  }

protected:
  void change_state(const uint8_t id, const state::OsatState state)
  {
    _states.at(id) = state;
    _state_change_cb(id, state); 
  }
protected:
  std::function<void (const uint8_t id, const state::OsatState state)> _state_change_cb;
  std::function<void (const std::string&, const Report&)> _mode_rdy_cb;
  std::function<void (const uint8_t, const uint32_t)> _change_timeout_cb;
  
  cfg::Mode _cfg;

  std::map<uint8_t, state::OsatState> _states;

  bool _running = false;
};



} //namespace osat

//ostream for Report and mode config

//cfg
  // std::vector<uint32_t> ids                    = {1u, 2u, 3u, 4u};
  // uint32_t             timeout_ms              = 1000u;
  // uint32_t             interval_ms             = 1000u;
  // uint32_t             interval_reduce_step_ms = 0u;
  // uint32_t             max_timeout_count       = 10u;
  // uint32_t             activate_count          = 1u;

inline std::ostream& operator<<(std::ostream &out, const osat::cfg::Mode& cfg)
{
  out << "ModeCfg:" << std::endl;
  out << "  mode_type: " << cfg.mode_type << std::endl;
  out << "  mode_name: " << cfg.mode_name << std::endl;
  out << "  ids: ";
  for(auto& e : cfg.ids)
  {
    out << e << " ";
  }
  out << std::endl;
  out << "  timeout_ms: " << cfg.timeout_ms << std::endl;
  out << "  interval_ms: " << cfg.interval_ms << std::endl;
  out << "  interval_reduce_step_ms: " << cfg.interval_reduce_step_ms << std::endl;
  out << "  max_timeout_count: " << cfg.max_timeout_count << std::endl;
  out << "  activate_count: " << cfg.activate_count << std::endl;
  return out;
}

//report
  // uint32_t hit_cnt;
  // uint32_t timeout_cnt;
  // Duration duration_total;

inline std::ostream& operator<<(std::ostream &out, const osat::Report& rep)
{
  out << "Report: " << std::endl;
  out << "  hit_count: " << rep.hit_cnt << std::endl;
  out << "  timeout_cnt: " << rep.timeout_cnt << std::endl;
  out << "  duration_total: " << rep.duration_total << std::endl;
  return out;
}

#endif  //MODE_H_
