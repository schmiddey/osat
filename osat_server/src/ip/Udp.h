/**
 * @file   Udp.h
 * @author Michael Schmidpeter
 * @date   2019-11-26
 * @brief  todo
 * 
 * PROJECT: osat
 * @see https://github.com/schmiddey/osat
 */


#ifndef UDP_H_
#define UDP_H_

#include <iostream>
#include <cstdint>
#include <memory>
#include <vector>
#include <optional>

//boost stuff
#include <boost/asio.hpp>
#include <boost/bind.hpp>


namespace osat{

struct UdpMeta{
  const std::string ip;
  const uint32_t    port;
  UdpMeta(const std::string& ip_, const uint32_t port_)
    : ip(ip_), port(port_)
  { }
};


class UdpBase{
public:
  UdpBase() = default;
  virtual ~UdpBase() = default;
  
  virtual operator bool() const
  {
    return _open_ok;
  }

  // virtual std::string get_last_error() const = 0;  //todo

protected:
  boost::asio::io_service                       _io_service;
  std::unique_ptr<boost::asio::ip::udp::socket> _socket;

  bool _open_ok = false;

};

class UdpSrvReceive : public UdpBase{
public:
  UdpSrvReceive() = delete;
  virtual ~UdpSrvReceive() = default;

  /**
   * @brief Construct a new UdpSrvReceive object
   * 
   * @param port 
   */
  UdpSrvReceive(const uint32_t port) noexcept :
    UdpBase(), 
    _deadline(_io_service)
  {
    _deadline.expires_at(boost::posix_time::pos_infin);
    try{
      _socket = std::make_unique<boost::asio::ip::udp::socket>(_io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port));
      _open_ok = true;
    } catch(std::exception& e)
    {
      std::cerr << "error at opening udp socket. what?: " << e.what() << std::endl;
    }

    this->check_deadline();
  }

  /**
   * @brief 
   * 
   * @throw boost::system::system_error on receive error
   * 
   * @param size 
   * @return std::vector<uint8_t> 
   */
  std::pair<std::vector<uint8_t>,UdpMeta> receive(const std::size_t size)
  {
    std::vector<uint8_t> buffer(size);
    boost::asio::ip::udp::endpoint remote_endpoint;
    boost::system::error_code error;
    
    auto n_rd_bytes = _socket->receive_from(boost::asio::buffer(buffer), remote_endpoint, 0, error);
    
    if(error && error != boost::asio::error::message_size) 
    {
      throw boost::system::system_error(error);
    }

    buffer.resize(n_rd_bytes);  //fix size, if less data is read then given size

    return std::make_pair(buffer, UdpMeta(remote_endpoint.address().to_string(), remote_endpoint.port()));
  }

  // std::optional<std::vector<uint8_t>> receive_all(const std::size_t size); //todo 

  /**
   * @brief 
   * 
   * @throw todo
   * 
   * @param size 
   * @param timeout_us 
   * @return std::optional<std::pair<std::vector<uint8_t>,UdpMeta>> 
   */
  std::optional<std::pair<std::vector<uint8_t>,UdpMeta>> receive(const std::size_t size, const std::size_t timeout_us)
  {
    std::vector<uint8_t> buffer(size);

    _deadline.expires_from_now(boost::posix_time::microseconds(timeout_us));

    boost::system::error_code ec;
    boost::asio::ip::udp::endpoint remote_endpoint;

    ec = boost::asio::error::would_block;
    std::size_t length = 0;

    _socket->async_receive_from(boost::asio::buffer(buffer), remote_endpoint, boost::bind(&UdpSrvReceive::handle_receive, _1, _2, &ec, &length));

    do _io_service.run_one(); while(ec == boost::asio::error::would_block);

    if(ec.value())
    {
      return std::nullopt;
    }

    buffer.resize(length);

    return std::make_pair(buffer, UdpMeta(remote_endpoint.address().to_string(), remote_endpoint.port()));
  }
  
private: //functions
  void check_deadline()
  {
    if (_deadline.expires_at() <= boost::asio::deadline_timer::traits_type::now())
    {
      _socket->cancel();

      _deadline.expires_at(boost::posix_time::pos_infin);
    }
    // Put the actor back to sleep.
    _deadline.async_wait(boost::bind(&UdpSrvReceive::check_deadline, this));
  }

  static void handle_receive(const boost::system::error_code& ec, std::size_t length,
                             boost::system::error_code* out_ec, std::size_t* out_length)
  {
    *out_ec = ec;
    *out_length = length;
  }

private: //data
  boost::asio::deadline_timer _deadline;
};








class UdpClSend : public UdpBase{
public:
  UdpClSend() = delete;
  virtual ~UdpClSend() = default;

  UdpClSend(const std::string& ip, const uint32_t port) noexcept : 
    UdpBase(),
    _endpoint(boost::asio::ip::address::from_string(ip), port)
  {
    _socket = std::make_unique<boost::asio::ip::udp::socket>(_io_service);
    try{
      _socket->open(boost::asio::ip::udp::v4());
      _open_ok = true;
    } catch(std::exception& e)
    {
      std::cerr << "error at opening udp socket. what?: " << e.what() << std::endl;
    }
  }

  /**
   * @brief 
   * 
   * @throw boost::system::system_error Thrown on failure.
   * 
   * @param data 
   * @return std::size_t 
   */
  std::size_t transmit(const std::string& data)
  {
    return _socket->send_to(boost::asio::buffer(data), _endpoint);
  }

  /**
   * @brief 
   * 
   * @throw boost::system::system_error Thrown on failure.
   * 
   * @param data 
   * @return std::size_t 
   */
  std::size_t transmit(const std::vector<uint8_t>& data)
  {
    return _socket->send_to(boost::asio::buffer(data), _endpoint);
  }
  // std::optional<std::size_t> transmit(void* data, const std::size_t size);   //--> use std::any

private:
  boost::asio::ip::udp::endpoint _endpoint;
};



} //namespace osat

#endif  //UDP_H_