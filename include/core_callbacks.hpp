#ifndef CORE_CALLBACKS_HPP
#define CORE_CALLBACKS_HPP
#include "nadi.h"

extern "C" {
    void callback(nadi_message* msg);

    void free_msg(nadi_message* msg);
}


#endif