#pragma once
#include "non_copyable.h"
namespace crpc_core{
    class NonCopyMoveable
        :public NonCopyable
    {   
    public:
        /// @brief  this must written because of when u 
        /// define  some Special Member Functions  the default 
        /// construction  will not generation by the compiler
        NonCopyMoveable() = default;


        NonCopyMoveable(NonCopyMoveable && rhs) = delete;
        
        NonCopyMoveable& operator=(NonCopyMoveable &&rhs) = delete;
    };

}