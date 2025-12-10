#📶 Reliable Data Transfer Protocols

**Alternating Bit (ABT) • Go-Back-N (GBN) • Selective Repeat (SR)**

This project implements three fundamental **reliable data transfer protocols** used in transport-layer networking. Each protocol is tested inside a controlled packet-level simulator that models delay, corruption, loss, and reordering.

The goal is to demonstrate how different ARQ (Automatic Repeat reQuest) strategies behave under unreliable network conditions.

---

## 🚀 Overview

### **1️⃣ Alternating Bit Protocol (ABT)**
A simple stop-and-wait mechanism using a 1-bit sequence number.  
Features:
- One packet in flight  
- Timer-based retransmission  
- Duplicate-packet handling  

---

### **2️⃣ Go-Back-N Protocol (GBN)**
A pipelined ARQ protocol that allows multiple outstanding packets.  
Features:
- Sliding window  
- Cumulative ACKs  
- Retransmits entire window on timeout  

---

### **3️⃣ Selective Repeat Protocol (SR)**
A more advanced protocol using selective acknowledgments.  
Features:
- Independent per-packet timers  
- Out-of-order buffering at the receiver  
- Only retransmits lost/corrupted packets  

---

## 🧪 Simulation Environment

All three protocols run inside a custom event-driven simulator that mimics an unreliable network.

The simulator provides:
- Random transmission delay  
- Random packet loss  
- Bit-level corruption  
- Packet reordering  
- Timer-driven events  
- APIs for delivering messages from A → B and B → A  

This creates a realistic environment for evaluating protocol behavior.

---

## 📂 Project Structure
```
RELIABLE PROTOCOLS/
├── include/
│   └── simulator.h          # Shared definitions, structs, and simulator API
│
├── src/
│   ├── abt.c                # Alternating Bit Protocol implementation
│   ├── gbn.c                # Go-Back-N implementation
│   ├── sr.c                 # Selective Repeat implementation
│   └── simulator.c          # Event-driven network simulator
│
├── Makefile                 # Builds ABT, GBN, and SR executables
└── README.md                # Documentation

```

The Makefile produces the following executables:  
`abt`, `gbn`, `sr`

---

## 🎯 Key Concepts Demonstrated

### **✔ Stop-and-Wait vs Pipelining**
- ABT transmits a single packet at a time.
- GBN and SR both support multiple outstanding packets.

### **✔ Window Management**
- GBN uses cumulative acknowledgments.
- SR buffers out-of-order packets and ACKs individually.

### **✔ Timer Strategies**
- ABT + GBN use a single sender timer.
- SR manages timers per packet.

### **✔ Handling Loss, Corruption, Delay**
The simulator introduces random noise (loss, corruption, delay), demonstrating how protocols recover from unreliable conditions.

---

## 📘 Files Explained

| File | Description |
|------|-------------|
| `abt.c` | Alternating Bit sender/receiver logic |
| `gbn.c` | Go-Back-N sender/receiver logic |
| `sr.c` | Selective Repeat implementation |
| `simulator.c` | Event scheduler, network layer, corruption/loss model |
| `simulator.h` | Structs, message format, event definitions |
| `Makefile` | Build instructions |

---

## 📌 Summary

This project provides a full comparison of three classical reliable data transfer protocols under simulated network conditions.  
It highlights the efficiency, retransmission behavior, and robustness of **ABT**, **GBN**, and **SR**.


