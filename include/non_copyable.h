#pragma once
namespace crpc_core{

class NonCopyable{
public:
    NonCopyable() = default;
    
    ~NonCopyable() = default;

    NonCopyable(const NonCopyable & rhs) = delete;

    NonCopyable & operator=(const NonCopyable &rhs) = delete;

    NonCopyable(NonCopyable && rhs) = default;

    NonCopyable& operator=(NonCopyable &&rhs) = default;
};

}