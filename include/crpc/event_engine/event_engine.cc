#include "crpc/event_engine/event_engine.h"
#include "event_engine.h"

namespace crpc_event_engine{
    const EventEngine::TaskHandle EventEngine::TaskHandle::kInvalid = {-1,-1};

}

bool crpc_event_engine::operator==(const EventEngine::TaskHandle & lsh, const EventEngine::TaskHandle & rhs)
{
    return lsh.keys[0] == rhs.keys[0] && rhs.keys[1] == rhs.keys[1];
}

bool crpc_event_engine::operator!=(const EventEngine::TaskHandle & lsh, const EventEngine::TaskHandle & rhs)
{
    return !(lsh == rhs);
}
