#ifndef SSERVER_MUTEX_H
#define SSERVER_MUTEX_H
#include "CurrentThread.h"
#include <assert.h>
#include <pthread.h>

#ifdef CHECK_PTHREAD_RETURN_VALUE

#ifdef NDEBUG
__BEGIN_DECLS
extern void __assert_perror_fail(int errnum,
                                 const char *file,
                                 unsigned int line,
                                 const char *function)
    __THROW __attribute__((__noreturn__));
__END_DECLS
#endif

#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       if (__builtin_expect(errnum != 0, 0))    \
                         __assert_perror_fail (errnum, __FILE__, __LINE__, __func__); })

#else // CHECK_PTHREAD_RETURN_VALUE

#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       assert(errnum == 0); (void) errnum; })

#endif // CHECK_PTHREAD_RETURN_VALUE

namespace sserver
{

// Use as data member of a class, eg.
//
// class Foo
// {
//  public:
//   int size() const;
//
//  private:
//   mutable MutexLock mutex_;
//   std::vector<int> data_; // GUARDED BY mutex_
// };

class MutexLock
{
public:
    MutexLock(const MutexLock &) = delete; //禁止拷贝构造
    MutexLock &operator=(const MutexLock &) = delete;
    MutexLock()
        : holder_(0)
    {
        int ret = pthread_mutex_init(&mutex_, NULL);
        assert(ret == 0);
        (void)ret; //编译器遇见函数中未使用的变量，会报错，这个是防止报错的
    }

    ~MutexLock()
    {
        assert(holder_ == 0);                     //断言这个锁没有被任何线程使用才可以销毁
        int ret = pthread_mutex_destroy(&mutex_); //销毁
        assert(ret == 0);
        (void)ret; //编译器遇见函数中未使用的变量，会报错，这个是防止报错的
    }

    // must be called when locked, i.e. for assertion
    bool isLockedByThisThread() const
    {
        return holder_ == CurrentThread::tid(); //判断当前线程是否拥有锁
    }

    void assertLocked() const
    {
        assert(isLockedByThisThread()); //断定当前线程拥有锁
    }

    // internal usage

    void lock()
    {
        pthread_mutex_lock(&mutex_);
        assignHolder(); //把锁的所有权给这个线程
    }

    void unlock()
    {
        unassignHolder(); //把锁的所有权滞空
        pthread_mutex_unlock(&mutex_);
    }

    pthread_mutex_t *getPthreadMutex() /* non-const */
    {
        return &mutex_; //返回锁
    }

private:
    friend class Condition; //条件变量和锁一起出现

    class UnassignGuard //条件变量时使用，封装和lockguard差不多
    {                   //https://www.cnblogs.com/lxy-xf/p/11289078.html
        /* 在wait()中，pthread_cond_wait会在内部释放锁，而在返回时占有这把锁， */
        /* UnassignGuard的目的正是在pthread_cond_wait释放锁前将锁的持有者清0， */
        /* 在pthread_cond_wait返回后重新占有锁时将锁的持有者改为当前线程。 */
        /* 看到UnassignGuard的构造函数，内容就是将锁的持有者清0，而在wait()结束时， */
        /* 对UnassignGuard进行析构时，UnassignGuard的析构函数被调用，内容为将当前线程设置为锁的持有者。 */

    public:
        UnassignGuard(const UnassignGuard &) = delete; //禁止拷贝构造
        UnassignGuard &operator=(const UnassignGuard &) = delete;
        UnassignGuard(MutexLock &owner)
            : owner_(owner)
        {
            owner_.unassignHolder(); //对该锁的持有者清0
        }

        ~UnassignGuard()
        {
            owner_.assignHolder(); //设置该锁的持有者线程。
        }

    private:
        MutexLock &owner_;
    };

    void unassignHolder() //把锁的所有权滞空
    {
        holder_ = 0;
    }

    void assignHolder()
    {
        holder_ = CurrentThread::tid(); //把锁的所有权给这这个线程
    }

    pthread_mutex_t mutex_;
    pid_t holder_;
};

// Use as a stack variable, eg.
// int Foo::size() const
// {
//   MutexLockGuard lock(mutex_);
//   return data_.size();
// }

class MutexLockGuard
{ //简化锁的操作，本来用的时候，在开始要在使用是调用一次锁，在不用时调用一次解锁
    //，这样的话，函数里只要一个创建一个这个对象把将锁传进去，构造时锁，离开是析构，正好
public:
    MutexLockGuard(const MutexLockGuard &) = delete; //禁止拷贝构造
    MutexLockGuard &operator=(const MutexLockGuard &) = delete;

    //在这里两个类只是关联关系，只是MutexLockGuard,这个对象使用了lock和unlock
    //如果是整体与局部的关系，那就是聚合关系
    //不仅仅是整体与局部关系，还负责对象的生成和销毁，就是组合关系
    explicit MutexLockGuard(MutexLock &mutex)
        : mutex_(mutex)
    {
        mutex_.lock();
    }

    ~MutexLockGuard()
    {
        mutex_.unlock(); //避免忘记解锁
    }

private:
    MutexLock &mutex_;
};

} // namespace sserver

#define MutexLockGuard(x) error "Missing guard object name" //防止大家错误的用法，出现一个匿名了临时对象，因为匿名对象不能长时间拥有锁

#endif
