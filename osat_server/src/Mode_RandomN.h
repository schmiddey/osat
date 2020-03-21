#ifndef MODE_RANDOMN_H_
#define MODE_RANDOMN_H_

#include <chrono>

#include "Mode.h"

#include "Timer.h"

namespace osat{



class Mode_RandomN : public Mode {
public:
  using Mode::Mode; //inherit Constructors

  virtual void setAction(const uint32_t id, const action::OsatAction action)
  {
    if(!_running)
    {
      return;
    }

    if(action == action::HIT)
    {
      //reduce inteval d
      _interval_d -= Duration(static_cast<double>(_cfg.interval_reduce_step_ms)/1000.0);
      _states.at(id) = state::INACTIVE;
      _hit_counter++;
    }
    else if(action == action::TIMEOUT)
    {
      _states.at(id) = state::INACTIVE;
      _timeout_counter++;
    }

    //estimate rdy
    if(_timeout_counter >= _cfg.max_timeout_count)
    { 
      //ready
      _total_d = _timer_total.elapsed();
      _finish_up = true;
      // _running = false;

    }

  }

  virtual void tick()
  {
    if(!_running)
    {
      return;
    }

    if(_finish_up)
    {
      _finish_up = false;
          //ready
      Report rep(_hit_counter, _timeout_counter, _total_d);
      //call callback
      _mode_rdy_cb(_cfg.mode_name, rep);

      this->reset();
      return;
    }
    if((_timer_interval.elapsed() >= _interval_d))
    {
      //active next;
      auto rnd_ids = this->getRandomIds(_cfg.activate_count, state::INACTIVE);

      for(auto& e : rnd_ids)
      {
        this->change_state(e, state::ACTIVE_TO);
      }

      _timer_interval.start();
    }
  }

  virtual void start()
  {
    //ensure all osats have given timeout...
    for(auto& e : _cfg.ids)
    {
      _change_timeout_cb(e, _cfg.timeout_ms);
    }
    this->reset(); //set all inactive

    _running = true;

    // auto rnd_ids = this->getRandomIds(_cfg.activate_count);

    // for(auto& e : rnd_ids)
    // {
    //   this->change_state(e, state::ACTIVE_TO);
    // }
    _timer_total.start();
    _timer_interval.start();
    _interval_d = Duration(static_cast<double>(_cfg.interval_ms) / 1000.0);
  }

  virtual void reset()
  {
    std::cout << "++++++++++++++ reset all osats" << std::endl;
    _running = false;
    _timeout_counter = 0;
    _hit_counter = 0;
    //set all invactive
    for(auto& e : _cfg.ids)
    {
      this->change_state(e, state::INACTIVE);
    }
  }

private:
  Timer _timer_total;
  Timer _timer_interval;

  Duration _interval_d;

  uint32_t _timeout_counter = 0;
  uint32_t _hit_counter     = 0;

  bool _finish_up = false;
  Duration _total_d;
};


} //namespace osat

#endif  //MODE_RANDOMN_H_