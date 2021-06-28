
// Unity build file for the engine + game

//~ Global Headers
#include <jackal_types.h>
#include <platform.h>


//~ CLib  Headers
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

// NOTE(Dustin): Where is this actually used?
#include <stddef.h>


// Used for determining if a registered component
// inherits from IComponent
#include <type_traits>
// needed for placement new
#include <new>
// NOTE(Dustin): Maybe can replace with hashtable
#include <set>
// NOTE(Dustin): Maybe can replace with jstring
#include <string>
#include <new>

// NOTE(Dustin): I don't think this header is actually needed
#include <iostream>

// NOTE(Dustin): Handle this better: Used for VK_CHECK_RESULT
#include <stdexcept>

// NOTE(Dustin): Is this still necessary?
#include <optional>

//~ Source Files
#include "engine/engine_unity.cpp"
#include "game/splicer_unity.cpp"

