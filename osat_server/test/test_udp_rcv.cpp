

#include <iostream>


#include "../src/ip/Udp.h"


int main(int argc, char const *argv[])
{
  osat::UdpSrvReceive _socket(1337);

  if(!_socket)
  {
    std::cout << "Error at creating Socket will exit..." << std::endl;
    std::exit(EXIT_FAILURE);
  }

  int timeout = 1000000;

  if(argc >= 2)
  {
    timeout = std::stoi(argv[1]);
  }
  
  std::cout << "start receiving with timout: " << timeout << " us" << std::endl;
  std::size_t to_cnt = 0;
  std::size_t cnt = 0;
  while(1)
  {
    try
    {
      auto ret = _socket.receive(100, timeout);
      if(ret)
      {
        auto data = ret.value();
        std::string str_data(data.first.begin(), data.first.end());
        
        // std::cout << "got " << data.first.size() << " bytes, from " << data.second.ip << ":" << data.second.port << " data: " << str_data << std::endl;
        std::cout << "cnt: " << cnt++ << " to_cnt: " << to_cnt <<", got " << data.first.size() << " bytes, from " << data.second.ip << ":" << data.second.port << " data: ";
        if(str_data == "hit")
          std::cout << str_data << "++++++++++++++++++++++++++++" << std::endl;
        else
          std::cout << str_data << std::endl;
      }
      else
      {
        std::cout << "got timeout" << std::endl;
        to_cnt++;
      }

     
    }
    catch(const std::exception& e)
    {
      std::cerr << e.what() << '\n';
    }
  }

  
  
  return 0;
}
