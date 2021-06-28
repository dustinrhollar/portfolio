#include "Scheduler.h"
#include "FiberContext.h"
#include "../Win32Common.h"

#include <string.h>
#include <emmintrin.h>

//----------------------------------------------------------------------------------------------------------//
// Various Macros
//----------------------------------------------------------------------------------------------------------//

#define FORCE_INLINE    __forceinline

using uptr = uintptr_t;
using spinlock = volatile uint32_t;

constexpr u32 MAX_WORK_QUEUE_JOBS = 1024;
constexpr u32 WORK_QUEUE_MASK =  (MAX_WORK_QUEUE_JOBS-1);
constexpr u32 MAX_WORKER_THREADS = 64;

constexpr u32 MAX_FIBERS = 1048;

static struct Scheduler *scheduler = 0;

//----------------------------------------------------------------------------------------------------------//
// Synchronization Helpers
//----------------------------------------------------------------------------------------------------------//

#define compare_and_swap_32(ptr, comparand, replacement) \
    _mcompare_and_swap_32((LONG volatile *)(ptr), (LONG)(comparand), (LONG)(replacement))
#define compare_and_swap_64(ptr, comparand, replacement) \
    _mcompare_and_swap_64((LONG64 volatile*)(ptr), (LONG64)(comparand), (LONG64)(replacement))
#define compare_and_swap_ptr(ptr, comparand, replacement) \
    _mcompare_and_swap_ptr((volatile PVOID*)(ptr), (PVOID)(comparand), (PVOID)(replacement))

FORCE_INLINE b8 
_mcompare_and_swap_32(volatile LONG *ptr, LONG comparand, LONG replacement)
{
    Assert(ptr);
    return _InterlockedCompareExchange(ptr, replacement, comparand) == comparand;
}

FORCE_INLINE b8 
_mcompare_and_swap_64(volatile LONG64 *ptr, LONG64 comparand, LONG64 replacement)
{
    return _InterlockedCompareExchange64(ptr, replacement, comparand) == comparand;
}

FORCE_INLINE b8 
_mcompare_and_swap_ptr(volatile PVOID *ptr, PVOID comparand, PVOID replacement)
{
    return _InterlockedCompareExchangePointer(ptr, replacement, comparand) == comparand;
}

//----------------------------------------------------------------------------------------------------------//
// Spinlock
//----------------------------------------------------------------------------------------------------------//

// NOTE(Dustin): Latency: 140 cycles. Is there a better approach?
// Maybe spin for a short period before pausing?
static void 
spinlock_enter(spinlock *lock)
{
    Assert(lock);
    while (compare_and_swap_32(lock, 0, 1) != 0)
        _mm_pause();
}

static void 
spinlock_leave(spinlock *lock)
{
    _mm_mfence(); 
    *lock = 0;
}

//----------------------------------------------------------------------------------------------------------//
// Semaphore
//----------------------------------------------------------------------------------------------------------//

struct Sem
{
    CRITICAL_SECTION   lock;
    CONDITION_VARIABLE notify;
};

static void
sem_init(Sem *sem)
{
    InitializeConditionVariable(&sem->notify);
    InitializeCriticalSectionAndSpinCount(&sem->lock, 0x00001000);
}

static void
sem_free(Sem *sem)
{
    DeleteCriticalSection(&sem->lock);
}

static void
sem_post_all(Sem *sem)
{
    WakeAllConditionVariable(&sem->notify);
}

static void
sem_post(Sem *sem)
{
    WakeConditionVariable(&sem->notify);
}

static void
sem_wait(Sem *sem)
{
    EnterCriticalSection(&sem->lock);
    SleepConditionVariableCS(&sem->notify, &sem->lock, INFINITE);
    LeaveCriticalSection(&sem->lock);
}

//----------------------------------------------------------------------------------------------------------//
// Tagged Heap
//----------------------------------------------------------------------------------------------------------//

template<u64 size>
struct BitmapTemplate
{
    u64 bitset[size];
};

constexpr u64 TAGGED_HEAP_MEMORY_SIZE = _1GB;
constexpr u64 TAGGED_HEAP_BLOCK_SIZE  = _2MB;
using Bitmap = BitmapTemplate<(TAGGED_HEAP_MEMORY_SIZE / TAGGED_HEAP_BLOCK_SIZE) / sizeof(u64)>;

static void
bitmap_init(Bitmap *bitmap)
{
    u32 len = ARRAYCOUNT(bitmap->bitset);
    Assert(len == 16);
}


struct TaggedHeap
{
    Bitmap bitmap;
};

static void
tagged_heap_init(TaggedHeap *heap)
{
    bitmap_init(&heap->bitmap);
}

//----------------------------------------------------------------------------------------------------------//
// Stack Allocations
//----------------------------------------------------------------------------------------------------------//

constexpr u64 FIBER_STACK_SIZE = _KB(10); // 4KB  for a stack size, maybe too large?
constexpr u8 RED_ZONE_SIZE = 2; // Red zone on a stack is 2 bytes
constexpr char RED_ZONE_FLAG = '\0';
using FiberStack = void*;

struct FiberStackAllocator
{
    void       *start;
    FiberStack *pool;
    spinlock    lock;
    u32         stride;
};

static void 
fiber_stack_allocator_init(FiberStackAllocator *a)
{
    u64 num_elements = MAX_FIBERS;
    u64 req_memory = num_elements * FIBER_STACK_SIZE + (2 * (num_elements * RED_ZONE_SIZE));
    a->start = PlatformAlloc(req_memory);
    Assert(a->start != nullptr);
    AssertCustom(((uptr)a->start % 16L) == 0, "Allocated stack should be aligned to 16L");

    a->stride = FIBER_STACK_SIZE + 2 * RED_ZONE_SIZE;
    a->pool = (FiberStack*)a->start;
    FiberStack *iter = a->pool;
    for (int i = 0; i < num_elements - 1; ++i)
    {
        // Insert into poo lise
        *iter = (char*)iter + a->stride;
        iter = (FiberStack*)(*iter);
    }
    *iter = NULL;
}

static void 
fiber_stack_allocator_free(FiberStackAllocator *a)
{
    a->pool = 0;
    PlatformFree(a->start);
    a->start = 0;
}

// Allocate the stack space for a fiber.
// pfn: Optional function pointer. At the end of a fiber fn call,
//      the stack will jump to this function.
static FiberStack
fiber_stack_allocator_alloc(FiberStackAllocator *a)
{
    FiberStack result = 0;

    spinlock_enter(&a->lock);
    if (a->pool)
    {
        result = (FiberStack)a->pool;
        a->pool = (FiberStack*)(*a->pool);
    }
    spinlock_leave(&a->lock);
    //Assert(result);

    if (result)
    {
        // Set red zones
        memset(result, '\0', RED_ZONE_SIZE);
        memset((char*)result + RED_ZONE_SIZE + FIBER_STACK_SIZE, '\0', RED_ZONE_SIZE);
    }

    return (char*)result + RED_ZONE_SIZE;
}

static void
fiber_stack_allocator_release(FiberStackAllocator *a, FiberStack ptr)
{
    spinlock_enter(&a->lock);
    char *iter = (char*)ptr - RED_ZONE_SIZE;
    *((FiberStack*)iter) = a->pool;
    a->pool = (FiberStack*)iter;
    spinlock_leave(&a->lock);
}

FORCE_INLINE bool
fiber_stack_check_redzone(FiberStack stack)
{
    bool result = true;

    // Check bottom of stack
    char *iter = (char*)stack - RED_ZONE_SIZE;
    for (int i = 0; i < RED_ZONE_SIZE; ++i)
    {
        if (iter[i] != RED_ZONE_FLAG) return false;
    }

    // Check top of stack
    iter = (char*)stack + FIBER_STACK_SIZE;
    for (int i = 0; i < RED_ZONE_SIZE; ++i)
    {
        if (iter[i] != RED_ZONE_FLAG) return false;
    }

    return true;
}

//----------------------------------------------------------------------------------------------------------//
// Job Queue
//----------------------------------------------------------------------------------------------------------//

// When a processor dequeues an item, it will swap in to the cell one of the two
// NULLs in such away that two consecutive dequeue operations on the same cell
// give different NULL values to the cell
constexpr u64 QUEUE_EMPTY   = 0;
constexpr u64 QUEUE_REMOVED = 1;

/* This is an implementation of a multi producer and consumer 
 * Non-Blocking Concurrent FIFO Queue based on the paper from Phillippas Tsigas 
 * and Yi Zhangs: www.cse.chalmers.se/~tsigas/papers/latest-spaa01.pdf 
 * And implementation: https://gist.github.com/vurtun/e051c955b68c719462b594347184ac35 
 */
struct JobQueue
{
    FiberJob    *jobs[MAX_WORK_QUEUE_JOBS];
    volatile u32 head;
    volatile u32 tail;
};

// Returns true if empty - when the callback is null
constexpr b8 
job_queue_is_empty(JobQueue *queue)
{
    return queue->jobs[queue->head] == QUEUE_EMPTY;
}

static void 
job_queue_init(JobQueue *queue)
{
    memset(queue, 0, sizeof(*queue));
    queue->jobs[0] = QUEUE_EMPTY;
    queue->head = 0;
    queue->tail = 1;
}

static b8
job_queue_entry_free(FiberJob *j)
{
    return (uptr)j == QUEUE_EMPTY || (uptr)j == QUEUE_REMOVED;
}

static int 
job_queue_push(JobQueue *q, FiberJob *job)
{
    while (1)
    {
        // Read tail
        u32 te = q->tail;
        u32 ate = te;
        FiberJob *tt = q->jobs[ate];
        u32 tmp = (ate + 1) & WORK_QUEUE_MASK;
        FiberJob *tnew;

        // Find the actual tail - maybe the tail was updated since we assigned it?
        while (!(job_queue_entry_free(tt)))
        {
            // Check tail constitency
            if (te != q->tail) goto retry;
            // Check if queue is full
            if (tmp == q->head) break;
            tt = q->jobs[tmp];
            ate = tmp;
            tmp = (ate + 1) & WORK_QUEUE_MASK;
        }

        // Check tail consistency...again
        if (te != q->tail) continue;

        // Check if full
        if (tmp == q->head)
        {
            ate = (tmp + 1) & WORK_QUEUE_MASK;
            tt = q->jobs[ate];
            if (!(job_queue_entry_free(tt)))
                return 0; // queue was full
        
            // Let pop update the header
            compare_and_swap_32(&q->head, tmp, ate);
            continue;
        }

        if ((uptr)tt == QUEUE_REMOVED)
        {
            job = (FiberJob*)((uptr)job | 0x01);
        }

        if (te != q->tail) continue;

        if (compare_and_swap_ptr(&q->jobs[ate], tt, job))
        {
            if ((tmp & 1) == 0)
            {
                compare_and_swap_32(&q->tail, te, tmp);
            }
            return 1;
        }
retry:;
    }
}

static int
job_queue_pop(FiberJob **job, JobQueue *q)
{
    while (1)
    {
        u32 th = q->head;
        u32 tmp = (th + 1) & WORK_QUEUE_MASK;
        FiberJob *tt = q->jobs[tmp];
        FiberJob *tnull = 0;

        // Find the actual head
        while (job_queue_entry_free(tt))
        {
            if (th != q->head) goto retry;
            if (tmp == q->tail) return 0; // empty queue?
            tmp = (tmp + 1) & WORK_QUEUE_MASK;
            tt = q->jobs[tmp];
        }

        // Check head's consistency
        if (th != q->head) continue;

        // Check if queue is empty
        if (tmp == q->tail)
        {
            // Help push update end
            compare_and_swap_32(&q->tail, tmp, (tmp+1) & WORK_QUEUE_MASK);
            continue; // retry
        }

        tnull = ((uptr)tt & 0x01) ? (FiberJob*)QUEUE_REMOVED : (FiberJob*)QUEUE_EMPTY;
        if (th != q->head) continue;

        // Get the actual head
        if (compare_and_swap_ptr(&q->jobs[tmp], tt, tnull))
        {
            if ((tmp & 0x01) == 0)
            {
                compare_and_swap_32(&q->head, th, tmp);
            }

            *job = (FiberJob*)((uptr)tt & ~(uptr)1);
            return 1;
        }
retry:;
    }
}

//----------------------------------------------------------------------------------------------------------//
// Fiber
//----------------------------------------------------------------------------------------------------------//

struct SchedFiber
{
    SchedFiber       *next_fiber; // these are for the free list right?
    SchedFiber       *prev_fiber;
    struct Scheduler *sched;
    // Note(Dustin): I could probably get away with this
    // ptr if I assume fiber.rsp will set correctly?
    // TODO(Dustin): Remove after stress testing  
    FiberStack        stack;
    FiberContext      ctx;
    FiberJob          job;
    u32               wait_value;
};

//----------------------------------------------------------------------------------------------------------//
// Scheduler
//----------------------------------------------------------------------------------------------------------//

struct SchedWorker
{
    u32         tid;          // thread id
    u32         tindex;       // index into thread pool
    DWORD       tls;          // thread local storage index
    HANDLE      handle;       // thread handle
    SchedFiber *active_fiber; // Active fiber if not null
    Scheduler  *scheduler;    // Back ptr to scheduler

    // TODO(Dustin): Tagged Heap
};

struct Scheduler
{
    volatile u32 active;

    // A single queue for each priority level.
    JobQueue     job_queues[SchedulerPriority_Count];
    
    Sem          sem_work; // Mutex for when threads sleep
    volatile u32 active_threads;
    u32          thread_count;
    SchedWorker  workers[MAX_WORKER_THREADS];

    SchedFiber   fibers[MAX_FIBERS];
    u32          next_fiber_idx;
    volatile u32 free_list_lock;
    SchedFiber  *free_list;
    volatile u32 wait_list_lock;
    SchedFiber  *wait_list;

    // Stack allocation
    FiberStackAllocator stack_allocator;
};

static SchedWorker* 
get_sched_worker(u32 tid)
{
    for (int i = 0; i < scheduler->thread_count; ++i)
        if (scheduler->workers[i].tid == tid) 
            return scheduler->workers + i;
    return 0;
}

static void 
sched_add_to_list(SchedFiber **list, SchedFiber *fiber, spinlock *lock)
{
    Assert(lock);
    spinlock_enter(lock);
    
    if (!*list)
    {
        *list = fiber;
        fiber->prev_fiber = 0;
        fiber->next_fiber = 0;
    }
    else
    {
        fiber->prev_fiber = 0;
        fiber->next_fiber = *list;
        (*list)->prev_fiber = fiber;
        *list = fiber;
    }
    
    spinlock_leave(lock);
}

static void 
sched_remove_from_list(SchedFiber **list, SchedFiber *fiber, spinlock *lock)
{
    if (lock) spinlock_enter(lock);

    if (fiber->next_fiber)
        fiber->next_fiber->prev_fiber = fiber->prev_fiber;
    if (fiber->prev_fiber)
        fiber->prev_fiber->next_fiber = fiber->next_fiber;
    if (*list == fiber)
        *list = fiber->next_fiber;

    fiber->next_fiber = 0;
    fiber->prev_fiber = 0;

    if (lock) spinlock_leave(lock);
}

static SchedFiber* 
sched_wake_fiber(Scheduler *sched)
{
    spinlock_enter(&sched->wait_list_lock);
    SchedFiber *iter = sched->wait_list;
    while (iter)
    {
        if (*iter->job.job_count == iter->wait_value) break;
        iter = iter->next_fiber;    
    }
    if (iter) sched_remove_from_list(&sched->wait_list, iter, 0);
    spinlock_leave(&sched->wait_list_lock);
    return iter;
}

JOB_ENTRY(dummy_job) {}

static void 
fiber_stack_reset(SchedFiber *fiber)
{
    // Setup stack space
    void *stack = fiber->stack;
    if (!stack)
    {
        fiber->stack = fiber_stack_allocator_alloc(&scheduler->stack_allocator);
        stack = fiber->stack;
    }
    
    bool red_zone_valid = fiber_stack_check_redzone(fiber->stack);
    AssertCustom(red_zone_valid, "Red Zone has been violated!");

    char *iter = (char*)(stack);
    iter = (char*)((char*)iter + FIBER_STACK_SIZE);
    iter = (char*)((uptr)iter & -16L);
    AssertCustom(((uptr)iter % 16L) == 0, "Allocated stack should be aligned to 16L");
    // Make 128 byte scratch space for the Red Zone. This arithmetic will not unalign
    // our stack pointer because 128 is a multiple of 16. The Red Zone must also be
    // 16-byte aligned.
    iter -= 128;
    fiber->ctx.rsp = iter;
    fiber->ctx.rip = dummy_job;
}

static SchedFiber*
sched_get_fiber(Scheduler *sched)
{
    // Wake any fibers that are finished waiting
    SchedFiber *result = sched_wake_fiber(sched);
    // Otherwise, acquire a new fiber
    if (!result)
    {
        spinlock_enter(&sched->free_list_lock);
        if (sched->next_fiber_idx < MAX_FIBERS)
        {
            result = sched->fibers + sched->next_fiber_idx;
            result->stack = 0;
            ++sched->next_fiber_idx;
        }
        else if (sched->free_list)
        {
            result = sched->free_list;
            sched_remove_from_list(&sched->free_list, result, 0);
        }
        spinlock_leave(&sched->free_list_lock);
    }

    fiber_stack_reset(result);
    return result;
}

FORCE_INLINE FiberJob*
sched_queue_job(JobQueue *qs, int qc)
{
    FiberJob *cjob = 0;
    for (int j = 0; j < qc; ++j)
    {
        if (job_queue_pop(&cjob, &qs[j])) 
        {
            return cjob;
        }
    }
    return 0;
}

constexpr int MAX_SPIN_COUNT = 4096;
static FiberJob* 
sched_acquire_job(JobQueue *queues, int queue_count)
{
    FiberJob *job = 0;
    // Thread Sleep Loop 
    // https://software.intel.com/content/www/us/en/develop/articles/benefitting-power-and-performance-sleep-loops.html
    while ((job = sched_queue_job(queues, queue_count)) == 0)
    {
        for (int i = 0; i < MAX_SPIN_COUNT; ++i)
        {
            _mm_pause();
            if ((job = sched_queue_job(queues, queue_count)) != 0) return job;
        }
        Sleep(0); // Yield this thread's time slice
    }
    return job;
}

static void 
sched_fiber_work_proc()
{
    u32 tid = GetCurrentThreadId();
    SchedWorker *worker = get_sched_worker(tid);
    Assert(worker);

    Scheduler *sched = worker->scheduler;
    InterlockedIncrement(&sched->active_threads);

    while (1)
    {
        SchedFiber *fiber = sched_wake_fiber(sched);
        if (fiber)
        {
            SchedFiber *old_fiber = worker->active_fiber;
            FiberContext old_ctx = old_fiber->ctx;

            memset(worker->active_fiber, 0, sizeof(*worker->active_fiber));
            sched_add_to_list(&sched->free_list, old_fiber, &sched->free_list_lock);

            worker->active_fiber = fiber;
            
            Assert(fiber->ctx.rip);
            swap_fiber_ctx(&old_ctx, &fiber->ctx);
            goto LBL_FIBER_LOOP_RETRY;
        }

        fiber_stack_reset(worker->active_fiber);
        FiberJob *job = sched_acquire_job(sched->job_queues, SchedulerPriority_Count);
        Assert(job);
        Assert(job->callback);
        worker->active_fiber->job = *job;
        job->callback(sched, job->data);
        InterlockedDecrement(job->job_count);
        u32 dummy = 0;

#if 0
        if (!(job_queue_pop(&job, &sched->job_queues[SchedulerPriority_High])) && 
            !(job_queue_pop(&job, &sched->job_queues[SchedulerPriority_Normal])) &&
            !(job_queue_pop(&job, &sched->job_queues[SchedulerPriority_Low])))
        { // Could not find a job
            InterlockedDecrement(&sched->active_threads);
            sem_wait(&sched->sem_work);
            InterlockedIncrement(&sched->active_threads);
        }
        else
        {
            Assert(job->callback);
            worker->active_fiber->job = *job;
            job->callback(sched, job->data);
            InterlockedDecrement(job->job_count);
            u32 dummy = 0;
        }
#endif

LBL_FIBER_LOOP_RETRY:;
    }

    ExitThread(1);
}

DWORD WINAPI 
sched_thread_proc(_In_ LPVOID lpParameter)
{
    SchedWorker *worker = (SchedWorker*)lpParameter;
    Scheduler *sched = worker->scheduler;
    worker->active_fiber = sched_get_fiber(sched);
    
    // Initialize the TLS index for this thread. 
    LPVOID lpvData; 
    lpvData = (LPVOID) LocalAlloc(LPTR, sizeof(LPVOID));
    Assert(lpvData);
    *((uptr*)lpvData) = (uptr)lpParameter;
    if (!TlsSetValue(worker->tls, lpvData)) 
      LogError("TlsSetValue error"); 

    //save_fiber_ctx(&worker->active_fiber->ctx);
    sched_fiber_work_proc();
    return 0;
}

void
scheduler_init(Scheduler **_scheduler)
{
    scheduler = (Scheduler*)malloc(sizeof(Scheduler));
    ZeroMemory(scheduler, sizeof(Scheduler));
    scheduler->next_fiber_idx = 0;
    scheduler->free_list_lock = 0;
    scheduler->wait_list_lock = 0;
    scheduler->active_threads = 0;
    scheduler->free_list = 0;
    scheduler->wait_list = 0;
    sem_init(&scheduler->sem_work);
    fiber_stack_allocator_init(&scheduler->stack_allocator);

    for (u32 i = 0; i < SchedulerPriority_Count; ++i)
    {
        job_queue_init(scheduler->job_queues + i);
    }

    InterlockedExchange(&scheduler->active, 1);

    Win32ProcessorInfo processor_info;
    Win32GetProcessorInfo(&processor_info);
    u32 thread_count = processor_info.logical_processor_count;
    Assert(thread_count <= MAX_WORKER_THREADS);
    scheduler->thread_count = thread_count;

    for (u32 i = 0; i < thread_count - 1; ++i)
    {
        scheduler->workers[i].tindex = i;

        if ((scheduler->workers[i].tls = TlsAlloc()) == TLS_OUT_OF_INDEXES) 
            LogFatal("TlsAlloc failed"); 

        scheduler->workers[i].handle = CreateThread(
            NULL, 
            0, 
            sched_thread_proc, 
            scheduler->workers + i, 
            0, // CREATE_SUSPENDED 
            (LPDWORD)&scheduler->workers[i].tid);
        scheduler->workers[i].active_fiber = 0;
        scheduler->workers[i].scheduler    = scheduler;
    }

    // Initialize the main thread as a worker thread
    scheduler->workers[thread_count - 1].handle = GetCurrentThread();
    scheduler->workers[thread_count - 1].tid = GetCurrentThreadId();
    scheduler->workers[thread_count - 1].scheduler = scheduler;
    scheduler->workers[thread_count - 1].active_fiber = sched_get_fiber(scheduler);

    LogInfo("Main thread id: %ld", scheduler->workers[thread_count - 1].tid);

#if 0
    for (u32 i = 0; i < processor_info.physical_core_count; ++i)
    {
        u32 mask = 1 << i;

        DWORD_PTR ret = SetThreadAffinityMask(
            scheduler->workers[i * processor_info.thread_per_processor + 0].handle, 
            mask
        );
        AssertCustom(ret != 0, "Failed to set a thread's affinity!");

        DWORD last_err = GetLastError();
        AssertCustom(last_err != ERROR_INVALID_PARAMETER, "Requested processor not selected for affinity mask!");

        ret = SetThreadAffinityMask(
            scheduler->workers[i * processor_info.thread_per_processor + 1].handle, 
            mask
        );
        AssertCustom(ret != 0, "Failed to set a thread's affinity!");

        last_err = GetLastError();
        AssertCustom(last_err != ERROR_INVALID_PARAMETER, "Requested processor not selected for affinity mask!");
    }
#endif

#if 0
    for (u32 i = 0; i < thread_count - 1; ++i)
    {
        ResumeThread(sched->workers[i].handle);
    }
#endif
}

void 
scheduler_free(Scheduler **_scheduler)
{
    Scheduler *sched = scheduler;

    //while (sched->active_threads > 1) {}

    InterlockedExchange(&scheduler->active, 0);


    //sem_free(&sched->sem_work);
    fiber_stack_allocator_free(&sched->stack_allocator);

    // Disallow picking up new jobs
    sched->wait_list = 0;
    sched->free_list = 0;

    //free(sched);
    //*scheduler = 0;
}

void 
scheduler_run_jobs(
    Scheduler            *_scheduler, 
    SchedulerPriority     priority, 
    FiberJob             *jobs, 
    int                   jobs_count, 
    volatile job_counter *counter) 
{
    AssertCustom(priority < SchedulerPriority_Count, "Invalid priority. Possible priority values {Low (0), Normal (1), High (2)}");
    JobQueue *queue = &scheduler->job_queues[priority];

    for (int i = 0; i < jobs_count; ++i)
    {
        jobs[i].job_count = counter;
        job_queue_push(queue, jobs + i);
        //sem_post(&scheduler->sem_work);
    }
}

// Allows a fiber to sleep until the counter reaches the expected value. When the fiber wakens,
// it will be placed into the high priority queue.
void scheduler_await(Scheduler *_sched,  volatile job_counter *counter, uint32_t expected_value)
{
    DWORD tid = GetCurrentThreadId();
    SchedWorker *worker = get_sched_worker(tid);
    Assert(worker);

    SchedFiber *old = worker->active_fiber;
    old->wait_value = expected_value;
    old->job.job_count = counter;
    sched_add_to_list(&scheduler->wait_list, old, &scheduler->wait_list_lock);

    worker->active_fiber = sched_wake_fiber(scheduler);
    if (!worker->active_fiber)
    {
        worker->active_fiber = sched_get_fiber(scheduler);
        worker->active_fiber->ctx.data = worker;
        worker->active_fiber->ctx.rip  = sched_fiber_work_proc;
    }
    swap_fiber_ctx(&old->ctx, &worker->active_fiber->ctx);
}