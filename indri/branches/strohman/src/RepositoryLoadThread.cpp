
//
// RepositoryLoadThread
//
// 31 January 2005 -- tds
//

#include "indri/RepositoryLoadThread.hpp"
#include "indri/Repository.hpp"

const int FIVE_SECONDS = 5*1000*1000;
const int HALF_SECOND = 500*1000;

//
// constructor
//

RepositoryLoadThread::RepositoryLoadThread( Repository& repository, UINT64 memory ) :
  UtilityThread(),
  _repository( repository ),
  _memory( memory )
{
}

//
// initialize
//

UINT64 RepositoryLoadThread::initialize() {
  return FIVE_SECONDS;
}

//
// deinitialize
//

void RepositoryLoadThread::deinitialize() {
  // do nothing
}

//
// work
//

UINT64 RepositoryLoadThread::work() {
  _repository._incrementLoad();

  Repository::index_state state = _repository.indexes();
  UINT64 memorySize = dynamic_cast<indri::index::MemoryIndex*>(state->back())->memorySize();

  if( memorySize > _memory ) {
    _repository._setThrashing( true );
    return HALF_SECOND;
  } else {
    _repository._setThrashing( false );
  }
  
  return FIVE_SECONDS;
}

//
// hasWork
//

bool RepositoryLoadThread::hasWork() {
  return false;
}



