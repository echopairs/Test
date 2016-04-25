/*********************************************************************
  Copyright (C), 1990-2016, HangZhou RED HEAT Tech. Co., Ltd.
  FileName: queue.h
  Author  : pairs & 736418319@qq.com
  Version : 1.0
  Date    : 2016/04/24
  Description:
  Function List:
  History:
      <author>    <time>    <version>    <desc>
        pairs     16/04/24      1.0     build this moudle
*********************************************************************/

// four status to queue
#include <vector>
#include <set>
#include <thread>
#include <stddef.h>
#include <mutex>
#include <map>
using TickTaskQueueEventType = enum TickTaskQueueEventType_ 
{
    TickTaskQueueEventEnterLowLevel = 0,
    TickTaskQueueEventEnterHighLevel = 1,
    TickTaskQueueEventFirstBlood = 2,
    TickTaskQueueEventLastBlood = 3,
};

// callback function
using TickTaskQuentEventCallback = void(*)(const TickTaskQueueEventType event, void *);

template<typename T>
class TickTaskQueue
{
    public:
      TickTaskQueue(size_t lowLevel = 0, size_t highLevel = (size_t)(-1)):
                    lowLevel_(lowLevel),
                    highLevel_(highLevel),
                    data_(compare)
                    {}
      virtual ~TickTaskQueue()
      {
          data_.clear(); 
      }

      void setHighLevel(T highLevel)
      {
          highLevel_ = highLevel;
      }

      size_t getHighLevel() const
      {
          return highLevel_;
      }

      void setLowLevel(T lowelLevel)
      {
          lowLevel_ = lowelLevel;
      }

      // get lowlevel 
      size_t getLowLevel() const
      {
          return lowLevel_;
      }

      // pop count element T out 
      void pop(size_t count, std::vector<T>*out)
      {
          size_t  sizeBefore = size();
          size_t  retSize = (sizeBefore > count ? count:sizeBefore);

          while(retSize--)
          {
              auto it = data_.begin();
              if(out)
              {
                  out->push_back(*it);
              }
              data_.erase(it);
          }

          // after pop
          if ((sizeBefore > lowLevel_) && (size() < lowLevel_))
          {
              eventCallBack(TickTaskQueueEventEnterLowLevel);
          }

          if (0 == size())
          {
              eventCallBack(TickTaskQueueEventLastBlood);
          }
      }

      // pop element out
      void pop(T* out)
      {
          size_t sizeBefore = size();
          if (!data_.empty())
          {
              if (out)
              {
                 *out = *(data_.begin());
              }
          }
          data_.erase(data_.begin());

          // after pop
          if (size() == lowLevel_)
          {
              eventCallBack(TickTaskQueueEventEnterLowLevel);
          }

          if (sizeBefore > 0 && size() == 0)
          {
              eventCallBack(TickTaskQueueEventLastBlood);
          }
      }
      // push element to data_
      void push(const T& t)
      {
          size_t sizeBefore = size();
          data_.insert(t);

          if (size() >= highLevel_)
          {
              eventCallBack(TickTaskQueueEventEnterHighLevel);
          }

          if (0 == sizeBefore)
          {
              eventCallBack(TickTaskQueueEventFirstBlood);
          }
      }

      void push(const std::vector<T>&t)
      {
          size_t sizeBefore = size();

          auto it = t.begin();
          while(it != t.end())
          {
              data_insert(*it);
              ++it;
          }

          // after push
          if (sizeBefore == 0 && size() > 0)
          {
              eventCallBack(TickTaskQueueEventFirstBlood);
          }

          if (size() >=highLevel_)
          {
              eventCallBack(TickTaskQueueEventEnterHighLevel);
          }
      }

      static bool compare(const T &a, const T &b)
      {
          return (a < b);
      }

      bool empty() const
      {
         return data_.empty();  
      }
      size_t size()
      {
          return data_.size();
      }
      void clear()
      {
          data_.clear();
      }
      void lock()
      {
          mutex_.lock();
      }
      void unlock()
      {
          mutex_.unlock();
      }
      std::mutex& mutex()
      {
          return mutex_;
      }

      void registEventCallback(const TickTaskQueueEventType evt, 
                               const TickTaskQuentEventCallback cb,
                               void *userData)
      {
          cb_[evt][cb] = userData;
      }
      void unregistEventCallback(const TickTaskQueueEventType evt, 
                                 const TickTaskQuentEventCallback cb)
      {
          if (cb)
          {
              cb_[evt].erase(cb);
          }
      }

    private:
      void eventCallBack(TickTaskQueueEventType evt)
      {
          std::map<TickTaskQuentEventCallback, void*>&temp = cb_[evt];
          auto it = temp.begin();
          while (it != temp.end())
          {
              it->first(evt, it->second);
              ++it;
          }
      }

    private:
      size_t lowLevel_;
      size_t highLevel_;
      std::mutex mutex_;
      std::set<T,bool(*)(const T&, const T&)>data_;
      std::map<TickTaskQueueEventType, std::map<TickTaskQuentEventCallback, void*> > cb_;
};


/*
    1. set<int>s0;
    2. set<int , greater<int>>s1; // default less<int>
    3. struct classcmp
       {
          bool operator()(const int &lhs, const int& rhs)
          {
              return lhs < rhs;
          }
       };
       set<int, classcmp>s2;
    4. bool fncmp(int lhs, int rhs)
       {
            return lhs < rhs;  
       }
       bool(*fn_pt)(int, int) = fncmp;
       set<int, bool(*)(int, int)> s3(fn_pt);
*/
