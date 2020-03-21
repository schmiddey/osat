#include <iostream>

#include <csignal>
#include <thread>
#include <mutex>

#include "../OsatConfig.h"
#include "../OsatManager.h"

uint8_t id = 0;
std::mutex mtx_osat;

osat::OsatManager osat_manager;

void signal_handler(int signum)
{
  ::exit(signum);
}

void thread_write(void)
{
  //wait for user intput
  getchar();
  
  mtx_osat.lock();
  
  std::cout << "Writing config to: " << std::endl;

  if(!osat_manager.getNumClients())
  {
    std::cout << "No clients connected..." << std::endl;
    ::exit(EXIT_FAILURE);
  }
  //get config from first one in map or with target id
  auto cfg = osat_manager.getConfig(id);
  if(!cfg)
  {
    cfg = osat_manager.getClientList().front().begin()->second.cfg;
  }
  
  auto ids = osat_manager.getClientList().getIds();
  
  cfg.value().id = id;
  for(auto& e : ids)
  {
    osat_manager.setConfig(e, cfg.value());
  }

  mtx_osat.unlock();
  ::exit(EXIT_SUCCESS);
} 


int main(int argc, char const *argv[])
{
  //parse cmd args
  if(argc != 2)
  {
    std::cout << "Wrong numer of arguments... usage: " << argv[0] << " <id>" << std::endl;
    ::exit(EXIT_FAILURE);
  }

  uint32_t tmp_id = std::stoi(argv[1]);

  if(tmp_id > 255 || tmp_id == 0)
  {
    std::cout << "invalid id given...  id range:  1..255" << std::endl;
    ::exit(EXIT_FAILURE);
  }

  id = tmp_id;

  std::cout << "This tool will set the given ID: " << (int) id << " to all connected osat clients. You must wait for all osats to connect and press then enter to set ID" << std::endl;


  std::signal(SIGINT, signal_handler);


  std::thread thd(thread_write);

  osat_manager.start();
  
  while(1)
  {
    mtx_osat.lock();
    osat_manager.spinOnce();
    mtx_osat.unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }



  return 0;
}
