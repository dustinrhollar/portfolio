#include "ecs.h"
#include "entity.h"
#include "component.h"
#include "system.h"

namespace jengine { namespace ecs {

void InitializeECS()
{
    IntializeEntityRegistry();
    InitializeComponentRegistry();
    InitializeSystemRegistry();
}

void ShutdownECS()
{
    ShutdownSystemRegistry();
    ShutdownComponentRegistry();
    ShutdownEntityRegistry();
}

} // ecs
} // jengine