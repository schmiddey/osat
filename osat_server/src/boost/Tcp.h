#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <functional>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <stdexcept>

#include <boost/asio.hpp>


namespace osat{

class Helper
{
public:
  static std::string check_eol(const std::string& data)
  {
    if(data.back() != '\n')
    {
      return data + '\n';
    }
    return data;
  }
};

namespace server{


struct ClientEndpoint{
  std::string ip;
  uint32_t    port;

  //for usage as key in unordered_map
  bool operator==(const ClientEndpoint& ce) const
  {
    return ( ip == ce.ip && port == ce.port);
  }

  std::string to_string() const
  {
    return ip + ":" + std::to_string(port);
  }

  ClientEndpoint(const std::string& ip, const uint32_t port) : 
    ip(ip),
    port(port)
  { }


  ClientEndpoint(const boost::asio::ip::tcp::endpoint& ep) :
    ip(ep.address().to_string()),
    port(ep.port())
  { }

  ClientEndpoint(const ClientEndpoint& other) = default;
  ClientEndpoint(ClientEndpoint&& other) = default;
  ClientEndpoint& operator=(const ClientEndpoint& other) = default;
  ClientEndpoint& operator=(ClientEndpoint&& other) = default;

};

namespace hash{

/**
 * @brief 
 * @todo prove if hash is ok so...
 */
struct ClientEndpointHasher{
  std::size_t operator()(const ClientEndpoint& key) const
  {
    return ((std::hash<std::string>()(key.ip)
           ^(std::hash<uint32_t>()   (key.port) << 1)));
  }
};

} //namespace hash


struct TcpServerClient{
  boost::asio::ip::tcp::socket socket;
  ClientEndpoint cl_endpoint;
  std::thread thrd;

  TcpServerClient() = delete;
  TcpServerClient(boost::asio::io_service& io_service, 
                  const ClientEndpoint& ce,
                  const std::function<void(const ClientEndpoint&)> close_cb) :
    socket(io_service),
    cl_endpoint(ce),
    _close_cb(close_cb)
  { 
    _data_cb = std::bind(&TcpServerClient::default_data_cb, this, std::placeholders::_1, std::placeholders::_2);
  }

  void attach_data_callback(const std::function<void(const std::string&, const ClientEndpoint&)> cb)
  {
    _data_cb = cb;
  }

  void start()
  {
    thrd = std::thread(&TcpServerClient::thrd_read, this);
    thrd.detach();
  }

  void stop()
  {
    _run = false;
    socket.close();
  }

private:
  void thrd_read()
  {
    while(_run.load())
    {
      boost::asio::streambuf buffer;
      try
      {
        boost::asio::ip::tcp::no_delay option(true);
        socket.set_option(option);
        boost::asio::read_until(socket, buffer, "\n");
      }
      catch(const boost::system::system_error& e)
      {
        // std::cerr << e.what() << '\n';
        //close socket at error...
        this->stop();
        //call close cb
        _close_cb(cl_endpoint);
        break;
      }
      
      std::string data = boost::asio::buffer_cast<const char*>(buffer.data());
      data.erase(std::remove(data.begin(), data.end(), '\n'), data.end());
      // std::cout << "data: " << data << std::endl;
      //execute callback
      _data_cb(data, cl_endpoint);
    }
  }

  void default_data_cb(const std::string& data, const ClientEndpoint& ce)
  {
    std::cout << "debug: default cb from client obj " << cl_endpoint.to_string() << ": " << data << std::endl;
  }

  std::atomic<bool> _run = true;
  std::function<void(const std::string&, const ClientEndpoint&)> _data_cb;
  const std::function<void(const ClientEndpoint&)> _close_cb;
};


using client_map = std::unordered_map<ClientEndpoint, std::unique_ptr<TcpServerClient>, hash::ClientEndpointHasher>;


/**
 * @brief 
 * 
 * @todo find way to detect closing client socket...
 * @todo send receive not only string but also raw data e.g. std::vector<uint8_t> or std::any? or void*?
 * @todo 
 * 
 */
class TcpServer{
public:
  TcpServer(const uint32_t port, const std::size_t max_num_clients = 1) :
    _acceptor(_io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
    _port(port),
    _max_num_clients(max_num_clients)
  {
    _read_callback_str = std::bind(&TcpServer::default_read_callback, this, std::placeholders::_1, std::placeholders::_2);
  }
  ~TcpServer()
  { }

  void start()
  {
    //start attaching clients (async)
    _thrd_accept = std::thread(&TcpServer::thrd_accepter, this);
    _thrd_accept.detach();

  }
  
  
  /**
   * @brief 
   * 
   * @param cb 
   * 
   */
  void attach_read_callback(const std::function<void(const ClientEndpoint&, const std::string&)>& cb)
  {
    //attach read callback
    _read_callback_str = cb;
  } 



  void spinOnce()
  {
    while(true)
    {
      _mtx_data_in_queue.lock();
      if(_data_in_queue.empty())
      {
        _mtx_data_in_queue.unlock();
        break;
      }
      auto e = _data_in_queue.front(); //cpy+
      _data_in_queue.pop();
      _mtx_data_in_queue.unlock();
      _read_callback_str(e.first, e.second);
    }

    //handle stoped clients
    while(true)
    {
      _mtx_stopped_cl.lock();
      if(_stoped_clients.empty())
      {
        _mtx_stopped_cl.unlock();
        break;
      }

      bool restart_acceptor = false;
      if(_current_num_clients.load() >= _max_num_clients.load())
      {
        restart_acceptor = true;
      }
      _mtx_cl.lock();
      _clients.erase(_stoped_clients.front());
      _stoped_clients.pop();
      _mtx_stopped_cl.unlock();
      _current_num_clients = _clients.size();
      _mtx_cl.unlock();
      //start accepting again
      if(restart_acceptor)
      {
        this->start(); 
      }
    }
    
  }
  
  void spin()
  {
    while(_run.load())
    {
      this->spinOnce();
      std::this_thread::sleep_for(std::chrono::microseconds(500)); //todo check if time is ok
    }
  }

  /**
   * @brief 
   * 
   * @param cl 
   * @param data 
   * 
   * @todo handle write error ...
   */
  void write_to(const ClientEndpoint& cl, const std::string& data) const
  {
    try
    {
      boost::asio::write(_clients.at(cl)->socket, boost::asio::buffer(Helper::check_eol(data)));
    }
    catch(const std::out_of_range& e)
    {
      std::cerr << "given Client (" << cl.to_string() << ") not available ... do nothing" << std::endl;
    }
  }

private: //functions

  void default_read_callback(const ClientEndpoint& cl, const std::string& data)
  {
    std::cout << "got tcp msg from " << cl.to_string() << ": " << data << std::endl;
    std::cout << "debug: num clients: " << _current_num_clients.load() << std::endl;
  }

  void thrd_accepter()
  {
    if(!_mtx_accept.try_lock())
    {
      //already running
      return;
    }
    while(_current_num_clients.load() < _max_num_clients.load())
    {
      auto cl = std::make_unique<TcpServerClient>(_io_service, ClientEndpoint("no_ip", 0), std::bind(&TcpServer::client_close_callback, this, std::placeholders::_1));
      boost::asio::ip::tcp::no_delay option(true);
      _acceptor.accept(cl->socket);
      cl->socket.set_option(option);
      // std::cout << " hans " << std::endl;
      //create client
      cl->cl_endpoint = ClientEndpoint(cl->socket.remote_endpoint());
      //add to client map
      cl->attach_data_callback(std::bind(&TcpServer::process_client_data, this, std::placeholders::_1, std::placeholders::_2));
      cl->start();
      _mtx_cl.lock();
      _clients.insert(std::make_pair(cl->cl_endpoint, std::move(cl)));
      _current_num_clients = _clients.size();
      _mtx_cl.unlock();
      // std::cout << "debug: got new client, current num cl: " << _current_num_clients.load() << std::endl;

    }
    _mtx_accept.unlock();
  }

  void process_client_data(const std::string& data, const ClientEndpoint ce)
  {
    _mtx_data_in_queue.lock();
    _data_in_queue.push(std::make_pair(ce,data));
    //for debug:
    if(_data_in_queue.size() > 500)
    {
      std::cout << "debug: WARNING: data in queue size > 500!!!! size: " << _data_in_queue.size() << std::endl;
    }
    _mtx_data_in_queue.unlock();
  }

  void client_close_callback(const ClientEndpoint& cle)
  {
    const std::lock_guard<std::mutex> lock(_mtx_stopped_cl);
    _stoped_clients.push(cle);
  }

private: //member obj
  boost::asio::io_service _io_service;
  boost::asio::ip::tcp::acceptor _acceptor;

  std::thread _thrd_accept;

  std::function<void(const ClientEndpoint&, const std::string&)> _read_callback_str;
  //todo do an other cb for raw data or 
  client_map _clients;
  

  std::queue<std::pair<ClientEndpoint, std::string>> _data_in_queue;
  std::queue<ClientEndpoint> _stoped_clients;
  

  const uint32_t    _port;
  const std::atomic<std::size_t> _max_num_clients;

  std::atomic<std::size_t> _current_num_clients = 0;

  std::mutex _mtx_accept;
  std::mutex _mtx_data_in_queue;
  std::mutex _mtx_cl;
  std::mutex _mtx_stopped_cl;

  std::atomic<bool> _run = true;
};  

} //namespace server

namespace client{


class TcpClient{
public:
  TcpClient(const std::string& ip, const uint32_t port, const bool auto_repair = false) :
    _socket(_io_service),
    _ip(ip),
    _port(port)
  {
    _read_cb = std::bind(&TcpClient::default_read_calback, this, std::placeholders::_1);
    _auto_repair = auto_repair;
  }

  ~TcpClient()
  { }

  void connect()
  {
    _socket.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(_ip), _port));
  }

  /**
   * @brief 
   * 
   * @param data 
   * @return true if ok
   * @return false if error
   */
  bool write(const std::string& data)
  {
    boost::system::error_code error;
    boost::asio::write(_socket, boost::asio::buffer(Helper::check_eol(data)), error);
    if(!error)
    {
      return true;
    }
    else
    {
      std::cerr << "error at sending tcp message ...." << std::endl;
      return false;
    }
  }

  void attach_read_callback(const std::function<void(const std::string&)> read_cb)
  {
    _read_cb = read_cb;
  }

  void start()
  {
    _thrd = std::thread(&TcpClient::thrd_read, this);
    _thrd.detach();
  }

  void stop()
  {
    boost::system::error_code ec;
    _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    _socket.close();
    _run = false;
  }

  void spinOnce()
  {
    while(true)
    {
      _mtx_data.lock();
      if(_data_queue.empty())
      {
        _mtx_data.unlock();
        break;
      }
      _read_cb(_data_queue.front());
      _data_queue.pop();
      _mtx_data.unlock();
    }
  }

  void spin()
  {
    while(_run.load())
    {
      this->spinOnce();
      std::this_thread::sleep_for(std::chrono::microseconds(500)); //todo check if time is ok
    }
  }
  
private:
  void default_read_calback(const std::string& data)
  {
    std::cout << "got data: " << data << std::endl;
  }
  /**
   * @brief 
   * 
   */
  void thrd_read()
  {
    while(_run.load())
    {
      boost::asio::streambuf buffer;
      try
      {
        boost::asio::read_until(_socket, buffer, "\n");
      }
      catch(const boost::system::system_error& e)
      {
        std::cerr << e.what() << '\n';
        if(_auto_repair.load())
        {
          boost::system::error_code ec;
          _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
          _socket.close();
          this->connect();
          continue; 
        }  
        break;
      }
      _mtx_data.lock();
      std::string data = boost::asio::buffer_cast<const char*>(buffer.data());
      _data_queue.push(data);
      _mtx_data.unlock();
    }
  }

private:
  boost::asio::io_service _io_service;
  boost::asio::ip::tcp::socket _socket;

  std::thread _thrd;

  std::function<void(std::string&)> _read_cb;

  std::mutex _mtx_data;

  std::queue<std::string> _data_queue;

  const std::string _ip;
  const uint32_t    _port;

  std::atomic<bool> _run = true;
  std::atomic<bool> _auto_repair;
};





} //namespace client


} //namespace osat
