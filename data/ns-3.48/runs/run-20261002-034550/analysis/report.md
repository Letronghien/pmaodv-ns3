# Analysis of `all-runs.csv`

## Runs per scenario and variant

| scenario    |   AODV |   AOMDV(mp=3) |   DSDV |   PMAODV(mp=2) |   PMAODV(mp=3) |   PMAODV(mp=4) |
|:------------|-------:|--------------:|-------:|---------------:|---------------:|---------------:|
| default     |    320 |           320 |    320 |            320 |            320 |            320 |
| density     |     30 |            30 |     30 |             30 |             30 |             30 |
| multi-flow  |    300 |           300 |    300 |            300 |            300 |            300 |
| stress-all  |    300 |           300 |    300 |            300 |            300 |            300 |
| stress-load |    320 |           320 |    320 |            320 |            320 |            320 |
| stress-mob  |    320 |           320 |    320 |            320 |            320 |            320 |

## Table 2 - means

| scenario    | variant      |   deliveryRatio |   throughputMbps |   avgDelayMs |   routingOverhead |   controlPackets |   totalEnergyJ |
|:------------|:-------------|----------------:|-----------------:|-------------:|------------------:|-----------------:|---------------:|
| default     | AODV         |          37.923 |            0.071 |      281.461 |          13217.1  |          5823.64 |        451.667 |
| default     | AOMDV(mp=3)  |          37.581 |            0.07  |      280.945 |          12782.9  |          5531.92 |        451.604 |
| default     | DSDV         |          24.524 |            0.046 |       45.605 |           4981.94 |          1988.57 |        451.182 |
| default     | PMAODV(mp=2) |          37.195 |            0.069 |      280.036 |          12713    |          5527.9  |        451.614 |
| default     | PMAODV(mp=3) |          37.748 |            0.07  |      288.298 |          12856.7  |          5604.9  |        451.654 |
| default     | PMAODV(mp=4) |          38.469 |            0.072 |      288.83  |          12758.4  |          5663.46 |        451.622 |
| density     | AODV         |          24.865 |            0.12  |       52.981 |          68702    |         47626.7  |       1151.17  |
| density     | AOMDV(mp=3)  |          24.311 |            0.118 |       53.845 |          72173.8  |         52104.9  |       1150.56  |
| density     | DSDV         |          21.372 |            0.104 |       25.701 |          22155.3  |         14236.9  |       1147.21  |
| density     | PMAODV(mp=2) |          25.267 |            0.121 |       54.361 |          71015.8  |         51280.7  |       1152.54  |
| density     | PMAODV(mp=3) |          25.733 |            0.123 |       64.849 |          86802.9  |         65571.8  |       1152.09  |
| density     | PMAODV(mp=4) |          25.128 |            0.121 |       61.946 |          84886.4  |         63834.8  |       1152.31  |
| multi-flow  | AODV         |          58.92  |            0.03  |      216.456 |           5603.46 |          3722.8  |        468.013 |
| multi-flow  | AOMDV(mp=3)  |          59.013 |            0.03  |      219.181 |           5616.81 |          3727.64 |        468.015 |
| multi-flow  | DSDV         |          33.713 |            0.017 |       38.624 |           3261.1  |          2088.99 |        467.83  |
| multi-flow  | PMAODV(mp=2) |          59.527 |            0.03  |      220.82  |           5700.07 |          3766.41 |        468.033 |
| multi-flow  | PMAODV(mp=3) |          59.264 |            0.03  |      225.209 |           5709.51 |          3764.77 |        468.028 |
| multi-flow  | PMAODV(mp=4) |          58.961 |            0.03  |      226.991 |           5677.56 |          3763.47 |        468.026 |
| stress-all  | AODV         |          55.871 |            0.07  |      343.976 |           9242.94 |          4542.19 |        468.696 |
| stress-all  | AOMDV(mp=3)  |          55.708 |            0.07  |      338.629 |           9205.59 |          4525.04 |        468.674 |
| stress-all  | DSDV         |          27.42  |            0.034 |       20.414 |           3827.12 |          2129.91 |        468.766 |
| stress-all  | PMAODV(mp=2) |          55.752 |            0.07  |      337.814 |           9235.83 |          4558.89 |        468.692 |
| stress-all  | PMAODV(mp=3) |          55.352 |            0.069 |      344.482 |           9221.31 |          4579.6  |        468.669 |
| stress-all  | PMAODV(mp=4) |          55.483 |            0.07  |      348.259 |           9271.99 |          4601.6  |        468.664 |
| stress-load | AODV         |          37.369 |            0.347 |      132.013 |          48382.6  |         16097.3  |        458.481 |
| stress-load | AOMDV(mp=3)  |          36.637 |            0.338 |      129.422 |          46195.5  |         14248.7  |        458.382 |
| stress-load | DSDV         |          26.186 |            0.249 |       21.885 |          17750.7  |          1966.71 |        457.325 |
| stress-load | PMAODV(mp=2) |          36.256 |            0.336 |      131.467 |          48365.8  |         15643.1  |        458.469 |
| stress-load | PMAODV(mp=3) |          36.583 |            0.34  |      131.714 |          49599.1  |         16242.4  |        458.45  |
| stress-load | PMAODV(mp=4) |          35.985 |            0.333 |      131.114 |          48127.7  |         15357.5  |        458.434 |
| stress-mob  | AODV         |          40.115 |            0.073 |      464.387 |          14303    |          6884.13 |        451.614 |
| stress-mob  | AOMDV(mp=3)  |          38.717 |            0.07  |      452.897 |          13693.6  |          6474.48 |        451.516 |
| stress-mob  | DSDV         |          21.676 |            0.04  |       43.765 |           3912.11 |          2023.7  |        451.149 |
| stress-mob  | PMAODV(mp=2) |          39.7   |            0.073 |      459.153 |          14388.7  |          6858.94 |        451.63  |
| stress-mob  | PMAODV(mp=3) |          39.194 |            0.072 |      459.664 |          14290.1  |          6771.42 |        451.602 |
| stress-mob  | PMAODV(mp=4) |          39.396 |            0.072 |      457.122 |          14282.1  |          6813.14 |        451.592 |

## Tables 3/4 - Welch t-test vs AODV, Holm-Bonferroni (alpha=0.05)

| family   | variant      | metric          |   tests |   sig_raw |   sig_holm |   sig_higher |   sig_lower |   cohens_d |
|:---------|:-------------|:----------------|--------:|----------:|-----------:|-------------:|------------:|-----------:|
| default  | DSDV         | deliveryRatio   |      16 |        16 |         16 |            0 |          16 |     -1.521 |
| default  | DSDV         | throughputMbps  |      16 |        16 |         16 |            0 |          16 |     -0.898 |
| default  | DSDV         | avgDelayMs      |      16 |        16 |         16 |            0 |          16 |     -1.371 |
| default  | DSDV         | routingOverhead |      16 |        16 |         16 |            0 |          16 |     -1.259 |
| default  | DSDV         | controlPackets  |      16 |        16 |         16 |            0 |          16 |     -1.27  |
| default  | DSDV         | totalEnergyJ    |      16 |         7 |          6 |            0 |           6 |     -0.003 |
| default  | AOMDV(mp=3)  | deliveryRatio   |      16 |         0 |          0 |            0 |           0 |     -0.033 |
| default  | AOMDV(mp=3)  | throughputMbps  |      16 |         0 |          0 |            0 |           0 |     -0.017 |
| default  | AOMDV(mp=3)  | avgDelayMs      |      16 |         0 |          0 |            0 |           0 |     -0.002 |
| default  | AOMDV(mp=3)  | routingOverhead |      16 |         0 |          0 |            0 |           0 |     -0.052 |
| default  | AOMDV(mp=3)  | controlPackets  |      16 |         0 |          0 |            0 |           0 |     -0.077 |
| default  | AOMDV(mp=3)  | totalEnergyJ    |      16 |         0 |          0 |            0 |           0 |     -0     |
| default  | PMAODV(mp=2) | deliveryRatio   |      16 |         0 |          0 |            0 |           0 |     -0.067 |
| default  | PMAODV(mp=2) | throughputMbps  |      16 |         0 |          0 |            0 |           0 |     -0.061 |
| default  | PMAODV(mp=2) | avgDelayMs      |      16 |         0 |          0 |            0 |           0 |     -0.006 |
| default  | PMAODV(mp=2) | routingOverhead |      16 |         1 |          0 |            0 |           0 |     -0.06  |
| default  | PMAODV(mp=2) | controlPackets  |      16 |         1 |          0 |            0 |           0 |     -0.079 |
| default  | PMAODV(mp=2) | totalEnergyJ    |      16 |         0 |          0 |            0 |           0 |     -0     |
| default  | PMAODV(mp=3) | deliveryRatio   |      16 |         0 |          0 |            0 |           0 |     -0.016 |
| default  | PMAODV(mp=3) | throughputMbps  |      16 |         0 |          0 |            0 |           0 |     -0.011 |
| default  | PMAODV(mp=3) | avgDelayMs      |      16 |         0 |          0 |            0 |           0 |      0.029 |
| default  | PMAODV(mp=3) | routingOverhead |      16 |         2 |          0 |            0 |           0 |     -0.042 |
| default  | PMAODV(mp=3) | controlPackets  |      16 |         1 |          0 |            0 |           0 |     -0.058 |
| default  | PMAODV(mp=3) | totalEnergyJ    |      16 |         1 |          0 |            0 |           0 |     -0     |
| default  | PMAODV(mp=4) | deliveryRatio   |      16 |         0 |          0 |            0 |           0 |      0.05  |
| default  | PMAODV(mp=4) | throughputMbps  |      16 |         0 |          0 |            0 |           0 |      0.03  |
| default  | PMAODV(mp=4) | avgDelayMs      |      16 |         0 |          0 |            0 |           0 |      0.031 |
| default  | PMAODV(mp=4) | routingOverhead |      16 |         0 |          0 |            0 |           0 |     -0.054 |
| default  | PMAODV(mp=4) | controlPackets  |      16 |         1 |          0 |            0 |           0 |     -0.041 |
| default  | PMAODV(mp=4) | totalEnergyJ    |      16 |         1 |          0 |            0 |           0 |     -0     |
| density  | DSDV         | deliveryRatio   |       3 |         2 |          0 |            0 |           0 |     -0.799 |
| density  | DSDV         | throughputMbps  |       3 |         2 |          0 |            0 |           0 |     -0.677 |
| density  | DSDV         | avgDelayMs      |       3 |         3 |          3 |            0 |           3 |     -2.213 |
| density  | DSDV         | routingOverhead |       3 |         3 |          3 |            0 |           3 |     -1.668 |
| density  | DSDV         | controlPackets  |       3 |         3 |          3 |            0 |           3 |     -1.365 |
| density  | DSDV         | totalEnergyJ    |       3 |         3 |          3 |            0 |           3 |     -0.017 |
| density  | AOMDV(mp=3)  | deliveryRatio   |       3 |         0 |          0 |            0 |           0 |     -0.102 |
| density  | AOMDV(mp=3)  | throughputMbps  |       3 |         0 |          0 |            0 |           0 |     -0.097 |
| density  | AOMDV(mp=3)  | avgDelayMs      |       3 |         0 |          0 |            0 |           0 |      0.039 |
| density  | AOMDV(mp=3)  | routingOverhead |       3 |         1 |          0 |            0 |           0 |      0.076 |
| density  | AOMDV(mp=3)  | controlPackets  |       3 |         1 |          0 |            0 |           0 |      0.108 |
| density  | AOMDV(mp=3)  | totalEnergyJ    |       3 |         0 |          0 |            0 |           0 |     -0.003 |
| density  | PMAODV(mp=2) | deliveryRatio   |       3 |         0 |          0 |            0 |           0 |      0.072 |
| density  | PMAODV(mp=2) | throughputMbps  |       3 |         0 |          0 |            0 |           0 |      0.038 |
| density  | PMAODV(mp=2) | avgDelayMs      |       3 |         0 |          0 |            0 |           0 |      0.063 |
| density  | PMAODV(mp=2) | routingOverhead |       3 |         0 |          0 |            0 |           0 |      0.051 |
| density  | PMAODV(mp=2) | controlPackets  |       3 |         0 |          0 |            0 |           0 |      0.087 |
| density  | PMAODV(mp=2) | totalEnergyJ    |       3 |         0 |          0 |            0 |           0 |      0.006 |
| density  | PMAODV(mp=3) | deliveryRatio   |       3 |         0 |          0 |            0 |           0 |      0.151 |
| density  | PMAODV(mp=3) | throughputMbps  |       3 |         0 |          0 |            0 |           0 |      0.127 |
| density  | PMAODV(mp=3) | avgDelayMs      |       3 |         1 |          0 |            0 |           0 |      0.47  |
| density  | PMAODV(mp=3) | routingOverhead |       3 |         1 |          0 |            0 |           0 |      0.331 |
| density  | PMAODV(mp=3) | controlPackets  |       3 |         1 |          0 |            0 |           0 |      0.357 |
| density  | PMAODV(mp=3) | totalEnergyJ    |       3 |         0 |          0 |            0 |           0 |      0.004 |
| density  | PMAODV(mp=4) | deliveryRatio   |       3 |         0 |          0 |            0 |           0 |      0.05  |
| density  | PMAODV(mp=4) | throughputMbps  |       3 |         0 |          0 |            0 |           0 |      0.028 |
| density  | PMAODV(mp=4) | avgDelayMs      |       3 |         1 |          1 |            1 |           0 |      0.358 |
| density  | PMAODV(mp=4) | routingOverhead |       3 |         1 |          0 |            0 |           0 |      0.29  |
| density  | PMAODV(mp=4) | controlPackets  |       3 |         1 |          0 |            0 |           0 |      0.314 |
| density  | PMAODV(mp=4) | totalEnergyJ    |       3 |         0 |          0 |            0 |           0 |      0.005 |
| stress   | DSDV         | deliveryRatio   |      62 |        62 |         62 |            0 |          62 |     -1.679 |
| stress   | DSDV         | throughputMbps  |      62 |        62 |         62 |            0 |          62 |     -0.345 |
| stress   | DSDV         | avgDelayMs      |      62 |        62 |         62 |            0 |          62 |     -1.291 |
| stress   | DSDV         | routingOverhead |      62 |        60 |         60 |            0 |          60 |     -0.645 |
| stress   | DSDV         | controlPackets  |      62 |        62 |         62 |            0 |          62 |     -0.763 |
| stress   | DSDV         | totalEnergyJ    |      62 |        29 |         15 |            1 |          14 |     -0.003 |
| stress   | AOMDV(mp=3)  | deliveryRatio   |      62 |         0 |          0 |            0 |           0 |     -0.038 |
| stress   | AOMDV(mp=3)  | throughputMbps  |      62 |         0 |          0 |            0 |           0 |     -0.021 |
| stress   | AOMDV(mp=3)  | avgDelayMs      |      62 |         0 |          0 |            0 |           0 |     -0.015 |
| stress   | AOMDV(mp=3)  | routingOverhead |      62 |         2 |          0 |            0 |           0 |     -0.029 |
| stress   | AOMDV(mp=3)  | controlPackets  |      62 |         3 |          0 |            0 |           0 |     -0.059 |
| stress   | AOMDV(mp=3)  | totalEnergyJ    |      62 |         0 |          0 |            0 |           0 |     -0     |
| stress   | PMAODV(mp=2) | deliveryRatio   |      62 |         2 |          0 |            0 |           0 |     -0.019 |
| stress   | PMAODV(mp=2) | throughputMbps  |      62 |         2 |          0 |            0 |           0 |     -0.019 |
| stress   | PMAODV(mp=2) | avgDelayMs      |      62 |         0 |          0 |            0 |           0 |     -0.007 |
| stress   | PMAODV(mp=2) | routingOverhead |      62 |         0 |          0 |            0 |           0 |      0.002 |
| stress   | PMAODV(mp=2) | controlPackets  |      62 |         1 |          0 |            0 |           0 |     -0.01  |
| stress   | PMAODV(mp=2) | totalEnergyJ    |      62 |         0 |          0 |            0 |           0 |      0     |
| stress   | PMAODV(mp=3) | deliveryRatio   |      62 |         0 |          0 |            0 |           0 |     -0.032 |
| stress   | PMAODV(mp=3) | throughputMbps  |      62 |         0 |          0 |            0 |           0 |     -0.016 |
| stress   | PMAODV(mp=3) | avgDelayMs      |      62 |         0 |          0 |            0 |           0 |      0.003 |
| stress   | PMAODV(mp=3) | routingOverhead |      62 |         0 |          0 |            0 |           0 |      0.012 |
| stress   | PMAODV(mp=3) | controlPackets  |      62 |         1 |          0 |            0 |           0 |      0.002 |
| stress   | PMAODV(mp=3) | totalEnergyJ    |      62 |         0 |          0 |            0 |           0 |     -0     |
| stress   | PMAODV(mp=4) | deliveryRatio   |      62 |         0 |          0 |            0 |           0 |     -0.042 |
| stress   | PMAODV(mp=4) | throughputMbps  |      62 |         0 |          0 |            0 |           0 |     -0.027 |
| stress   | PMAODV(mp=4) | avgDelayMs      |      62 |         0 |          0 |            0 |           0 |      0.005 |
| stress   | PMAODV(mp=4) | routingOverhead |      62 |         0 |          0 |            0 |           0 |     -0.002 |
| stress   | PMAODV(mp=4) | controlPackets  |      62 |         1 |          0 |            0 |           0 |     -0.017 |
| stress   | PMAODV(mp=4) | totalEnergyJ    |      62 |         1 |          0 |            0 |           0 |     -0     |

## Table 5 - normalized load, default scenario (mean ± SD per run)

NRL_legacy = routingOverhead / delivered; NRL_control = control packets / delivered; delivered packets are counted exactly by FlowMonitor.

| variant      |   n_runs |   runs_without_delivery | NRL_legacy    | NRL_control   | energy_per_pkt_J   |
|:-------------|---------:|------------------------:|:--------------|:--------------|:-------------------|
| AODV         |      320 |                       0 | 3.869 ± 1.877 | 1.721 ± 0.715 | 0.151 ± 0.053      |
| DSDV         |      320 |                       0 | 2.433 ± 2.001 | 0.858 ± 0.454 | 0.231 ± 0.088      |
| AOMDV(mp=3)  |      320 |                       0 | 3.790 ± 1.752 | 1.663 ± 0.635 | 0.151 ± 0.054      |
| PMAODV(mp=2) |      320 |                       0 | 3.877 ± 1.795 | 1.719 ± 0.697 | 0.155 ± 0.059      |
| PMAODV(mp=3) |      320 |                       0 | 3.805 ± 1.738 | 1.698 ± 0.674 | 0.152 ± 0.058      |
| PMAODV(mp=4) |      320 |                       0 | 3.707 ± 1.686 | 1.682 ± 0.692 | 0.149 ± 0.056      |

## PMAODV route selection

| scenario    | variant      |   selEvents |   selMultiCandidate |   selNonMinHop |   multi_%_of_all |   nonMin_%_of_all |   nonMin_%_of_multi |
|:------------|:-------------|------------:|--------------------:|---------------:|-----------------:|------------------:|--------------------:|
| default     | PMAODV(mp=2) |     2816227 |               90001 |          19379 |             3.2  |              0.69 |               21.53 |
| default     | PMAODV(mp=3) |     2821626 |              139751 |          35216 |             4.95 |              1.25 |               25.2  |
| default     | PMAODV(mp=4) |     2814256 |              174470 |          47269 |             6.2  |              1.68 |               27.09 |
| density     | PMAODV(mp=2) |     1617691 |               71018 |          19419 |             4.39 |              1.2  |               27.34 |
| density     | PMAODV(mp=3) |     1885068 |              114895 |          37063 |             6.1  |              1.97 |               32.26 |
| density     | PMAODV(mp=4) |     1838076 |              146039 |          50912 |             7.95 |              2.77 |               34.86 |
| multi-flow  | PMAODV(mp=2) |      649227 |               17408 |           3731 |             2.68 |              0.57 |               21.43 |
| multi-flow  | PMAODV(mp=3) |      651927 |               27311 |           6404 |             4.19 |              0.98 |               23.45 |
| multi-flow  | PMAODV(mp=4) |      650662 |               33835 |           8259 |             5.2  |              1.27 |               24.41 |
| stress-all  | PMAODV(mp=2) |     1481498 |               38520 |           7828 |             2.6  |              0.53 |               20.32 |
| stress-all  | PMAODV(mp=3) |     1481593 |               60369 |          13137 |             4.07 |              0.89 |               21.76 |
| stress-all  | PMAODV(mp=4) |     1485708 |               82436 |          19194 |             5.55 |              1.29 |               23.28 |
| stress-load | PMAODV(mp=2) |    13039046 |              542464 |         134314 |             4.16 |              1.03 |               24.76 |
| stress-load | PMAODV(mp=3) |    13145458 |              889286 |         253620 |             6.76 |              1.93 |               28.52 |
| stress-load | PMAODV(mp=4) |    12979682 |             1125813 |         335721 |             8.67 |              2.59 |               29.82 |
| stress-mob  | PMAODV(mp=2) |     3131539 |               99673 |          22990 |             3.18 |              0.73 |               23.07 |
| stress-mob  | PMAODV(mp=3) |     3122370 |              155922 |          40521 |             4.99 |              1.3  |               25.99 |
| stress-mob  | PMAODV(mp=4) |     3116862 |              203270 |          55646 |             6.52 |              1.79 |               27.38 |
