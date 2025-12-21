# Hierarchic Stream Graph (HSG)

**HSG** is a minimal, language- and transport-agnostic abstraction for building hierarchical, directed stream graphs.

It has been distilled from decades of real-world industrial and scientific systems (CANopen, IO-Link, ASI-bus, Modbus, MQTT, OPC UA, EPICS, ROS, audio graphs, …) that all converged on the same core pattern.

### Core Concepts

| Concept       | Meaning                                                                                 | Real-world examples                                   |
|---------------|------------------------------------------------------------------------------------------|--------------------------------------------------------|
| **Node**      | Entity that produces and/or consumes data                                               | PLC, sensor, drive, robot, GUI, camera, IOC, MQTT client |
| **Handle**    | Opaque identifier for a node, valid only within its containing **context**             | Slave address, port number, IOC name, client ID        |
| **Channel**   | Typed port on a node (input or output)                                                  | PV name, topic, Modbus function code, IO-Link ISDU     |
| **Packet**    | Immutable data unit: source handle + source channel + optional format + payload       | CAN frame, MQTT message, CA monitor, OPC UA DataSet    |
| **Stream**    | Directed, ordered flow of packets from one channel to one or more channels             | CA monitor subscription, MQTT topic subscription       |
| **Context**   | Container of nodes with its own handle space and dispatch rules                         | CAN bus, ASI master, MQTT broker, OPC UA server, EPICS IOC |
| **Sub-context**| A node that is also a context (nesting)                                                | IO-Link hub, ASI repeater, soft IOC inside another IOC |

### Key Rules

1. **Handles are scoped** – a handle is only meaningful inside its context.
2. **Boundary channels** – when a packet enters a sub-context from its parent, the source handle becomes the **context node** and the source channel becomes one of the context’s input channels.
3. **No identity leakage** – nodes inside a context never see concrete handles from outside their context.
4. **Consistency guarantee** – every context declares its stream ordering guarantee:
   - **Total order** (e.g. CANopen, ASI-bus, IO-Link master)
   - **Partial order** (per-node ordering only)
   - **No order** (e.g. pure UDP pub-sub)

### Real-world systems that are pure HSG

| System      | Context                  | Node                     | Channel                     | Order guarantee |
|-------------|--------------------------|--------------------------|-----------------------------|-----------------|
| CANopen     | CAN bus                  | ECU                      | COB-ID                      | Total           |
| IO-Link     | Master                   | Device (port)            | Cyclic PD + ISDU            | Total           |
| ASI-bus     | Master                   | Slave (address)          | Cyclic I/O + parameters     | Total           |
| Modbus TCP  | Master / gateway         | Slave                    | Function code               | Partial         |
| MQTT        | Broker                   | Client                   | Topic                       | QoS-dependent   |
| OPC UA      | Server / namespace       | Node (Object/Variable)   | Reference / subscription    | Configurable    |
| EPICS       | IOC                      | Record / client          | Process Variable (PV)       | Soft real-time  |
| ROS 1       | ROS core                 | Node                     | Topic                       | Best-effort     |

All of them are just different concrete implementations of the same HSG abstraction.

### Implementations

- **NADI** – reference C11 implementation of HSG (this repository)
- Future: JavaScript/WebSocket, Rust, Go, embedded bare-metal, …

HSG itself contains **no code, no data types, no reserved channels** — only the universal pattern.

See `README_NADI.md` for the concrete C implementation.