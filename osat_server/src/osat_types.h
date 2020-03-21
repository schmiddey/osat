#ifndef OSAT_TYPES_H_
#define OSAT_TYPES_H_

namespace osat{

namespace state{
  enum OsatState{
    IDLE = 0,
    ACTIVE,
    ACTIVE_TO,
    INACTIVE,
    FLASH,
    ANY
  };
} //namespace state

} //namespace osat

#endif  //OSAT_TYPES_H_