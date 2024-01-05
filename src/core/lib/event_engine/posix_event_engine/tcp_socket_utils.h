//
// Created by czy on 2024/1/4.
//

#ifndef CZYSERVER_TCP_SOCKET_UTILS_H
#define CZYSERVER_TCP_SOCKET_UTILS_H

#include<cstdlib>



namespace crpc_event_engine{
struct PosixTcpOptions{
    static constexpr int kDefaultReadChunkSize = 8192;
    static constexpr int kDefaultMinReadChunksize = 256;
    static constexpr int kDefaultMaxReadChunksize = 4 * 1024 * 1024;
    static constexpr int kZerocpTxEnabledDefault = 0;
    static constexpr int kMaxChunkSize = 32 * 1024 * 1024;
    static constexpr int kDefaultMaxSends = 4;
    static constexpr size_t kDefaultSendBytesThreshold = 16 * 1024;
    // Let the system decide the proper buffer size.
    static constexpr int kReadBufferSizeUnset = -1;
    static constexpr int kDscpNotSet = -1;
    int tcp_read_chunk_size = kDefaultReadChunkSize;
    int tcp_min_read_chunk_size = kDefaultMinReadChunksize;
    int tcp_max_read_chunk_size = kDefaultMaxReadChunksize;
    int tcp_tx_zerocopy_send_bytes_threshold = kDefaultSendBytesThreshold;
    int tcp_tx_zerocopy_max_simultaneous_sends = kDefaultMaxSends;
    int tcp_receive_buffer_size = kReadBufferSizeUnset;
    bool tcp_tx_zero_copy_enabled = kZerocpTxEnabledDefault;
    int keep_alive_time_ms = 0;
    int keep_alive_timeout_ms = 0;
    bool expand_wildcard_addrs = false;
    bool allow_reuse_port = false;
    int dscp = kDscpNotSet;


};

}


#endif //CZYSERVER_TCP_SOCKET_UTILS_H
