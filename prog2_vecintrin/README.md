Program 2: Solution
===================

2. Vecot utilization for differetn SIMD width:

        Vector Width:              2
        Total Vector Instructions: 162729
        Vector Utilization:        77.570992%
        Utilized Vector Lanes:     252461
        Total Vector Lanes:        325458
        -------------------------------------
        Vector Width:              4
        Total Vector Instructions: 94577
        Vector Utilization:        70.228755%
        Utilized Vector Lanes:     265681
        Total Vector Lanes:        378308
        -------------------------------------
        Vector Width:              8
        Total Vector Instructions: 51629
        Vector Utilization:        66.429962%
        Utilized Vector Lanes:     274377
        Total Vector Lanes:        413032
        -------------------------------------
        Vector Width:              16
        Total Vector Instructions: 26969
        Vector Utilization:        64.663363%
        Utilized Vector Lanes:     279025
        Total Vector Lanes:        431504

    Vecotr utilization is decreased with the increasing vector width. Since the values in `exponents` array are generated randomly exponeniation is taking different number of iterations to converge for every element. With the increase of vector length the probability of unbalanced workload within the SIMD vector is increasing, which (probably) results in lower utilization.