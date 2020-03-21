#include <iostream>
#include <csignal>
#include <atomic>


#include "../ip/Tcp.h"

#include "../OsatConfig.h"
#include "../OsatManager.h"
#include "../Mode.h"
#include "../ModeHandler.h"

std::atomic<bool> g_run = true;

osat::server::TcpServer g_srv(1333);
osat::OsatManager g_osat_manager;

osat::ModeHandler g_mode_handler;

void signal_handler(int signum)
{
  g_run = false;
}



//g_srv callback
void tcp_read_cb(const osat::server::ClientEndpoint& cl, const std::string& data)
{
  std::cout << "msg from: " << cl.to_string() << std::endl;
  std::string cmd    = data.substr(0, data.find(' '));
  std::cout << "cmd: " << cmd << std::endl;
  std::string remain = data.substr(data.find(' ') + 1);
  std::cout << "remain: " << remain << std::endl;
  
  if(cmd == "load")
  {
    //remain is path to json config
    g_srv.write_to(cl, "try to create Mode with given config: " + remain);
    if(g_mode_handler.create(remain).expired())
    {
      g_srv.write_to(cl, "unable to create Mode...");
      return;
    }
    g_srv.write_to(cl, "Laded Mode successfully");
  }
  else if(cmd == "start")
  {
    g_osat_manager.startMode(g_mode_handler.get(remain));
    g_srv.write_to(cl, "started Mode" + remain);
  }
  else if(cmd == "stop")
  {
    g_osat_manager.stopMode(remain);
    g_srv.write_to(cl, "stoppend Mode: " + remain);
  }
  else if(cmd == "create_mode_cfg")
  {
    osat::cfg::Mode cfg; //with default values
    if(cfg.writeConfig(remain))
    {
      g_srv.write_to(cl, "written config to: " + remain);
    }
    else
    {
      g_srv.write_to(cl, "unable to write config to: " + remain);
    }
  }
  else if(cmd == "show")
  {
    g_srv.write_to(cl, "ClientList: ");
    for(auto& e :  g_osat_manager.getClientList().get_container())
    {
      g_srv.write_to(cl, "id: " + std::to_string((int)e.first) );
      std::cout << "id: " << (int)e.first << std::endl;
      for(auto& c : e.second)
      {
        g_srv.write_to(cl, "  client: " + c.second.ip );
        std::cout << "  client: " << c.second.ip << std::endl;
      }
    }
  }
  else if(cmd == "modes")
  {
    g_srv.write_to(cl, "Modes: ");
    for(auto& e : g_mode_handler.getModes())
    {
      std::string msg = "name: " + e.second->getConfig().mode_name;
      g_srv.write_to(cl, msg);
    }
  }
  else //help
  {
    //send possible coommands
    g_srv.write_to(cl, "Possible Commands:");
    g_srv.write_to(cl, "load <path/to/cfg.json>  //loads Mode by given config");
    g_srv.write_to(cl, "start <mode_namme> //starts Mode by mode name -> defined in config");
    g_srv.write_to(cl, "stop <mode_name> //stops mode by mode name -> defined in config");
    g_srv.write_to(cl, "create_mode_cfg <path/to/cfg.json> //creates a default config file at given name");
    g_srv.write_to(cl, "show // display all connected osat...");
    g_srv.write_to(cl, "<somthing else> -> will show this");
  }
  

  //send back
  // g_srv.write_to(cl, "hallo");
  // if(!osat::client::TcpClient::writeOnce(cl.ip, 1222, "hallo"))
  // {
  //   std::cout << "Error sending msg to: " << cl.to_string() << std::endl;
  // }
}

int main(int argc, char const *argv[])
{
  std::signal(SIGINT, signal_handler);

  std::cout << "create server for ctrl:" << std::endl;
  g_srv.attach_read_callback(std::bind(tcp_read_cb, std::placeholders::_1, std::placeholders::_2));
  g_srv.start();
  

  std::cout << "starting osat manager" << std::endl;

  g_osat_manager.start();

  std::cout << "wait for action" << std::endl;
  while(g_run.load())
  {
    g_osat_manager.spinOnce();
    g_srv.spinOnce();
    std::this_thread::sleep_for(std::chrono::microseconds(1000));
  }

  return 0;
}
