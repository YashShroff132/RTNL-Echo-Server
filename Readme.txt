in build 

to run server do 
run ./server 8080 
You can now run the client like:
./client client-123
Or simulate 100 clients:
for i in {1..100}; do ./client client-$i & done; wait


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



