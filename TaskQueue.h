#include <condition_variable>
#include <mutex>
#include <queue>

namespace SandServer
{
template <typename T>
class TaskQueue_t
{
 public:
   // ----------------------------------------------------------------------------
   void push( T& clientFD );

   // ----------------------------------------------------------------------------
   T    pop();

   // ----------------------------------------------------------------------------
   bool isEmpty() const;

 private:
   std::queue<T> queue;
   mutable std::mutex mutex;
   std::condition_variable condition;
};
};   // namespace SandServer
