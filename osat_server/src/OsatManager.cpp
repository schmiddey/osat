#include "OsatManager.h"

namespace{
  const uint32_t OSAT_SERVER_PORT = 1337;
}

namespace osat{


OsatManager::OsatManager() :
  _tcp_server(OSAT_SERVER_PORT)
{
  _tcp_server.attach_read_callback(std::bind(&OsatManager::tcp_in_callback, this, std::placeholders::_1, std::placeholders::_2));
}

OsatManager::~OsatManager()
{ }

void OsatManager::start()
{
  std::cout << "start tcp" << std::endl;
  _tcp_server.start();
  std::cout << "find clients: " << std::endl;
  // Duration d(1.0);
  this->findClients();
}

void OsatManager::stop()
{
  _run = false;
}


void OsatManager::spin()
{
  // _tcp_server.spin();
  while(_run.load())
  {
    _tcp_server.spinOnce();
    _mode->tick();
    //delay
    std::this_thread::sleep_for(std::chrono::microseconds(1000));
  }
}

void OsatManager::spinOnce()
{
  _tcp_server.spinOnce();
  if(_mode)
  {
    _mode->tick();
  }
}

void OsatManager::startMode(std::weak_ptr<Mode> mode)
{
  if(mode.expired())
  {
    std::cout << "Warning: given mode is expired..." << std::endl;
    return;
  }

  _mode = mode.lock();

  //attach callbacks
  _mode->attach_state_change_callback(std::bind(&OsatManager::setOsatState, this, std::placeholders::_1, std::placeholders::_2));
  _mode->attach_mode_ready_callback(std::bind(&OsatManager::modeRdy_callback, this, std::placeholders::_1, std::placeholders::_2));
  _mode->attach_change_timeout_callback(std::bind(&OsatManager::setOsatTimeout, this, std::placeholders::_1, std::placeholders::_2));

  _mode->start();

}

void OsatManager::stopMode(const std::string& mode_name)
{
  _mode->reset();
}

void OsatManager::modeRdy_callback(const std::string& mode_name, const Report& rep)
{
  std::cout << "Mode Rdy: " << rep << std::endl;
}


ClientList& OsatManager::getClientList() 
{
  return _clients;
}

void OsatManager::setOsatState(const uint8_t id, const state::OsatState state)
{
  if(!_clients.contains(id))
  {
    std::cout << "Requested id not available" << std::endl;
    return;
  }
  for(auto& e : _clients.at(id))
  {
    OsatCommand::write_to(e.first, OsatCommand::create_set_state_cmd(state));
  }
}


void OsatManager::setOsatTimeout(const uint8_t id, const uint32_t timeout_ms)
{
  if(!_clients.contains(id))
  {
    std::cout << "Requested id not available" << std::endl;
    return;
  }

  for(auto& e : _clients.at(id))
  {
    OsatCommand::write_to(e.first, OsatCommand::create_cfg_timeout_cmd(timeout_ms));
  }
}


void OsatManager::setConfig(const uint8_t id, const OsatConfig& cfg)
{
  if(!_clients.contains(id))
  {
    std::cout << "Requested id not available" << std::endl;
    return;
  }


  for(auto& e : _clients.at(id))
  {
    //send cfg
    //only send if something is different than old cfg
    
    //send new id
    if(e.second.cfg.id != cfg.id)
    {
      OsatCommand::write_to(e.first, OsatCommand::create_cfg_id_cmd(cfg.id));
    }
    //send new thresh
    if(e.second.cfg.hit_thresh != cfg.hit_thresh)
    {
      OsatCommand::write_to(e.first, OsatCommand::create_cfg_thresh_cmd(cfg.hit_thresh));
    }
    //for now dont set timeout in default config set.... is set by mode
    // //send new timeout
    // if(e.second.cfg.timeout_ms != cfg.timeout_ms)
    // {
    //   OsatCommand::write_to(e.first, OsatCommand::create_cfg_timeout_cmd(cfg.timeout_ms));
    // }
  }

}

std::optional<OsatConfig> OsatManager::getConfig(const uint8_t id)
{
  if(!_clients.contains(id))
  {
    return std::nullopt;
  }
  //return config //if multiple clients per id just return first one (should all the same by definition)
  return _clients.at(id).begin()->second.cfg;
}



void OsatManager::findClients()
{
  std::cout << "try finding active Clients..." << std::endl;
  std::vector<std::thread> thrds(253);
  std::atomic<std::size_t> num_thrds = 0;
  Duration d(0.005);
  for(unsigned int i = 2; i < 255; i++)
  {
    std::string ip = "192.168.5." + std::to_string(i);
    while(num_thrds.load() >= 100)
    {
      d.sleep();
    }
    // std::cout << "ip: " << ip << std::endl;
    thrds[i-2] = std::thread([&num_thrds, ip]() {
      num_thrds++;
      //request config
      OsatCommand::write_to(ip, OsatCommand::create_cfg_request());
      num_thrds--;
    });

    thrds[i-2].detach();
  }

  while(num_thrds.load() != 0)
  {
    d.sleep();
  }


  std::cout << "running threads: " << num_thrds.load() << std::endl;
  std::cout << "searching for clients rdy" << std::endl;
}

void OsatManager::tcp_in_callback(const osat::server::ClientEndpoint& ce, const std::string& data)
{
  //extract id from data
  std::cout << "-----------          data: " << data << std::endl; 

  std::string id_str  = data.substr(0, data.find('|'));
  std::string payload = data.substr(data.find('|') +1, data.length());
  
  std::cout << "id_str: " << id_str << std::endl;
  std::cout << "payload: " << payload << std::endl;
  
  uint8_t id = 0;
  try{
    id = boost::lexical_cast<uint32_t>(id_str);
  }catch(const boost::bad_lexical_cast& e)
  {
    std::cerr << "error at received data: " << e.what() << '\n';
    return;
  }
  

  //only init if config is received
  if(OsatConfig::isConfig(payload))
  {
    //parse config
    OsatConfig cfg(payload);
    if(!cfg)
    {
      //nothing to do ... broken config;
      return;
    }
    ClientInfo ci(ce.ip, cfg);
    //prove if client is in list if not insert it
    if(_clients.insert(id, ci))
    {
      std::cout << "Got new Client: " << ce.to_string() << ", with config: " << cfg << std::endl;

      //show current client list
      std::cout << "Current client list: " << _clients.to_string() << std::endl;
      return;
    }

    //alread in list -> update config
    _clients.update_config(cfg, id, ce.ip);
    return;
  }
  else
  {
    //is client not in list and no Config is sent then request config
    if(!_clients.contains(id, ce.ip))
    {
      //request config
      if(!OsatCommand::write_to(ce.ip, OsatCommand::create_cfg_request()))
      {
        std::cout << "Unable to write Msg to: " << ce.ip << std::endl;
      }
    }
  }
  if(!_mode)
  {
    return;
  }
  //parse other commands
  if(payload == std::string("hit"))
  {
    _mode->setAction(id, action::HIT);
  }
  else if(payload == std::string("timeout"))
  {
    _mode->setAction(id, action::TIMEOUT);
  }
  else if(payload == std::string("Got invalid Command ..."))
  {
    std::cout << "Warning -> Osat with id: " << id << ", ip: " << ce.ip << " got an ivalid command from server" <<  std::endl;
  }
  else
  {
    //other command only show nothing to do
    std::cout << "Got msg from Osat id: " << id << ", ip: " << ce.ip << " -> : " << data << std::endl;
  }



}




} //namespace osat
