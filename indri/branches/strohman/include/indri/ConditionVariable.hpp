
//
// ConditionVariable
//
// 15 November 2004 -- tds
//

#ifndef INDRI_CONDITIONVARIABLE_HPP
#define INDRI_CONDITIONVARIABLE_HPP

#ifndef WIN32
#include <pthread.h>
#endif

#include "Mutex.hpp"

class ConditionVariable {
private:
#ifdef WIN32
  HANDLE _singleEvent;
  HANDLE _allEvent;
#else
  pthread_cond_t _condition;
#endif

public:
  ConditionVariable() {
    #ifdef WIN32
        _singleEvent = ::CreateEvent( NULL, FALSE, FALSE, NULL );
        _allEvent = ::CreateEvent( NULL, TRUE, FALSE, NULL );
    #else
        pthread_cond_init( &_condition, NULL );
    #endif
  }

  ~ConditionVariable() {
    #ifdef WIN32
      ::CloseHandle( _event );
    #else
      pthread_cond_destroy( &_condition );
    #endif
  }

  void wait( Mutex& mutex ) {
#ifdef WIN32
    mutex.unlock();
    ::WaitForSingleObject( &_event, INFINITE );
    mutex.lock();
#else
    pthread_cond_wait( &_condition, &mutex._mutex );
#endif
  }

  void notifyOne( ) {
#ifdef WIN32
    ::PulseEvent( _singleEvent );
#else
  pthread_cond_signal( &_condition );
#endif
  }

  void notifyAll( ) {
#ifdef WIN32
    ::PulseEvent( _allEvent );
#else
    pthread_cond_broadcast( &_condition );
#endif
  }
};

#endif // INDRI_CONDITIONVARIBLE_HPP

