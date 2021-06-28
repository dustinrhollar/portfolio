#ifndef _TIMER_H
#define _TIMER_H

struct Timer 
{
    u64 start;
    
    void Begin();
    
    r32 SecondsElapsed();
    r32 MiliSecondsElapsed();
    r32 NanoSecondsElapsed();
};

void GlobalTimerDataSetup();

inline r32 SecondsToNanoSeconds(r32 Seconds)
{
    return Seconds * 1000000000.0f;
}

inline r32 NanoSecondsToSeconds(r32 NanoSeconds)
{
    return NanoSeconds / 1000000000.0f;
}

inline r32 SecondsToMiliSeconds(r32 Seconds)
{
    return Seconds * 1000.0f;
}

inline r32 MiliSecondsToSeconds(r32 MiliSeconds)
{
    return MiliSeconds / 1000.0f;
}

#endif //_WIN32_TIMER_H
