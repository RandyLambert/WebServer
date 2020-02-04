#include "Condition.h"
#include <errno.h>

bool sserver::Condition::waitForSeconds(int seconds)
{
    struct timespec abstime;
    /* clock_gettime(CLOCK_REALTIME, &abstime); */
    clock_gettime(CLOCK_MONOTONIC, &abstime);
    //可以根据需要，获取不同要求的精确时间
    //clk_id : 检索和设置的clk_id指定的时钟时间。
    /*CLOCK_REALTIME:系统实时时间,随系统实时时间改变而改变,即从UTC1970-1-1 0:0:0开始计时, */
    /*中间时刻如果系统时间被用户改成其他,则对应的时间相应改变 */
    /*CLOCK_MONOTONIC:从系统启动这一刻起开始计时,不受系统时间被用户改变的影响 */
    /*CLOCK_PROCESS_CPUTIME_ID:本进程到当前代码系统CPU花费的时间 */
    /*CLOCK_THREAD_CPUTIME_ID:本线程到当前代码系统CPU花费的时间 */

    abstime.tv_sec += seconds;
    MutexLock::UnassignGuard ug(mutex_); //解锁在加锁的类
    return ETIMEDOUT == pthread_cond_timedwait(&pcond_, mutex_.getPthreadMutex(), &abstime);
    //线程阻塞在这里,如果返回超时的，是true其他是false
}
