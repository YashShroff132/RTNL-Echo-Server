#!/bin/bash

TOTAL_CLIENTS=1000   # Cap to 1000 due to ulimit
CLIENT_BINARY=build/client
DELAY_MS=5
LOG_FILE=simulation_summary.log
CLIENT_LOG=logs/client.log

# Clean up old logs
rm -f $CLIENT_LOG $LOG_FILE logs/client.csv

echo "Starting simulation with $TOTAL_CLIENTS clients..." | tee -a $LOG_FILE

for i in $(seq 1 $TOTAL_CLIENTS)
do
    CLIENT_ID="client-$i"
    $CLIENT_BINARY "$CLIENT_ID" &

    echo "Launched $CLIENT_ID" >> $LOG_FILE

    sleep 0.005  # 5ms
done

wait
echo "All clients finished." | tee -a $LOG_FILE

# Convert client.log to CSV
awk '
/Message ID:/ { id = $3 }
/Sent Timestamp/ { sent = $4 }
/ACK Received/ { ack = $3 }
/Received Timestamp/ { recv = $4 }
/RTT \(ns\)/ { rtt = $3 }
/---/ {
  print id "," sent "," recv "," rtt "," ack
}
' $CLIENT_LOG > logs/client.csv

# Calculate stats
TOTAL_ACKS=$(wc -l < logs/client.csv)
AVG_RTT=$(awk -F',' '{sum+=$4} END {if (NR > 0) print int(sum/NR)}' logs/client.csv)
MIN_RTT=$(awk -F',' 'NR == 1 {min=$4} $4 < min {min=$4} END {print min}' logs/client.csv)
MAX_RTT=$(awk -F',' '$4 > max {max=$4} END {print max}' logs/client.csv)

# Log summary
{
echo "-----------------------------------"
echo "Client Simulation Summary"
echo "Clients launched: $TOTAL_CLIENTS"
echo "Clients received ACK: $TOTAL_ACKS"
echo "Average RTT (ns): $AVG_RTT"
echo "Min RTT (ns): $MIN_RTT"
echo "Max RTT (ns): $MAX_RTT"
echo "CSV log: logs/client.csv"
echo "Raw log: $CLIENT_LOG"
echo "-----------------------------------"
} | tee -a $LOG_FILE
