#!/bin/bash
# Run V4 Maze experiment 10 times
# Usage: bash run_maze_v4.sh

set -e

ROS_SETUP="source /opt/ros/noetic/setup.bash && source /home/island/EDEN/catkin_ws/devel/setup.bash"
LAUNCH="roslaunch eden single_exp_demo_maze3_lite_low.launch"
RESULT_DIR="/home/island/EDEN/data/maze3/v4_runs"
TIMEOUT=300
LAUNCH_TIMEOUT=15  # Give launch 15s to start, then rely on process monitoring

mkdir -p "$RESULT_DIR"

echo "============================================"
echo "V4 Maze Experiment - 10 runs"
echo "Start time: $(date)"
echo "============================================"

for run in $(seq 1 10); do
    echo ""
    echo "===== Run $run/10 ====="
    LOGFILE="$RESULT_DIR/run_${run}_$(date +%Y%m%d_%H%M%S).log"

    # Kill any lingering ROS processes
    pkill -f "roslaunch\|rosmaster\|rosout\|eden_node\|rviz\|map_render\|poscmd_2_odom\|traj_exc\|map_pcl" 2>/dev/null || true
    sleep 3

    # Additional clean
    pkill -9 -f "ros\|eden" 2>/dev/null || true
    sleep 1

    # Launch experiment
    echo "Launching run $run..."
    bash -c "$ROS_SETUP && timeout $TIMEOUT $LAUNCH" > "$LOGFILE" 2>&1 &
    LAUNCH_PID=$!

    # Wait for launch to start (look for "Spectral-EDEN V4" message)
    for i in $(seq 1 30); do
        if grep -q "Spectral-EDEN V4" "$LOGFILE" 2>/dev/null; then
            echo "  V4 started successfully after ${i}s"
            break
        fi
        if ! kill -0 $LAUNCH_PID 2>/dev/null; then
            echo "  ERROR: Launch process died during startup!"
            break
        fi
        sleep 2
    done

    # Wait for completion
    echo "  Waiting for exploration to complete..."
    wait $LAUNCH_PID 2>/dev/null || true
    EXIT_CODE=$?

    # Kill any remaining processes
    pkill -f "roslaunch\|rosmaster\|rosout\|eden_node\|rviz\|map_render\|poscmd_2_odom\|traj_exc\|map_pcl" 2>/dev/null || true
    sleep 2

    # Extract key results
    if grep -q "Exploration complete\|volume coverage reached" "$LOGFILE" 2>/dev/null; then
        echo "  ✓ Exploration completed normally"
    elif grep -q "MAP_FINISHED\|NO_ACTIVE_FRONTIER" "$LOGFILE" 2>/dev/null; then
        echo "  ✓ Exploration finished (no frontiers)"
    elif [ $EXIT_CODE -eq 124 ] || [ $EXIT_CODE -eq 143 ]; then
        echo "  ⚠ Run timed out after ${TIMEOUT}s"
    else
        echo "  ⚠ Run ended with code $EXIT_CODE"
    fi

    # Extract targets explored
    TARGETS=$(grep -c "target explored" "$LOGFILE" 2>/dev/null || echo 0)
    echo "  Targets explored: $TARGETS"

    # Extract average planning time
    AVG_PLAN=$(grep "t total2:" "$LOGFILE" 2>/dev/null | awk -F: '{sum+=$NF; n++} END{if(n>0) printf "%.2f", sum/n; else print "N/A"}')
    echo "  Avg plan time (t total2): ${AVG_PLAN}ms"

    # Check V4 activation
    if grep -q "Spectral-EDEN V4 enabled=1" "$LOGFILE" 2>/dev/null; then
        echo "  V4: ACTIVATED"
    else
        echo "  V4: NOT ACTIVATED!"
    fi

    # Check for crashes
    CRASHES=$(grep -ci "SIGSEGV\|segfault\|assert\|abort" "$LOGFILE" 2>/dev/null || echo 0)
    if [ "$CRASHES" -gt 0 ]; then
        echo "  ⚠ CRASH DETECTED ($CRASHES occurrences)!"
    fi

    echo "  Log: $LOGFILE"

    # Brief pause between runs
    sleep 2
done

echo ""
echo "============================================"
echo "All 10 runs completed at $(date)"
echo "Results in: $RESULT_DIR"
echo "============================================"

# Summary
echo ""
echo "===== SUMMARY ====="
for run in $(seq 1 10); do
    LOG=$(ls -t "$RESULT_DIR"/run_${run}_*.log 2>/dev/null | head -1)
    if [ -n "$LOG" ]; then
        TARGETS=$(grep -c "target explored" "$LOG" 2>/dev/null || echo 0)
        AVG_PLAN=$(grep "t total2:" "$LOG" 2>/dev/null | awk -F: '{sum+=$NF; n++} END{if(n>0) printf "%.2f", sum/n; else print "N/A"}')
        V4_OK=$(grep -c "Spectral-EDEN V4 enabled=1" "$LOG" 2>/dev/null || echo 0)
        VOL_DONE=$(grep -c "volume coverage reached\|Exploration complete" "$LOG" 2>/dev/null || echo 0)
        echo "Run $run: targets=$TARGETS avg_plan=${AVG_PLAN}ms v4=$V4_OK vol_done=$VOL_DONE"
    else
        echo "Run $run: NO LOG FOUND"
    fi
done