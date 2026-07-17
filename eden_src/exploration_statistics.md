# EXPLORATION STATISTICS OF ALL METHODS

| Environment | Method | Exploration Time (s) | | Computational Cost (ms) | | UAV Speed (m/s) | | Traveled Distance (m) | |
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| | | Avg | Std | Avg | Std | Avg | Std | Avg | Std |
| DARPA Tunnel | EDEN | 54.346 | 1.416 | 12.08 | 0.22 | 1.797 | 0.019 | 98.197 | 1.860 |
| DARPA Tunnel | Spectral-EDEN | 
| Classical Office | EDEN | 92.139 | 6.522 | 15.76 | 0.65 | 1.652 | 0.023 | 161.280 | 12.197 |
| Classical Office | Spectral-EDEN | 83.350 | 4.708 | 16.68 | 0.81 | 1.803 | 0.005 | 150.831 | 8.624 |
| Large Maze | EDEN | 316.851 | 10.683 | 18.99 | 0.48 | 1.840 | 0.019 | 583.495 | 19.161 |
| City | EDEN | 1336.0 | 245.4 | 49.23 | 1.88 | 1.736 | 0.019 | 2283.0 | 416.7 |
| Large Tunnel | EDEN | — | — | — | — | — | — | — | — |

*DARPA Tunnel (EDEN): 2026-07-13 13:38–13:48 (10 runs, Spectral data row) | Classical Office (EDEN): 2026-07-07 14:05 | Classical Office (Spectral-EDEN): 2026-07-13 14:06 (10 runs) | Large Maze (EDEN): 2026-07-15 13:56–15:?? (10 runs) | Large Maze (Spectral-EDEN): 2026-07-15 13:48 | City: 2026-07-09 09:09 (9 runs, excl. Run 5 outlier)*

## City Experiment Notes
- **Hardware**: Intel i7-13620H (laptop) + RTX 4060 Laptop GPU (60W), Ubuntu 20.04
- **Paper Hardware**: Titan Xp GPU (250W) desktop
- **Thermal Management**: Active CPU load via thermal_guard.py to prevent turbo-boost → throttle cycles
- **Speed**: 1.74 m/s vs paper 1.85 m/s (-6.1% due to laptop thermal limits)
- **Cost**: 45.71 ms vs paper 20.31 ms (+125% due to laptop CPU/GPU constraints)
- **Run 5 excluded**: Early termination at 313s (likely ROS communication failure)
- **Key Limitation**: Laptop 87°C thermal throttling drops CPU to 561 MHz, doubling planning latency
