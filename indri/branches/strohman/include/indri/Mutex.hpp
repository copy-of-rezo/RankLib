
//
// Mutex
//
// 15 November 2004 -- tds
//

#ifndef INDRI_MUTEX_HPP
#define INDRI_MUTEX_HPP

#include <assert.h>

#ifndef WIN32
#include <pthread.h>
#else
#define _WIN32_WINNT 0x0400
#include <windows.h>
#endif

#include "indri/Lockable.hpp"

class Mutex : public Lockable {
  friend class ConditionVariable;

private:
#ifdef WIN32
  HANDLE _mutex;
#else // POSIX
  pthread_mutex_t _mutex;
#endif

public:
  Mutex() {
#ifdef WIN32
    _mutex = ::CreateMutex( NULL, FALSE, NULL );
#else
    pthread_mutex_init( &_mutex, NULL );
#endif
  }

  ~Mutex() {
#ifdef WIN32
    ::CloseHandle( _mutex );
#else
    pthread_mutex_destroy( &_mutex );
#endif
  }

  void lock() {
#ifdef WIN32
    ::WaitForSingleObject( _mutex, INFINITE );
#else
    int result = pthread_mutex_lock( &_mutex );
    assert( result == 0 );
#endif
  }

  bool tryLock() {
#ifdef WIN32
    HRESULT result = ::WaitForSingleObject( _mutex, 0 );
    return (result == WAIT_OBJECT_0) || (result == WAIT_ABANDONED);
#else
    return pthread_mutex_trylock( &_mutex ) == 0;
#endif
  }

  void unlock() {
#ifdef WIN32
    ::ReleaseMutex( _mutex );
#else
    int result = pthread_mutex_unlock( &_mutex );
    assert( result == 0 );
#endif
  }
};

#endif // INDRI_MUTEX_HPP

