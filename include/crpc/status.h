//
// Created by czy on 2023/12/25.
//

#ifndef CZYSERVER_STATUS_H
#define CZYSERVER_STATUS_H

enum class Status{
    kOk = 0,
    kCancelled,
    kUnknown,
    kInvalidArgument,
    kDeadlineExceeded,
    kNotFound,
    kAlreadyExists,
    kPermissionDenied,
    kResourceExhausted,
    kFailedPrecondition,
    kAborted,
    kOutOfRange,
    kUnimplemented,
    kInternal,
    kUnavailable,
    kDataLoss,
};
#endif //CZYSERVER_STATUS_H
