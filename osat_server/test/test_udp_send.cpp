

#include <iostream>


#include "../src/ip/Udp.h"


int main(int argc, char const *argv[])
{
  
  if(argc < 6)
  {
    std::cout << "to less arguments ... will exit, usage: " << argv[0] << "<ip> <port> <num_send> <delay> <your message for udp>"  << std::endl;
    std::exit(EXIT_FAILURE);
  }


  std::string ip = argv[1];
  uint32_t port  = std::stoi(argv[2]);
  int num        = std::stoi(argv[3]);
  int delay      = std::stoi(argv[4]);

  std::string data;




  for(int i = 5; i < argc; i++)
  {
    data += argv[i];
    data += " ";
  }

  data+='\n';

  std::cout << "Sent " << num << "x the given msg: " << data << "to: "  << ip << ":" << port << std::endl;



  osat::UdpClSend _socket(ip, port);
  // osat::UdpClSend _socket("10.42.0.51", 1338);
  if(!_socket)
  {
    std::cout << "Error at creating udpsocket... wil exit.." << std::endl;
    std::exit(EXIT_FAILURE);
  }

  for(int i = 0; i < num; i++)
  {
    std::cout << "sending " << i << ". msg..." << std::endl;
    _socket.transmit(data);
    usleep(delay);
  }

  std::cout << "rdy" << std::endl;
  return 0;
}
