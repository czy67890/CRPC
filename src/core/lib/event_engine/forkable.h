#pragma once

#include <memory>
#include <vector>
#include <functional>
namespace crpc_event_engine{

class Forkable{
public:
    virtual ~Forkable() = default;
    virtual void Prefork() = 0;
    virtual void PostforkParent() = 0;
    virtual void PostforkChild() = 0;
};

class ObjectGroupForkHandler{
public:
    void RegisterForkable(std::shared_ptr<Forkable> forkable,
                          std::function<void(void)> prepare,
                          std::function<void(void)> parent,
                          std::function<void(void)> child);

    void Prefork();
    void PostforkParent() ;
    void PostforkChild() ;

private:
    bool registered_ = false;
    bool is_forking_ = false;
    std::vector<std::weak_ptr<Forkable>> forkables_;
};



}