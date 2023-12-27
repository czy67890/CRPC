//
// Created by czy on 2023/12/27.
//

#ifndef CZYSERVER_HARDWARE_VALUE_H
#define CZYSERVER_HARDWARE_VALUE_H
#include <new>
#include <cstdlib>
#include <unistd.h>
#include <thread>
constexpr size_t GetCacheLineSize(){
    constexpr size_t kCacheSize =
#if defined(__i386__) || defined(__x86_64__)
    1<<6
#else
    0
#endif
;
    return kCacheSize;
}



namespace crpc_hard_ware{
    struct Initer{
    friend class SysConf;
    private:
        Initer(){
            kCacheLine = sysconf(_SC_LEVEL1_DCACHE_LINESIZE); ;
        }
        size_t kCacheLine;
    };

    class SysConf{
    public:

        static size_t kCacheLine(){
            return initer.kCacheLine;
        }
    private:
        static Initer initer;
    };

}




#endif //CZYSERVER_HARDWARE_VALUE_H
