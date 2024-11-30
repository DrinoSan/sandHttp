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
                   SLOG_WARN( "---THREADPOOL: Thread ID: {0} waiting for tasks",
                              i );
                   if ( stop && taskQueue.isEmpty() )
                      return;
                   task = taskQueue.pop();
                   SLOG_WARN( "---THREADPOOL: Thread ID: {0} got task task",
                              i );
                }

                task();
                SLOG_WARN( "---THREADPOOL: Thread ID: {0} finished task", i );
             }
          } );
   }
}

//-----------------------------------------------------------------------------
ThreadPool_t::~ThreadPool_t()
{
   stop = true;
   for ( std::thread& worker : workers )
      worker.join();
}

};   // namespace SandServer
