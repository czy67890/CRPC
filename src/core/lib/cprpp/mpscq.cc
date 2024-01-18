//
// Created by chenzeyu on 2024/1/11.
//

#include "core/lib/cprpp/mpscq.h"
namespace crpc_core{
    bool MPSCQueue::Push(Node* node){
        /// here use a very good skill
        /// by exchange this read-modify-write op
        /// make Push op safe and easy
        node->next.store(nullptr,std::memory_order_relaxed);
        Node *prev = head_.exchange(node,std::memory_order_acq_rel);
        prev->next.store(node,std::memory_order_release);
        return prev == &stub_;
    }

    MPSCQueue::Node* MPSCQueue::Pop() {
        bool empty;
        return PopAndCheckEnd(empty);
    }

    MPSCQueue::Node* MPSCQueue::PopAndCheckEnd(bool &empty) {
        Node *tail = tail_;
        Node *next = tail_->next.load(std::memory_order_acquire);

        if(tail == &stub_){
            if(next == nullptr){
                empty = true;
                return nullptr;
            }
            tail_ = next;
            tail = next;
            next = tail->next.load(std::memory_order_acquire);
        }

        if(next != nullptr)[[likely]]{
            tail_ = next;
            return tail;
        }

        Node *head = head_.load(std::memory_order_acquire);
        if(tail != head){
            empty = false;
            return nullptr;
        }

        Push(&stub_);
        next = tail->next.load(std::memory_order_acquire);
        if(next != nullptr){
            empty = false;
            tail_ = next;
            return tail;
        }
        empty = false;
        return nullptr;
    }

    bool LockedMPSCQueue::Push(Node* node){
        return queue_.Push(node);
    }

    LockedMPSCQueue::Node *LockedMPSCQueue::TryPop(){
        if(mux_.try_lock()){
            auto node = queue_.Pop();
            mux_.unlock();
            return node;
        }
        return nullptr;
    }

    LockedMPSCQueue::Node *LockedMPSCQueue::Pop(){
        std::lock_guard<std::mutex> lk{mux_};
        bool empty = false;
        Node *node;
        do{
            node = queue_.PopAndCheckEnd(empty);
        }while(node == nullptr && !empty);

        return node;
    }

}
