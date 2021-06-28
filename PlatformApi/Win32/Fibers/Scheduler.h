#ifndef _FIBERS_SCHEDULER_H
#define _FIBERS_SCHEDULER_H

#include <stdint.h>

typedef void (*job_callback)(struct Scheduler *sched, void *arg);
#define JOB_ENTRY(fn) static void fn(struct Scheduler *sched, void *arg)
using job_counter = uint32_t;

struct FiberJob
{
    void              *data;
    job_callback       callback;
    // Useful for batching jobs together. 
    // For example:
    //   Let's say I wanted to batch 50 jobs together and sleep the
    //   fiber until all 50 jobs are complete. job_count is set to job
    //   for all jobs, and each job will do an atomic decrement when completed
    //   The fiber will wake up (rescheduled) when job_count == 0 
    volatile uint32_t *job_count;
};
enum SchedulerPriority 
{
    SchedulerPriority_High,
    SchedulerPriority_Normal,
    SchedulerPriority_Low,

    SchedulerPriority_Count,
    SchedulerPriority_Unknown = SchedulerPriority_Count,
};

enum class SchedulerMemoryTag : uint8_t
{
    Game,
    Render,
    GPU,
    IO,

    Count,
    Unknown = Count,
};

void scheduler_init(struct Scheduler **scheduler);
void scheduler_free(struct Scheduler **scheduler);
void scheduler_run_jobs(struct Scheduler *scheduler, SchedulerPriority priority, FiberJob *jobs, int jobs_count, volatile uint32_t *counter);
// Allows a fiber to sleep until the counter reaches the expected value. When the fiber wakens,
// it will be placed into the high priority queue.
void scheduler_await(struct Scheduler *scheduler,  volatile uint32_t *counter, uint32_t expected_value);

#endif // _FIBERS_SCHEDULER_H