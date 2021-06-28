#ifndef _INPUT_H
#define _INPUT_H

struct PlayerMovement
{
    // Keyboard keys
    u8 w:1;
    u8 s:1;
    u8 a:1;
    u8 d:1;
    u8 q:1;
    u8 pad0:3;
    
    // Mouse keys
    u8 m1:1;
    u8 m2:1;
    u8 pad1:6;
    
    // Special keys
    u8 ctrl:1;
    u8 shift:1;
    u8 alt:1;
    
    // Mouse position
    r64 old_mouse_x;
    r64 old_mouse_y;
    r64 mouse_x;
    r64 mouse_y;
    
    // NOTE(Dustin): Not really working right now.
    // Tabbing this for later.
    // Mouse scroll wheel
    r64 mouse_scroll_delta;
    r64 mouse_hscroll_delta;
};

#endif //_INPUT_H
