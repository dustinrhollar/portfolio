
file_global i64 g_performance_frequency;

void GlobalTimerDataSetup()
{
    LARGE_INTEGER perf_count_frequency_result;
    QueryPerformanceFrequency(&perf_count_frequency_result);
    g_performance_frequency = perf_count_frequency_result.QuadPart;
    
    UINT desired_scheduler_ms = 1;
    // Check the time result?
    (timeBeginPeriod(desired_scheduler_ms) == TIMERR_NOERROR);
}

void Timer::Begin()
{
    LARGE_INTEGER local_start;
    QueryPerformanceCounter(&local_start);
    start = local_start.QuadPart;
}

r32 Timer::SecondsElapsed()
{
    LARGE_INTEGER local_counter;
    QueryPerformanceCounter(&local_counter);
    
    r32 Result = ((r32)(local_counter.QuadPart - start) / (r32)g_performance_frequency);
    return(Result);
}

r32 Timer::MiliSecondsElapsed()
{
    LARGE_INTEGER local_counter;
    QueryPerformanceCounter(&local_counter);
    
    r32 Result = ((r32)(local_counter.QuadPart - start) * 1000 / (r32)g_performance_frequency);
    return(Result);
}

r32 Timer::NanoSecondsElapsed()
{
    LARGE_INTEGER local_counter;
    QueryPerformanceCounter(&local_counter);
    
    r32 Result = ((r32)(local_counter.QuadPart - start) * 1000000000.0f / (r32)g_performance_frequency);
    return(Result);
}
