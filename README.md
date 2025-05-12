
--Made using asio boost (asynchronous io)

# High-Concurrency TCP Server

This project implements a high-concurrency TCP server designed to handle multiple client connections efficiently. It is built with scalability and performance in mind, making it suitable for applications requiring high throughput and low latency.


## Features

- **High Concurrency**: Supports thousands of simultaneous client connections.
- **Efficient Networking**: Uses non-blocking I/O and event-driven architecture for optimal performance.
- **Logging**: Detailed logs of client activity are stored for monitoring and debugging.
- **Acknowledgment System**: Each client request is acknowledged with a unique response.
- **Scalability**: Designed to scale horizontally for increased load handling.

---

## Logs Overview

The server logs client activity in a CSV file (`logs/client.csv`). Each row in the file represents a client interaction with the following fields:

1. **Client ID**: A unique identifier for the client.
2. **Timestamp**: The time of the interaction in nanoseconds.
3. **Request Size**: The size of the client's request in bytes.
4. **Response Size**: The size of the server's response in bytes.
5. **Acknowledgment Message**: A custom acknowledgment message sent to the client.

### Example Log Entries

Below are some sample entries from the `logs/client.csv` file:

```csv
client-2,1746708928369557955,4050687652553,79381,ACK:client-2
client-4,1746708928369710907,4050687832527,106328,ACK:client-4
client-994,1746708928915163930,4051233235186,56132,ACK:client-994
 

to run server do (in the build directory)
./server 8080 
You can now run the client like:
./client client-123
Or simulate 1000 clients:
for i in {1..1000}; do ./client client-$i & done; wait


Convert `client.log` to CSV

a minimal shell command

### 🟢 Terminal CSV conversion: (do this in root directory)

awk '
/Message ID:/ { id = $3 }
/Sent Timestamp/ { sent = $4 }
/ACK Received/ { ack = $3 }
/Received Timestamp/ { recv = $4 }
/RTT \(ns\)/ { rtt = $3 }
/---/ {
  print id "," sent "," recv "," rtt "," ack
}
' logs/client.log > logs/client.csv



#without epoll 