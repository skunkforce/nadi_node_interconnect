/**
 * @file nadi.h
 * @brief This file defines an API for datastreaming nodes to interact with eachother through.
 * @author Odin Holmes
 * @email odinthenerd@gmail.com
 *
 * This interface provides a minimalistic API for datastream producers and consumers to interact with each other.
 * It uses the common meta + data pattern where the meta portion is a json string which is extensible and fascilitates 
 * layered and versined support where as the data is binary which provides efficientcy. 
 */


 #ifndef NADI_H
 #define NADI_H

#include <stddef.h>

#ifdef _WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

//serves as a unique connection id. A DLL can be loaded multiple times each producing a unique instance handle
typedef void* nadi_instance_handle;

typedef int nadi_status;

struct nadi_message;

typedef void(*nadi_receive_callback)(nadi_message*);
typedef void(*nadi_free_callback)(nadi_message*);

struct nadi_message {
    char* meta;  //null terminated JSON string
    unsigned long meta_hash; //value 0 is unused
    char* data; //raw bytes interpreted based on the contents of meta
    unsigned int data_length;
    nadi_free_callback free;
    nadi_instance_handle instance;
    unsigned int channel;
};

DLL_EXPORT nadi_status nadi_init(nadi_instance_handle* instance, nadi_receive_callback);

DLL_EXPORT nadi_status nadi_deinit(nadi_instance_handle instance);

DLL_EXPORT nadi_status nadi_send(nadi_message* message, nadi_instance_handle instance);

DLL_EXPORT void nadi_free(nadi_message*);

//returns a non owning const string containing a JSON description of the nodes version, capabilities etc.
DLL_EXPORT char* nadi_descriptor();

#ifdef __cplusplus
}
#endif

#endif // UADI.h