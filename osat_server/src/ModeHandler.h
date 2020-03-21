#ifndef MODEHANDLER_H_
#define MODEHANDLER_H_

#include <iostream>
#include <map>
#include <memory>


#include "Mode.h"

//include all Modes
#include "Mode_RandomN.h"
#include "Mode_RandomFill.h"
#include "Mode_ReverseFill.h"


namespace osat{


/**
 * @brief Creates and holds Mode (creates them only by jsdon config)
 * 
 */
class ModeHandler{
public:
  ModeHandler()
  { }
  ~ModeHandler()
  { }

  std::weak_ptr<Mode> create(const std::string& path_to_cfg)
  {
    //todo !!! create different modes by config!!!!
    //parse config
    cfg::Mode cfg;
    if(!cfg.readConfig(path_to_cfg))
    {
      std::cout << "Warning: Unable to read config for mode: " << path_to_cfg << std::endl;
      return std::weak_ptr<Mode>();
    }
    if(cfg.mode_type == "random_n")
    {
      std::cout << "Create random_n" << std::endl;
      _modes.insert(std::make_pair(cfg.mode_name, std::make_shared<Mode_RandomN>(cfg)));
    }
    else if(cfg.mode_type == "random_fill")
    {
      std::cout << "Create random_fill" << std::endl;
      _modes.insert(std::make_pair(cfg.mode_name, std::make_shared<Mode_RandomFill>(cfg)));
    }
    else if(cfg.mode_type == "reverse_fill")
    {
      std::cout << "Create  reverse_fill" << std::endl;
      _modes.insert(std::make_pair(cfg.mode_name, std::make_shared<Mode_ReverseFill>(cfg)));
    }
    else
    {
      std::cout << "Warning: Config broken -> mode_type wrong... will use random_n" << std::endl;
      cfg.mode_type = "random_n";
      _modes.insert(std::make_pair(cfg.mode_name, std::make_shared<Mode_RandomN>(cfg)));
    }

    return _modes.at(cfg.mode_name);
  }

  std::weak_ptr<Mode> get(const std::string& mode_name)
  {
    try
    {
      return _modes.at(mode_name);
    }
    catch(const std::out_of_range& e)
    {
      return std::weak_ptr<Mode>();
    }
  }

  std::map<std::string, std::shared_ptr<Mode>>& getModes()
  {
    return _modes;
  }

private:
  std::map<std::string, std::shared_ptr<Mode>> _modes;
};


} //namespace osat

#endif  //MODEHANDLER_H_