#include "TaskQueue.h"

namespace SandServer
{
// ----------------------------------------------------------------------------
template<class T>
void TaskQueue_t<T>::push( T& clientFD )
{
   {
      std::unique_lock<std::mutex> lock( mutex );
      queue.push( clientFD );
   }

   // Above scope is needed to unlock the mutex and only then to notify the waiting thread
   condition.notify_one();
}

// ----------------------------------------------------------------------------
template<class T>
T TaskQueue_t<T>::pop()
{
   std::unique_lock<std::mutex> lock( mutex );
   condition.wait( lock, [this]{ return !isEmpty(); });

   // This is so if someone uses T& = queue.front()
   // And then queue.pop() we dont get in trouble with the reference...
   T clientFD = std::move( queue.front() );
   queue.pop();

   return clientFD;
}

// ----------------------------------------------------------------------------
template<class T>
bool TaskQueue_t<T>::isEmpty() const
{
   std::unique_lock<std::mutex> lock( mutex );
   return queue.empty();
}
};   // namespace SandServer
