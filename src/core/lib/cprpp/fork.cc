//
// Created by chenzeyu on 2024/1/15.
//
#include "core/lib/cprpp/fork.h"
#include "core/lib/event_engine/thread_local.h"
#include "core/lib/cprpp/no_destruct.h"
#include <mutex>
#include <condition_variable>
namespace crpc_core {
    namespace {

#define UNBLOCKED(n) ((n) +2)
#define BLOCKED(n) (n)

        class ExecCtxState {
        public:
        ExecCtxState()
        : fork_complete_(true) {
            blocked_.store(UNBLOCKED(0), std::memory_order_release);
        }

        void IncExecCtxCount() {
            if (crpc_event_engine::ThreadLocal::GetIsEventEngineThreadLocal()) {
                return;
            }

            size_t count = blocked_.load(std::memory_order_relaxed);
            while (true) {
                if (count <= BLOCKED(1)) {
                    std::unique_lock<std::mutex> lk{mux_};
                    if (count = blocked_.load(std::memory_order_acquire);count <= BLOCKED(1)) {
                        cond_.wait(lk, [this]() {
                            return !fork_complete_;
                        });
                    }
                } else if (blocked_.compare_exchange_strong(count, count + 1, std::memory_order_acq_rel)) {
                    break;
                }
                count = blocked_.load(std::memory_order_acquire);
            }
        }

        void DecExecCtx() {
            if (crpc_event_engine::ThreadLocal::GetIsEventEngineThreadLocal()) {
                return;
            }
            blocked_.fetch_add(-1);
        }

        bool BlockExecCtx() {
            size_t count = UNBLOCKED(1);
            if (blocked_.compare_exchange_strong(count, BLOCKED(1))) {
                std::lock_guard<std::mutex> lk{mux_};
                fork_complete_ = false;
                return true;
            }
            return false;
        }

        void AllowExecCtx() {
            std::unique_lock<std::mutex> lk{mux_};
            blocked_.store(UNBLOCKED(0));
            fork_complete_ = true;
            cond_.notify_all();
        }


        private:
        bool fork_complete_;
        std::mutex mux_;
        std::condition_variable cond_;
        std::atomic<size_t> blocked_;
        };

        class ThreadCount {
        public:
        void IncThreadCount() {
            std::lock_guard<std::mutex> lk{mux_};
            count_++;
        }

        void DecThreadCount() {
            std::lock_guard<std::mutex> lk{mux_};
            count_--;
            if (count_ == 0 && awaiting_thread_) {
                thread_done_ = true;
                cond_.notify_all();
            }
        }

        void AwaitThreads() {
            std::unique_lock<std::mutex> lk{mux_};
            awaiting_thread_ = true;
            thread_done_ = (count_ == 0);
            cond_.wait(lk, [this]() {
                return !thread_done_;
            });
        }

        private:
        bool thread_done_{false};
        bool awaiting_thread_{false};
        std::mutex mux_;
        std::condition_variable cond_;
        size_t count_{0};
        };

    }//namespace

    void Fork::GlobalInit() {
        if (!override_enabled_) {
            ///TODO:: this should depends on the ConfVar
            support_enabled_.store(true);
        }
    }

    bool Fork::Enabled() {
        return support_enabled_.load();
    }

    void Fork::Enable(bool enable) {
        override_enabled_ = true;
        support_enabled_.store(enable);
    }

    void Fork::DoDecExecCtxCount() {
        NoDestructSingletion<ExecCtxState>::Get()->DecExecCtx();
    }

    void Fork::DoIncExecCtxCount() {
        NoDestructSingletion<ExecCtxState>::Get()->IncExecCtxCount();
    }


    bool Fork::RegisterResetChildPollingEngineFunc(crpc_core::Fork::child_postfork_func reset_child_polling_engine) {
        if (!reset_child_polling_engine_) {
            reset_child_polling_engine_ = new std::set<child_postfork_func>;
        }
        auto ret = reset_child_polling_engine_->insert(reset_child_polling_engine);
        return ret.second;
    }

    const std::set<Fork::child_postfork_func>& Fork::GetResetChildPollingEngineFunc() {
        if (!reset_child_polling_engine_) {
            reset_child_polling_engine_ = new std::set<child_postfork_func>;
        }
        return *reset_child_polling_engine_;
    }

    bool Fork::BlockExecCtx() {
        if(support_enabled_){
            return NoDestructSingletion<ExecCtxState>::Get()->BlockExecCtx();
        }
        return false;
    }

    void Fork::AllowExecCtx() {
        if(support_enabled_){
            NoDestructSingletion<ExecCtxState>::Get()->AllowExecCtx();
        }
    }

    void Fork::IncThreadCount() {
        if(support_enabled_){
            NoDestructSingletion<ThreadCount>::Get()->IncThreadCount();
        }
    }

    void Fork::AwaitThreads() {
        if(support_enabled_){
            NoDestructSingletion<ThreadCount>::Get()->AwaitThreads();
        }
    }

    void Fork::DecThreadCount() {
        if(support_enabled_){
            NoDestructSingletion<ThreadCount>::Get()->DecThreadCount();
        }
    }

    std::atomic<bool> Fork::support_enabled_{false};
    bool Fork::override_enabled_{false};
    std::set<Fork::child_postfork_func>*  reset_child_polling_engine_{nullptr};

}