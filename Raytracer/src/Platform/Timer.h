#ifndef _TIMER_H
#define _TIMER_H

typedef struct 
{
    u64 start;
} Timer;

void global_timer_setup();
void timer_begin(Timer *timer);
r32  timer_seconds_elapsed(Timer *timer);
r32  timer_mili_seconds_elapsed(Timer *timer);
r32  timer_nano_seconds_elapsed(Timer *timer);

FORCE_INLINE r32 seconds_to_nano_seconds(r32 Seconds)
{
    return Seconds * 1000000000.0f;
}

FORCE_INLINE r32 nano_seconds_to_seconds(r32 NanoSeconds)
{
    return NanoSeconds / 1000000000.0f;
}

FORCE_INLINE r32 seconds_to_mili_seconds(r32 Seconds)
{
    return Seconds * 1000.0f;
}

FORCE_INLINE r32 mili_seconds_to_seconds(r32 MiliSeconds)
{
    return MiliSeconds / 1000.0f;
}

#endif //_WIN32_TIMER_H
