//
// Created by chenzeyu on 2024/1/11.
//

#ifndef CZYSERVER_MPSCQ_H
#define CZYSERVER_MPSCQ_H
#include <atomic>
#include <cassert>
#include <cstddef>
#include <mutex>

namespace crpc_core
{

    class MPSCQueue{
    public:
        struct Node{
            std::atomic<Node*> next;
        };

        MPSCQueue():
            head_{&stub_},tail_{&stub_}
        {
        }

        ~MPSCQueue(){
            assert(head_.load(std::memory_order_relaxed) == &stub_);
            assert(tail_ == &stub_);
        }

        /// return true or false only depend on new node
        /// is the first node
        bool Push(Node* node);

        Node *Pop();

        Node *PopAndCheckEnd(bool &empty);

    private:
        union{
            char padding_[alignof(std::max_align_t)];
            std::atomic<Node*> head_{nullptr};
        };

        Node* tail_;
        Node stub_;
    };

    class LockedMPSCQueue{
    public:
        typedef MPSCQueue::Node Node;

        bool Push(Node* node);

        Node *TryPop();

        Node *Pop();
    private:
        std::mutex mux_;
        MPSCQueue  queue_;

    };

}

#endif //CZYSERVER_MPSCQ_H