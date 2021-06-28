#ifndef _FRAME_PARAMS_H
#define _FRAME_PARAMS_H

struct FrameParams
{
    // TODO(Dustin): Timer
    
    u64                   frame;
    struct ControlState*  input_state;
    struct SimpleTerrain* terrain;
};

#endif //_FRAME_PARAMS_H
