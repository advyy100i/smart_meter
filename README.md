# Smart Energy Meter Telemetry System

A real-time IoT telemetry platform for a **cost-effective 3-phase smart energy meter**, built using embedded firmware, custom binary communication protocols, Apache Kafka, and a Flutter dashboard. The system continuously measures electrical parameters, streams telemetry with low overhead, and provides live monitoring with persistent storage.

---

# Features

* Real-time monitoring of:

  * Voltage (RMS)
  * Current (RMS)
  * Power
  * Energy
* Custom binary UART communication between ATmega328P and ESP8266
* Custom 20-byte binary TCP protocol for efficient telemetry transmission
* Apache Kafka–based event-driven streaming pipeline
* Live Flutter dashboard via WebSockets
* MongoDB persistence for historical analytics
* NTP-based timestamp synchronization
* Modular and scalable architecture

---

# Architecture
![backend](https://github.com/user-attachments/assets/07ee0de9-7451-4c88-9ac0-9e1d2e80bc98)
```text
                +-------------------------+
                |     Smart Energy Meter  |
                |      (ATmega328P)       |
                +-------------------------+
                           |
                           | Binary UART
                           |
                           ▼
                +-------------------------+
                |        ESP8266          |
                | Wi-Fi + TCP Client      |
                +-------------------------+
                           |
                 Custom Binary TCP
                           |
                           ▼
                +-------------------------+
                |    Node.js TCP Server   |
                |   Binary Packet Parser  |
                +-------------------------+
                           |
                           ▼
                    Apache Kafka
                  meter-readings topic
               ┌────────────┴────────────┐
               ▼                         ▼
        MongoDB Consumer         WebSocket Consumer
               │                         │
               ▼                         ▼
      Historical Storage        Flutter Dashboard
```

---

# Technology Stack

## Embedded

* ATmega328P
* ESP8266
* Arduino C++
* SoftwareSerial
* SerialTransfer

## Backend

* Node.js
* Apache Kafka
* KafkaJS
* TCP Sockets

## Database

* MongoDB

## Frontend

* Flutter
* WebSockets

---

# Telemetry Flow

1. ATmega328P samples voltage and current.
2. RMS voltage, RMS current, power, and cumulative energy are calculated.
3. Measurements are packaged into a compact binary packet.
4. Packet is transmitted to ESP8266 over UART.
5. ESP8266 appends timestamp and meter ID.
6. ESP8266 sends a 20-byte binary packet over TCP.
7. Node.js parses the packet and publishes telemetry to Kafka.
8. Kafka streams data to:

   * MongoDB consumer (storage)
   * WebSocket consumer (live dashboard)

---

# Custom Binary TCP Protocol

Each telemetry packet is **20 bytes**.

| Offset | Size    | Field     |
| ------ | ------- | --------- |
| 0      | 4 bytes | Timestamp |
| 4      | 4 bytes | Meter ID  |
| 8      | 2 bytes | Voltage   |
| 10     | 2 bytes | Current   |
| 12     | 4 bytes | Power     |
| 16     | 4 bytes | Energy    |

Compared to an equivalent JSON message, the binary protocol significantly reduces payload size, making it suitable for resource-constrained IoT devices and real-time telemetry.

---

# Project Highlights

* Developed firmware for a **3-phase smart energy meter**
* Implemented **custom UART and TCP binary protocols**
* Built a **Kafka-based event-driven telemetry pipeline**
* Enabled **real-time Flutter dashboards**
* Designed for **low-overhead telemetry transmission**
* Persistent storage using MongoDB
* Timestamp synchronization using NTP

---

# Repository Structure

```text
.
├── firmware/
│   ├── atmega328p/
│   └── esp8266/
│
├── backend/
│   ├── tcp-server/
│   ├── kafka-consumer-websocket/
│   └── kafka-consumer-mongodb/
│
├── flutter_app/
│
└── README.md
```

---

# Future Improvements

* TLS-secured TCP communication
* Automatic device registration
* OTA firmware updates
* Packet checksum and CRC validation
* MQTT support
* Grafana dashboards
* Alerting for abnormal power consumption
* Multi-meter fleet management

---

# Key Concepts Demonstrated

* Embedded Systems
* IoT
* Binary Protocol Design
* TCP Networking
* Event-Driven Architecture
* Apache Kafka
* Real-Time Streaming
* WebSockets
* Flutter
* MongoDB
* Distributed Systems

This README clearly communicates both **what the project does** and **why its architecture is technically interesting**, making it suitable for GitHub and for recruiters reviewing your portfolio.



