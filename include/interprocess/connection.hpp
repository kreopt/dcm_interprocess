#ifndef __INTERPROC_CONNECTION_H_
#define __INTERPROC_CONNECTION_H_
namespace interproc {
    enum class conn_type {
        tcp,
        unix,
        ipc
    };
}
#endif