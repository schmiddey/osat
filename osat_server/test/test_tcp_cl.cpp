#include <iostream>

#include "../src/boost/Tcp.h"

void read_callback(const std::string& data)
{
  std::cout << "got data: " << data << std::endl;
} 

int main(int argc, char const *argv[])
{
  osat::client::TcpClient cl("127.0.0.1", 1339);

  cl.connect();
  std::cout << "connected" << std::endl;
  cl.start();

  std::cout << "started read thrd... write msg" << std::endl;
  cl.write("shit!!!");

  usleep(1000000);
  cl.stop();

  std::cout << "written msg" << std::endl;
  for(unsigned int i = 0; i < 10000; i++)
  {
    // cl.spinOnce();
    ::usleep(500);
  }

  return 0;
}
