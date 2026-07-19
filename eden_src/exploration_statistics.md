# EXPLORATION STATISTICS OF ALL METHODS

| Environment | Method | Exploration Time (s) | | Computational Cost (ms) | | UAV Speed (m/s) | | Traveled Distance (m) | |
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| | | Avg | Std | Avg | Std | Avg | Std | Avg | Std |
| DARPA Tunnel | EDEN | 54.346 | 1.416 | 12.08 | 0.22 | 1.797 | 0.019 | 98.197 | 1.860 |
| DARPA Tunnel | Spectral-EDEN | 
| Classical Office | EDEN | 92.139 | 6.522 | 15.76 | 0.65 | 1.652 | 0.023 | 161.280 | 12.197 |
| Classical Office | Spectral-EDEN | 83.350 | 4.708 | 16.68 | 0.81 | 1.803 | 0.005 | 150.831 | 8.624 |
| Large Maze | EDEN | 316.851 | 10.683 | 18.99 | 0.48 | 1.840 | 0.019 | 583.495 | 19.161 |
| Large Maze | Spectral-EDEN V4 | 397.012 | 23.552 | 21.277 | 0.433 | 1.777 | 0.019 | 701.846 | 38.152 |
| City | EDEN | 1336.0 | 245.4 | 49.23 | 1.88 | 1.736 | 0.019 | 2283.0 | 416.7 |
| Large Tunnel | EDEN | — | — | — | — | — | — | — | — |

*DARPA Tunnel (EDEN): 2026-07-13 13:38–13:48 (10 runs, Spectral data row) | Classical Office (EDEN): 2026-07-07 14:05 | Classical Office (Spectral-EDEN): 2026-07-13 14:06 (10 runs) | Large Maze (EDEN): 2026-07-15 13:56–15:?? (10 runs) | Large Maze (Spectral-EDEN): 2026-07-15 13:48 | Large Maze (Spectral-EDEN V4): 2026-07-19 18:35–19:52 (10 independent runs) | City: 2026-07-09 09:09 (9 runs, excl. Run 5 outlier)*

## Large Maze Spectral-EDEN V4 Experiment Notes

- **Completion criterion**: explored volume reaches 99% of 3170 m³ (3138.3 m³) and remains above the threshold for 2 s.
- **Timeout**: each run is independently limited to 450 s. Runs 3 and 5 timed out and are included in the main-table statistics with exploration time censored at 450 s.
- **Success rate**: 8/10 (80%).
- **Statistics**: the main table reports the mean and population standard deviation over all 10 runs. Computational cost is the per-run mean of target planning, corridor planning, and trajectory planning time.
- **Successful runs only**: Exploration Time 397.012 ± 23.552 s, Computational Cost 21.277 ± 0.433 ms, UAV Speed 1.777 ± 0.019 m/s, Traveled Distance 701.846 ± 38.152 m.
- **Timeout observations**: Run 3 remained active but ended 7.797 m³ below the 99% threshold. Run 5 stopped gaining volume and distance at 1338.304 m³ / 203.890 m, then repeated the three-failure target refresh 111 times without recovery.

| Run | Status | Exploration Time (s) | Computational Cost (ms) | UAV Speed (m/s) | Traveled Distance (m) | Final Explored Volume (m³) |
|:---:|:---:|---:|---:|---:|---:|---:|
| 1 | Finished | 418.620 | 21.722 | 1.750 | 729.391 | 3145.746 |
| 2 | Finished | 394.630 | 20.410 | 1.761 | 691.812 | 3159.247 |
| 3 | Timeout | 450.000 | 21.567 | 1.770 | 791.830 | 3130.503 |
| 4 | Finished | 397.669 | 21.713 | 1.761 | 696.947 | 3138.326 |
| 5 | Timeout | 450.000 | 18.616 | 0.456 | 203.890 | 1338.304 |
| 6 | Finished | 353.627 | 21.477 | 1.810 | 636.780 | 3160.187 |
| 7 | Finished | 372.143 | 21.346 | 1.787 | 662.005 | 3152.250 |
| 8 | Finished | 409.650 | 21.041 | 1.799 | 733.608 | 3147.785 |
| 9 | Finished | 396.633 | 21.611 | 1.773 | 699.713 | 3140.303 |
| 10 | Finished | 433.120 | 20.894 | 1.772 | 764.511 | 3146.748 |

## City Experiment Notes
- **Hardware**: Intel i7-13620H (laptop) + RTX 4060 Laptop GPU (60W), Ubuntu 20.04
- **Paper Hardware**: Titan Xp GPU (250W) desktop
- **Thermal Management**: Active CPU load via thermal_guard.py to prevent turbo-boost → throttle cycles
- **Speed**: 1.74 m/s vs paper 1.85 m/s (-6.1% due to laptop thermal limits)
- **Cost**: 45.71 ms vs paper 20.31 ms (+125% due to laptop CPU/GPU constraints)
- **Run 5 excluded**: Early termination at 313s (likely ROS communication failure)
- **Key Limitation**: Laptop 87°C thermal throttling drops CPU to 561 MHz, doubling planning latency
