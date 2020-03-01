#include <iostream>

#include "../src/boost/Tcp.h"

osat::server::TcpServer server(1337, 100);


void read_cb(const osat::server::ClientEndpoint& cl, const std::string& data)
{
  std::cout << "msg from " << cl.to_string() << ": " << data  << " -- (size " << data.size() << ")" << std::endl;
}

int main(int argc, char const *argv[])
{
  server.attach_read_callback(std::bind(read_cb, std::placeholders::_1, std::placeholders::_2));
  server.start();


  server.spin();

  return 0;
}
