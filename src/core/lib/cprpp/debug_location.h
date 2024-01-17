//
// Created by chenzeyu on 2024/1/12.
//

#ifndef CZYSERVER_DEBUG_LOCATION_H
#define CZYSERVER_DEBUG_LOCATION_H

#if defined(__has_builtin)
#if __has_builtin(__builtin_FILE)
#define CRPC_DEFAULT_FILE __builtin_FILE()
#endif
#ifndef GRPC_DEFAULT_FILE
#define GRPC_DEFAULT_FILE "<unknown>"
#endif

#if defined(__has_builtin)
#if __has_builtin(__builtin_LINE)
#define CRPC_DEFAULT_LINE __builtin_LINE()
#endif
#endif
#ifndef CRPC_DEFAULT_LINE
#define CRPC_DEFAULT_LINE -1
#endif

namespace crpc_core{
class SourceLocation{
public:
    SourceLocation(const char *file = CRPC_DEFAULT_FILE,int line = CRPC_DEFAULT_LINE)
        :file_(file),line_(line)
    {
    }

    const char* File() const{
        return file_;
    }

    int Line() const{
        return line_;
    }
private:
    const char *file_;
    int line_;
};

#ifndef NDEBUG
class DebugLocation{
public:
    DebugLocation(const char * file = CRPC_DEFAULT_FILE,int line = CRPC_DEFAULT_LINE)
        :location_(file,line)
    {

    }

    const char * File  () const{
        return location_.File();
    };

    int Line() const{
        return location_.Line();
    }

private:
    SourceLocation location_;
};

#else
class DebugLocation{
public:
    DebugLocation(const char * file = CRPC_DEFAULT_FILE,int line = CRPC_DEFAULT_LINE)
        :location_(file,line)
    {

    }

    const char * File  () const{
        return location_.File();
    };

    int Line() const{
        return location_.Line();
    }

private:
    SourceLocation location_;
};

#endif

#define DEBUG_LOCATION ::crpc_core::DebugLocation(__FILE__,__LINE__)
}
#endif
#endif //CZYSERVER_DEBUG_LOCATION_H
