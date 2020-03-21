#ifndef TIME_H_
#define TIME_H_





#include <iostream>
#include <chrono>
#include <string>
#include <sstream>
#include <cmath>
#include <ctime>

#include <thread> //for cross platform sleep

namespace osat
{

using Duration_t = std::chrono::duration<double>;
using TimePoint_t = std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::duration<double>>;

/**
 * Duration class based on <chrono> c++ library
 *
 * @author Michael Schmidpeter
 */
class Duration
{
public:

  /**
   * Default Constructor
   */
  Duration() noexcept
  { }

  /**
   * Copy Constructor, as default
   * @param[in] d object to copy
   */
  Duration(const Duration& d) = default;

  /**
   * Move Constructor, as default
   * @param[in, out] d object to move
   */
  Duration(Duration&& d) = default;

  /**
   * Constructor for duration using double as seconds
   * @param[in] dur_ms as [s]
   */
  Duration(const double dur_s) noexcept
  {
    _duration = Duration_t(dur_s);  //default constructor as s
  }

  /**
   * Constructor for chrono::duration<double>
   * @param[in] t std::chrono::duration
   */
  Duration(Duration_t t) noexcept
  {
    _duration = t;
  }

  /**
   * sleeps with given duration
   */
  void sleep()
  {
    std::this_thread::sleep_for(_duration);   //cross platform c++11
  }

  /**
   * Converts duration to seconds (not rounded)
   *
   * @return Duration as seconds
   */
  double toSec() const noexcept
  {
    return std::chrono::duration_cast<std::chrono::duration<double>>(_duration).count();
  }

  /**
   * Converts duration to milliseconds (not rounded)
   *
   * @note cast to std::chrono::milliseconds is uint64_t not double, so manual duration-double cast is used
   *
   * @return Duration as milliseconds
   */
  double toMSec() const noexcept
  {
    return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(_duration).count();
  }

  /**
   * Converts duration to microseconds (not rounded)
   *
   * @note cast to std::chrono::microseconds is uint64_t not double, so manual duration-double cast is used
   *
   * @return Duration as microseconds
   */
  double toUSec() const noexcept
  {
    return std::chrono::duration_cast<std::chrono::duration<double, std::micro>>(_duration).count();
  }

  /**
   * Converts duration to nanoseconds (not rounded)
   *
   * @note cast to std::chrono::nanoseconds is uint64_t not double, so manual duration-double cast is used
   *
   * @return Duration as nanoseconds
   */
  double toNSec() const noexcept
  {
    return std::chrono::duration_cast<std::chrono::duration<double, std::nano>>(_duration).count();
  }

  /**
   * Converts to Chrono duration
   *
   * @return Duration as std::chrono::duration<double>
   */
  Duration_t toChronoDuration() const noexcept
  {
    return _duration;
  }

  Duration& operator=(const Duration& d) = default;

  Duration& operator=(Duration&& d) = default;

  Duration& operator+=(const Duration& d) noexcept
  {
    _duration += d._duration;
    return *this;
  }

  Duration operator+(const Duration& d) const noexcept
  {
    Duration tmp(d);
    tmp += d;
    return tmp;
  }

  Duration& operator-=(const Duration& d) noexcept
  {
    _duration -= d._duration;
    return *this;
  }

  Duration operator-(const Duration& d) const noexcept
  {
    Duration tmp(d);
    tmp += d;
    return tmp;
  }

  bool operator!=(const Duration& d) const noexcept
  {
    return _duration != d._duration;
  }

  bool operator<(const Duration& d) const noexcept
  {
    return _duration < d._duration;
  }

  bool operator<=(const Duration& d) const noexcept
  {
    return _duration <= d._duration;
  }

  bool operator==(const Duration& d) const noexcept
  {
    return _duration == d._duration;
  }

  bool operator>(const Duration& d) const noexcept
  {
    return _duration > d._duration;
  }

  bool operator>=(const Duration& d) const noexcept
  {
    return _duration >= d._duration;
  }

private:
  Duration_t _duration;   ///< std::chrono::duration as base data type
};

/**
 * Time Class for Time points based on <chrono> c++ library
 *
 * @author Michael Schmidpeter
 */
class Time
{
public: //static

  /**
   * Get current system time
   *
   * @return current time as evo::Time
   */
  static Time now() noexcept
  {
    return Time(std::chrono::high_resolution_clock::now());
  }

  /**
   * Converts std::chrono::time_point to std::string
   *
   * @param[in] t time point to convert
   * @return date+time as std::string
   */
  static std::string toString(TimePoint_t t) noexcept
  {
    std::time_t tt = std::chrono::high_resolution_clock::to_time_t(
        std::chrono::high_resolution_clock::time_point(
            std::chrono::duration_cast<std::chrono::milliseconds>(t.time_since_epoch()) ) );

    std::stringstream sstr;
    char cstr[64];
    std::strftime(cstr, sizeof(cstr), "%Y%m%d_%H-%M-%S", std::localtime(&tt));
    return cstr;
  }

  /**
   * Converts evo::Time to std::string
   *
   * @param[in] t time point to convert
   * @return date+time as std::string
   */
  static std::string toString(Time t) noexcept
  {
    return Time::toString(t._timepoint);
  }

public: //member functions
  /**
   * Default Constructor creates time point at beginning of time (unix time)
   */
  Time() noexcept
  { }

  /**
   * Copy Constructor, as default
   * @param[in] t  object to copy
   */
  Time(const Time& t) = default;

  /**
   * Move Constructor, as default
   * @param[in,out] t object to move
   */
  Time(Time&& t) = default;

  /**
   * Constructor for chrono::high_resolution_clock::time_point
   *
   * @param tp time point to set
   */
  Time(TimePoint_t tp) noexcept
  {
    _timepoint = tp;
  }

  /**
   * Constructor for Time as seconds
   * @param[in] s absolute Time in seconds
   */
  Time(const double s) noexcept
  {
    Duration d(s);
    (*this) += d;
  }

  /**
   * Converts this object to std::string
   *
   * @return time point as string
   */
  std::string toString() const noexcept
  {
    return Time::toString(_timepoint);
  }

  /**
   * Converts time point to seconds (not rounded)
   *
   * @return time point as seconds
   */
  double toSec() const noexcept
  {
    TimePoint_t t;   //time zero
    Duration tmp(_timepoint - t);
    return tmp.toSec();
  }

  Time& operator=(const Time& t) = default;

  Time& operator=(Time&& t) = default;

  /*
   * Timepoint - Timepoint = Duration
   */
  Duration operator-(const Time& t) const noexcept
  {
    return Duration(std::chrono::duration_cast<std::chrono::duration<double>>(_timepoint - t._timepoint));
  }

  /*
   * Timepoint +/- Duration = Timepoint
   */

  Time operator-(const Duration& d) const noexcept
  {
    Time tmp(_timepoint);
    return tmp -= d;
  }

  Time& operator-=(const Duration& d) noexcept
  {
    _timepoint = _timepoint - d.toChronoDuration();
    return *this;
  }

  Time operator+(const Duration& d) const noexcept
  {
    Time tmp(_timepoint);
    return tmp += d;
  }

  Time& operator+=(const Duration& d) noexcept
  {
    _timepoint = _timepoint + d.toChronoDuration();
    return *this;
  }

  bool operator!=(const Time& t) const noexcept
  {
    return _timepoint != t._timepoint;
  }

  bool operator<(const Time& t) const noexcept
  {
    return _timepoint < t._timepoint;
  }

  bool operator<=(const Time& t) const noexcept
  {
    return _timepoint <= t._timepoint;
  }

  bool operator==(const Time& t) const noexcept
  {
    return _timepoint == t._timepoint;
  }

  bool operator>(const Time& t) const noexcept
  {
    return _timepoint > t._timepoint;
  }

  bool operator>=(const Time& t) const noexcept
  {
    return _timepoint >= t._timepoint;
  }

private:
  TimePoint_t _timepoint; ///< std::chrono::time_point as base data type
};

}  // namespace osat

// Overloaded Ostreams

/**
 * Overloaded ostream for output for Time
 *
 * @param[in] os ostream for output
 * @param[in] t time point as osat::Time
 * @return given ostream
 */
inline std::ostream& operator<<(std::ostream& os, const osat::Time& t)
{
  os << "(time: " << static_cast<unsigned long>(std::round(t.toSec())) << " s)";
  return os;
}

/**
 * Overloaded ostream for output of Duration
 * @param[in] os ostream for output
 * @param[in] d duration as osat::Duration
 * @return given ostream
 */
inline std::ostream& operator<<(std::ostream& os, const osat::Duration& d)
{
  os << "(duration: " << static_cast<unsigned long>(std::round(d.toSec())) << " s)";
  return os;
}

#endif  //TIME_H_
