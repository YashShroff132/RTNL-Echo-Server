#!/bin/bash

TOTAL_CLIENTS=1000
CLIENT_BINARY=build/client
DELAY_MS=0.005
LOG_FILE=logs/simulation_summary.log
CLIENT_LOG=logs/client.log

# Ensure logs folder exists
mkdir -p logs

# Clean and re-create log files
> $CLIENT_LOG
> $LOG_FILE
rm -f logs/client.csv

echo "Starting simulation with $TOTAL_CLIENTS clients..." | tee -a $LOG_FILE

for i in $(seq 1 $TOTAL_CLIENTS)
do
    CLIENT_ID="client-$i"
    $CLIENT_BINARY "$CLIENT_ID" >> $CLIENT_LOG 2>&1 &

    echo "Launched $CLIENT_ID" >> $LOG_FILE
    sleep $DELAY_MS
done

wait
echo "All clients finished." | tee -a $LOG_FILE

# Convert client.log to CSV
awk -F'[:,()]+' '
/Received ACK:/ {
    # Clean up and extract parts
    match($0, /ACK:client-[0-9]+/, m)
    id = m[0]
    rtt = $(NF)
    gsub(/ /, "", id)
    print id ",,,," rtt
}
' logs/client.log > logs/client.csv


# Calculate stats
TOTAL_ACKS=$(wc -l < logs/client.csv)
AVG_RTT=$(awk -F',' '{sum+=$5} END {if (NR > 0) print int(sum/NR)}' logs/client.csv)
MIN_RTT=$(awk -F',' 'NR == 1 {min=$5} $5 < min {min=$5} END {print min}' logs/client.csv)
MAX_RTT=$(awk -F',' '$5 > max {max=$5} END {print max}' logs/client.csv)

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
