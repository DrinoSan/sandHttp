#include "ThreadPool.h"
#include "Log.h"

namespace SandServer
{

//-----------------------------------------------------------------------------
ThreadPool_t::ThreadPool_t( size_t numThreads ) : stop{ false }
{
   SLOG_INFO( "Constructiong ThreadPool with {0} threads", numThreads );
   for ( size_t i = 0; i < numThreads; ++i )
   {
      workers.emplace_back(
          [ this, i ]
          {
             while ( true )
             {
                std::function<void()> task;

                // Scope for locking reasons to make sure that somehow pop is
                // not still locked when we try to execute task()
                {
                   task = taskQueue.pop();
                }

                // After we notify the queue in the threadPool destructor we
                // will get a nullptr back from .pop therefore this check
                // then we return and can join peacefully
                if ( !task )
                {
                   return;
                }

                task();
             }
          } );
   }
}

//-----------------------------------------------------------------------------
ThreadPool_t::~ThreadPool_t()
{
   stop                = true;
   taskQueue.terminate = true;
   taskQueue.notifyAll();   // Notify all waiting threads

   for ( std::thread& worker : workers )
   {
      worker.join();
   }
}

//-----------------------------------------------------------------------------
void ThreadPool_t::init( size_t numThreads )
{
   SLOG_INFO( "Constructiong ThreadPool with {0} threads", numThreads );
   for ( size_t i = 0; i < numThreads; ++i )
   {
      workers.emplace_back(
          [ this, i ]
          {
             while ( true )
             {
                std::function<void()> task;

                // Scope for locking reasons to make sure that somehow pop is
                // not still locked when we try to execute task()
                {
                   task = taskQueue.pop();
                }

                // After we notify the queue in the threadPool destructor we
                // will get a nullptr back from .pop therefore this check
                // then we return and can join peacefully
                if ( !task )
                {
                   return;
                }

                task();
             }
          } );
   }
}

};   // namespace SandServer
