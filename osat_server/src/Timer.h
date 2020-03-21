#ifndef TIMER_H_
#define TIMER_H_



#include <iostream>

#include "Time.h"

namespace osat
{

/**
 * Timer class
 *
 * @author Michael Schmidpeter
 */
class Timer
{
public:
  /**
   * Default Constructor
   */
  Timer()
  {
    this->start();
  }

  /**
   * Destuctor
   */
  virtual ~Timer()
  { }

  /**
   * Starts Timer
   */
  void start() noexcept
  {
    _start = Time::now();
  }

  /**
   * Computes Duration between last start and current elapsed-call
   * @return Duration from start until now
   */
  Duration elapsed() noexcept
  {
    return (Time::now() - _start);
  }

protected:
  Time _start;  ///< Timepoint based on osat::Time
};

/**
 * TimerAuto class logs info (given msg + Duration as [ms]) when Destructor is called
 *
 * @author Michael Schmidpeter
 */
class TimerAuto_ms: public Timer
{
public:

  /**
   * Constructor starts Timer and saves info-msg
   * @param[in] msg to log when elapsed, default value = "timer: "
   */
  TimerAuto_ms(std::string msg) noexcept :
      Timer()
  {
    _msg = msg;
    this->start();
  }

  TimerAuto_ms() : TimerAuto_ms(std::string("timer: "))
  {  }


  /**
   * Destructor, computes Duration when called and logs msg + Duration as milliseconds.
   */
  virtual ~TimerAuto_ms()
  {
    double t = this->elapsed().toMSec();
    std::cout << _msg << t << std::endl;
  }

private:
  std::string _msg; ///< Message to log when elapsed
};

/**
 * TimerAuto class logs info (given msg + Duration as [us]) when Destructor is called
 *
 * @author Michael Schmidpeter
 */
class TimerAuto_us: public Timer
{
public:
  /**
   * Constructor starts Timer and saves info-msg
   * @param[in] msg to log when elapsed, default value = "timer: "
   */
  TimerAuto_us(std::string msg) noexcept :
      Timer()
  {
    _msg = msg;
    this->start();
  }

  TimerAuto_us() : TimerAuto_us(std::string("timer: "))
  { }

  /**
   * Destructor, computes Duration when called and logs msg + Duration as microseconds.
   */
  virtual ~TimerAuto_us()
  {
    double t = this->elapsed().toUSec();
    std::cout << _msg << t << std::endl;
  }

private:
  std::string _msg; ///< Message to log when elapsed
};

}  // namespace osat


#endif  //TIMER_H_

