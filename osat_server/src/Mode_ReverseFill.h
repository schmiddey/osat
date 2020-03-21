#ifndef MODE_REVERSEFILL_H_
#define MODE_REVERSEFILL_H_

namespace osat{

class Mode_ReverseFill : public Mode {
public:
  using Mode::Mode; //inherit constructors

  virtual void setAction(const uint32_t id, const action::OsatAction action)
  {
    if(!_running)
    {
      return;
    }

    if(action == action::HIT)
    {
      if(_fist_hit)
      {
        _fist_hit = false;
        _timer_total.start();
      }
      //reduce inteval d
      _states.at(id) = state::INACTIVE;
      _hit_counter++;
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

    //check rdy
    bool rdy = true;
    for(auto& e : _states)
    {
      if(e.second != state::INACTIVE)
      {
        rdy = false;
      }
    }
    if(rdy)
    {
      _finish_up = true;
      _total_d = _timer_total.elapsed();
    }
  }

  virtual void start()
  {
    this->reset();
    _running = true;

    //set all active;
    for(auto& e : _cfg.ids)
    {
      this->change_state(e, state::ACTIVE);
    }
    // _timer_total.start();
    // _timer_interval.start();
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

  uint32_t _timeout_counter = 0;
  uint32_t _hit_counter     = 0;

  bool _finish_up = false;
  bool _fist_hit  = true;
  Duration _total_d;

};

} //namespace osat

#endif  //MODE_REVERSEFILL_H_