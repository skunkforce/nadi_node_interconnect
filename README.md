# Nadi Interconnect

## Overview
**Nadi Interconnect** is an application that orchestrates data streaming between nodes implementing the **NADI (Node Agnostic Datastream Interface)**, forming a programmable data flow graph. Developed by the `skunkforce` organization, it dynamically loads NADI-compliant libraries (DLLs on Windows, `.so` files on Linux/macOS) from a user-specified directory (default: `./nodes`), constructs nodes with unique instance identifiers, configures connections between node output and input streams across multiple channels, and manages message lifetimes for streams feeding multiple consumers. It’s ideal for IoT, sensor networks, data acquisition, or real-time analytics.

The NADI interface, defined in `include/nadi.h`, provides a minimalistic, platform-independent API for datastreaming, using a `meta` (JSON string) + `data` (binary) pattern. It supports dynamic library loading, reentrant C-style callbacks, and cross-language compatibility (e.g., C++, Python).

## Features
- **Dynamic Node Loading**: Loads NADI-compliant DLLs/`.so` files from a user-specified directory (default: `./nodes`) using `LoadLibrary` (Windows) or `dlopen` (Linux/macOS).
- **Node Construction/Destruction**: Constructs multiple instances from a single node library with unique identifiers; destructs instances as needed.
- **Programmable Stream Routing**: Connects and disconnects producer node output streams to consumer node input streams via JSON control messages.
- **Connection Management**: Queries active stream connections between instances.
- **Message Lifetime Management**: Uses `scope-guard` for single-consumer streams; multi-consumer support planned.
- **JSON Message Processing**: Parses `channel`, `meta`, and `data` fields from `stdin` using `nlohmann_json`.
- **Bootstrap Configuration**: Initializes nodes and connections at startup via a JSON bootstrap file, parsed with `CLI11`.
- **Stdio Integration**: Built-in `stdio` instance for `stdin`/`stdout`/`stderr` communication.
- **Modern C++23**: Leverages `std::expected`, `std::print`, and `scope-guard` for robust design.
- **Cross-Platform**: Runs on Linux, macOS, and Windows via `vcpkg`.

## Prerequisites
- **CMake**: Version 3.27 or higher.
- **C++ Compiler**: Supporting C++23 (e.g., GCC 13, Clang 16, MSVC 2022).
- **Git**: For cloning the repository.
- **vcpkg**: Automatically fetched for dependency management.
- **Libraries**: `nlohmann_json`, `CLI11`, `scope-guard` (installed via `vcpkg`).
- **Operating System**: Linux, macOS, or Windows.

## Installation
1. **Clone the Repository**:
   ```bash
   git clone https://github.com/skunkforce/nadi_node_interconnect.git
   cd nadi_node_interconnect
   ```

2. **Create a Build Directory**:
   ```bash
   mkdir build
   cd build
   ```

3. **Configure with CMake**:
   ```bash
   cmake -S .. -B . -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
   ```

4. **Build the Project**:
   ```bash
   cmake --build . --config Release
   ```

   The executable `nadi_interconnect` will be generated in the `build` directory.

## Usage
1. **Prepare Node Libraries**:
   Place NADI-compliant libraries in the node directory (default: `./nodes` or specified via `--nodes`). Each library must export all NADI functions (`nadi_init`, `nadi_deinit`, `nadi_send`, `nadi_free`, `nadi_descriptor`). Example descriptor:
   ```json
   {"name":"node1","version":"1.0.0"}
   ```

2. **Prepare Bootstrap File** (Optional):
   Create a JSON bootstrap file (default: `bootstrap.json`) to construct, connect, and configure nodes at startup. Example `bootstrap.json` for a WebSocket server:
   ```json
   {
     "messages": [
       {
         "channel": 61440,
         "meta": {"format": "json"},
         "data": {
           "type": "nodes.instances.construct",
           "node_name": "websocket_server",
           "instance": "ws_server"
         }
       },
       {
         "channel": 61440,
         "meta": {"format": "json"},
         "data": {
           "type": "nodes.instances.connections.connect",
           "source": ["ws_server", 2],
           "target": ["interconnect", 61440]
         }
       },
       {
         "channel": 61440,
         "meta": {"format": "json"},
         "data": {"type": "nodes.loaded"}
       }
     ]
   }
   ```

3. **Run the Executable**:
   Run with default settings:
   ```bash
   ./nadi_interconnect
   ```
   Specify a custom node directory and/or bootstrap file:
   ```bash
   ./nadi_interconnect --nodes /path/to/nodes --bootstrap custom_bootstrap.json
   ```
   Parses `--nodes` and `--bootstrap` with `CLI11`, loads libraries from the specified node directory (default: `./nodes`), processes bootstrap messages, loads the `stdio` instance (with output channel `0xF000` connected to the interconnect’s input `0xF000`), and waits for JSON messages on `stdin`.

4. **Construct Nodes**:
   Send a control message to construct a node (e.g., via `stdio`):
   ```bash
   echo '{"channel":61440,"meta":{"format":"json"},"data":{"type":"nodes.instances.construct","node_name":"node1","instance":"node1_instance"}}' | ./nadi_interconnect
   ```
   Constructs the `node1` library as `node1_instance`.

5. **Destruct Nodes**:
   Send a control message to destruct a node instance:
   ```bash
   echo '{"channel":61440,"meta":{"format":"json"},"data":{"type":"nodes.instances.destruct","instance":"node1_instance"}}' | ./nadi_interconnect
   ```
   Destructs the `node1_instance` instance.

6. **List Loaded Node Libraries**:
   Send a control message to list loaded node libraries:
   ```bash
   echo '{"channel":61440,"meta":{"format":"json"},"data":{"type":"nodes.loaded"}}' | ./nadi_interconnect
   ```
   Response (sent to the originating instance, e.g., `stdio` channel 0 for `stdout` or a dynamic node):
   ```json
   {
     "meta": {"format": "json"},
     "data": {
       "type": "nodes.loaded.list",
       "nodes": [
         {"name": "node1", "version": "1.0.0"},
         {"name": "node2", "version": "1.1.0"}
       ]
     }
   }
   ```

7. **List Constructed Node Instances**:
   Send a control message to list constructed node instances:
   ```bash
   echo '{"channel":61440,"meta":{"format":"json"},"data":{"type":"nodes.instances"}}' | ./nadi_interconnect
   ```
   Response (sent to the originating instance):
   ```json
   {
     "meta": {"format": "json"},
     "data": {
       "type": "nodes.instances.list",
       "instances": [
         {"instance": "node1_instance"},
         {"instance": "node1_instance2"},
         {"instance": "node2_instance"}
       ]
     }
   }
   ```

8. **List Active Connections**:
   Send a control message to list active stream connections:
   ```bash
   echo '{"channel":61440,"meta":{"format":"json"},"data":{"type":"nodes.instances.connections"}}' | ./nadi_interconnect
   ```
   Response (sent to the originating instance):
   ```json
   {
     "meta": {"format": "json"},
     "data": {
       "type": "nodes.instances.connections.list",
       "connections": [
         {"source": ["node1_instance", 1], "target": ["node2_instance", 1]},
         {"source": ["ws_server", 2], "target": ["interconnect", 61440]}
       ]
     }
   }
   ```

9. **Configure Stream Connections**:
   Send a control message to connect streams:
   ```bash
   echo '{"channel":61440,"meta":{"format":"json"},"data":{"type":"nodes.instances.connections.connect","source":["node1_instance",1],"target":["node2_instance",1]}}' | ./nadi_interconnect
   ```
   Connects `node1_instance`’s output channel 1 to `node2_instance`’s input channel 1.

10. **Disconnect Streams**:
    Send a control message to disconnect streams:
    ```bash
    echo '{"channel":61440,"meta":{"format":"json"},"data":{"type":"nodes.instances.connections.disconnect","source":["node1_instance",1],"target":["node2_instance",1]}}' | ./nadi_interconnect
    ```
    Disconnects the stream from `node1_instance` channel 1 to `node2_instance` channel 1.

11. **Send Data Messages**:
    Pipe a NADI-compliant data message:
    ```bash
    echo '{"channel":1,"meta":{"format":"json"},"data":{"value":42}}' | ./nadi_interconnect
    ```
    The message is routed to connected nodes, with lifetimes managed for single-consumer streams.

12. **Monitor Output**:
    Check `stdout` (stdio channel 0) for node metadata, connection status, and responses, or `stderr` (stdio channel 1) for errors, if using the `stdio` instance.

### Example Use Cases
- **WebSocket Server**:
  Use a bootstrap file to construct a WebSocket server node, connect its output channel to the interconnect’s `0xF000` channel, and enable clients to list nodes (`nodes.loaded`, `nodes.instances`), construct/destruct nodes (`nodes.instances.construct`, `nodes.instances.destruct`), and manage connections (`nodes.instances.connections.connect`, `nodes.instances.connections.disconnect`).
  ```json
  {
    "messages": [
      {
        "channel": 61440,
        "meta": {"format": "json"},
        "data": {
          "type": "nodes.instances.construct",
          "node_name": "websocket_server",
          "instance": "ws_server"
        }
      },
      {
        "channel": 61440,
        "meta": {"format": "json"},
        "data": {
          "type": "nodes.instances.connections.connect",
          "source": ["ws_server", 2],
          "target": ["interconnect", 61440]
        }
      }
    ]
  }
  ```

- **Device Driver Logging**:
  Use a bootstrap file to construct a device driver node and a file output node, connect their streams, configure logging, and destruct the nodes afterward.
  ```json
  {
    "messages": [
      {
        "channel": 61440,
        "meta": {"format": "json"},
        "data": {
          "type": "nodes.instances.construct",
          "node_name": "device_driver",
          "instance": "driver"
        }
      },
      {
        "channel": 61440,
        "meta": {"format": "json"},
        "data": {
          "type": "nodes.instances.construct",
          "node_name": "file_output",
          "instance": "file"
        }
      },
      {
        "channel": 61440,
        "meta": {"format": "json"},
        "data": {
          "type": "nodes.instances.connections.connect",
          "source": ["driver", 1],
          "target": ["file", 1]
        }
      },
      {
        "channel": 61440,
        "meta": {"format": "json"},
        "data": {
          "type": "nodes.instances.destruct",
          "instance": "driver"
        }
      },
      {
        "channel": 61440,
        "meta": {"format": "json"},
        "data": {
          "type": "nodes.instances.destruct",
          "instance": "file"
        }
      }
    ]
  }
  ```

## Configuration
- **Node Directory**: Place NADI-compliant DLLs/`.so` files in the node directory (default: `./nodes`, or specified via `--nodes`). The `management` class loads these libraries, querying `nadi_descriptor()` for metadata.
- **Bootstrap File**: A JSON file (default: `bootstrap.json`) with a `"messages"` array of control or data messages to construct, destruct, connect, disconnect, and configure nodes at startup, parsed using `CLI11`.
- **Stdio Instance**: Automatically loaded as `"stdio"` with:
  - Output channel `0xF000`: Connected to the interconnect’s input `0xF000` for control messages.
  - Output channel 0: Prints to `stdout`.
  - Output channel 1: Prints to `stderr`.
  - Input channels: Can receive messages from other nodes, addressed via the `"channel"` field.
- **Message Types**:
  - **Data Messages**: For stream data, must include:
    - `channel`: Integer for the communication channel (not `0xF000`).
    - `meta`: JSON object with a `format` field (e.g., `"json"`, `"microseconds-double"`).
    - `data`: Payload, interpreted based on `meta.format`.
    Example:
    ```json
    {
      "channel": 1,
      "meta": {"format": "microseconds-double"},
      "data": [ [1625097600000000, 25.3], [1625097601000000, 25.5] ]
    }
    ```
  - **Control Messages**: For node construction, destruction, connection management, or node queries (on channel `0xF000`), must include:
    - `channel`: `61440` (0xF000 in decimal).
    - `meta`: JSON object with `"format":"json"`.
    - `data`: JSON object with control details.
    - **Node Construction**:
      - `type`: `"nodes.instances.construct"`.
      - `node_name`: String specifying the node library’s name (from `nadi_descriptor`).
      - `instance`: String defining a unique identifier for the instance, used in connections.
      Example:
      ```json
      {
        "channel": 61440,
        "meta": {"format": "json"},
        "data": {
          "type": "nodes.instances.construct",
          "node_name": "node1",
          "instance": "node1_instance"
        }
      }
      ```
    - **Node Destruction**:
      - `type`: `"nodes.instances.destruct"`.
      - `instance`: String identifying the instance to destruct.
      Example:
      ```json
      {
        "channel": 61440,
        "meta": {"format": "json"},
        "data": {
          "type": "nodes.instances.destruct",
          "instance": "node1_instance"
        }
      }
      ```
    - **Node Connection**:
      - `type`: `"nodes.instances.connections.connect"`.
      - `source`: Tuple `[instance, channel]`, where `instance` is the instance identifier (string) and `channel` is a number.
      - `target`: Tuple `[instance, channel]`, specifying the target instance identifier and channel.
      Example:
      ```json
      {
        "channel": 61440,
        "meta": {"format": "json"},
        "data": {
          "type": "nodes.instances.connections.connect",
          "source": ["node1_instance", 1],
          "target": ["node2_instance", 1]
        }
      }
      ```
    - **Node Disconnection**:
      - `type`: `"nodes.instances.connections.disconnect"`.
      - `source`: Tuple `[instance, channel]`, identifying the source instance and channel.
      - `target`: Tuple `[instance, channel]`, identifying the target instance and channel.
      Example:
      ```json
      {
        "channel": 61440,
        "meta": {"format": "json"},
        "data": {
          "type": "nodes.instances.connections.disconnect",
          "source": ["node1_instance", 1],
          "target": ["node2_instance", 1]
        }
      }
      ```
    - **Node Library Listing**:
      - `type`: `"nodes.loaded"`.
      Response (sent to the originating instance):
      ```json
      {
        "meta": {"format": "json"},
        "data": {
          "type": "nodes.loaded.list",
          "nodes": [
            {"name": "node1", "version": "1.0.0"},
            {"name": "node2", "version": "1.1.0"}
          ]
        }
      }
      ```
    - **Node Instance Listing**:
      - `type`: `"nodes.instances"`.
      Response (sent to the originating instance):
      ```json
      {
        "meta": {"format": "json"},
        "data": {
          "type": "nodes.instances.list",
          "instances": [
            {"instance": "node1_instance"},
            {"instance": "node1_instance2"},
            {"instance": "node2_instance"}
          ]
        }
      }
      ```
    - **Connection Listing**:
      - `type`: `"nodes.instances.connections"`.
      Response (sent to the originating instance):
      ```json
      {
        "meta": {"format": "json"},
        "data": {
          "type": "nodes.instances.connections.list",
          "connections": [
            {"source": ["node1_instance", 1], "target": ["node2_instance", 1]},
            {"source": ["ws_server", 2], "target": ["interconnect", 61440]}
          ]
        }
      }
      ```

## API
The NADI interface is defined in `include/nadi.h`. Key components include:

### Struct `nadi_message`
```c
struct nadi_message {
    char* meta;              // Null-terminated JSON string
    unsigned long meta_hash; // Hash of meta content (0 if unused)
    char* data;              // Raw binary data
    unsigned int data_length;// Length of data in bytes
    nadi_free_callback free; // Callback to free the message
    nadi_instance_handle instance; // Connection instance handle
    unsigned int channel;    // Channel identifier
};
```

### Functions
- `nadi_init(nadi_instance_handle* instance, nadi_receive_callback callback)`: Initializes a NADI instance with a receive callback.
- `nadi_deinit(nadi_instance_handle instance)`: Deinitializes an instance, blocking until threads complete.
- `nadi_send(nadi_message* message, nadi_instance_handle instance)`: Sends a message, transferring ownership on success.
- `nadi_free(nadi_message* message)`: Frees a message using its `free` callback.
- `nadi_descriptor()`: Returns a JSON string with node metadata (e.g., `{"name":"node","version":"1.0.0"}`).

### Struct `nadi_library` (in `node.hpp`)
```cpp
struct nadi_library {
    nadi_init_pt init;        // Pointer to nadi_init
    nadi_deinit_pt deinit;    // Pointer to nadi_deinit
    nadi_send_pt send;        // Pointer to nadi_send
    nadi_free_pt free;        // Pointer to nadi_free
    nadi_descriptor_pt descriptor; // Pointer to nadi_descriptor
    // Platform-specific library handle (HMODULE on Windows, void* on Linux/macOS)
};
```

### Function `load_node` (in `node.hpp`)
- `nadi_library load_node(std::string path)`: Loads a NADI-compliant library, mapping its exported functions to `nadi_library` pointers.

### Class `management` (in `management.hpp`)
- `load_nodes(const std::string& dir)`: Loads NADI-compliant libraries from the specified directory using `load_node`.
- `to_json()`: Returns JSON representation of loaded nodes.
- `callback(nadi_message* msg)`: Processes messages:
  - Routes data messages (non-`0xF000` channels) to connected nodes.
  - Parses control messages (`0xF000` channel) for node construction (`nodes.instances.construct`), destruction (`nodes.instances.destruct`), connection management (`nodes.instances.connections.connect`, `nodes.instances.connections.disconnect`, `nodes.instances.connections`), node library listing (`nodes.loaded`), node instance listing (`nodes.instances`), or bootstrap initialization.
  - Manages message lifetimes with `scope-guard` (single-consumer); multi-consumer support planned.

### Class `node_management` (in `node_management.hpp`)
- `load_nodes(const std::string& path)`: Loads libraries from the specified directory using `find_nodes::get_node_paths` and `load_node`.
- `construct_node`: Constructs a node instance from a library, initializing it with `nadi_init`.
- `destruct_node`: Destructs an instance, deinitializing it with `nadi_deinit`.
- `send_loaded_list`: Sends a `nodes.loaded.list` response with library metadata.
- `send_instances_list`: Sends a `nodes.instances.list` response with instance identifiers.
- `lib_from_instance`: Retrieves the `nadi_library` for an instance handle.

### Class `message_routing` (in `message_routing.hpp`)
- `connect`: Connects a source instance and channel to a target instance and channel (implementation incomplete).
- `disconnect`: Disconnects a source-target stream (implementation incomplete).
- `send_connections_list`: Sends a `nodes.instances.connections.list` response with active connections (implementation incomplete).
- `destinations_from`: Returns destinations for a given source instance and channel.

*(Note: Full details for connection management require complete implementations in `message_routing.hpp`.)*

## Building for Development
To build with debug symbols:
```bash
cmake -S .. -B . -CMAKE_BUILD_TYPE=Debug -CMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Debug
```

## Contributing
Contributions are welcome! Follow these steps:
1. Fork the repository.
2. Create a feature branch (`git checkout -b feature/your-feature`).
3. Commit changes (`git commit -m 'Add your feature'`).
4. Push to the branch (`git push origin feature/your-feature`).
5. Open a Pull Request.

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## License
Licensed under the MIT License. See [LICENSE](LICENSE) for details.