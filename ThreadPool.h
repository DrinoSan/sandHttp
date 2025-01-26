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
   ThreadPool_t() = default;
   ThreadPool_t( size_t numThreads );
   ~ThreadPool_t();

   template <typename F>
   void enqueue( F&& f )
   {
      taskQueue.push( std::forward<F>( f ) );
   }

   bool isQueueEmpty()
   {
      return taskQueue.isEmpty();
   }

   void init( size_t numThreads );

 public:
   bool stop;

 private:
   std::vector<std::thread>           workers;
   TaskQueue_t<std::function<void()>> taskQueue;
};
};   // namespace SandServer
