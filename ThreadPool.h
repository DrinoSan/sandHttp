// System Headers
#include <thread>

// Project Headers
#include "TaskQueue.h"
#include "Log.h"

namespace SandServer
{
class ThreadPool_t
{

 public:
   ThreadPool_t( size_t numThreads );
   ~ThreadPool_t();

   template <typename F>
   void enqueue( F&& f )
   {
      SLOG_ERROR("ADding new task");
      taskQueue.push( std::forward<F>( f ) );
   }

 private:
   std::vector<std::thread>           workers;
   TaskQueue_t<std::function<void()>> taskQueue;
   bool                               stop;
};
};   // namespace SandServer
