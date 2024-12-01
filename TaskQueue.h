#include <condition_variable>
#include <mutex>
#include <queue>

#include "Log.h"

namespace SandServer
{
template <typename T>
class TaskQueue_t
{
 public:
   // ----------------------------------------------------------------------------
   void push( T&& requestHandler )
   {
      {
         std::unique_lock<std::mutex> lock( mutex );
         queue.push( requestHandler );
      }

      // Above scope is needed to unlock the mutex and only then to notify the
      // waiting thread
      condition.notify_one();
   }

   // ----------------------------------------------------------------------------
   T pop()
   {
      std::unique_lock<std::mutex> lock( mutex );
      condition.wait( lock, [ this ] { return !isEmpty() || terminate; } );

      // Check the state after waking up
      if ( isEmpty() && terminate )
      {
         return nullptr;
      }

      // This is so if someone uses T& = queue.front()
      // And then queue.pop() we dont get in trouble with the reference...
      T handleRequest = std::move( queue.front() );
      queue.pop();

      return handleRequest;
   }

   // ----------------------------------------------------------------------------
   bool isEmpty() const
   {
      return queue.empty();
   }

   // ----------------------------------------------------------------------------
   inline void notifyAll() { condition.notify_all(); }

 public:
   bool terminate{ false };

 private:
   std::queue<T> queue;
   mutable std::mutex mutex;
   std::condition_variable condition;
};
};   // namespace SandServer
