Program 3, Part 1: Solution
===========================

1. I was running the experiments on Dual Core Intel Core i5 with 4 virtual cores. For this processor the maximum theoretical speedup is `4x` for single core and `16x` for the multi-core when ISPC is compiled for `see4` (`4`-wide SIMD instructions). I've observed the speedup of `3.30x` on average for the SIMD only case. The observed numbers are lower then the maximum possible due to the unbalanced workload. The most problematic regions of the image are those with high gradients (borders). For this regions computation of one value of the 4-wide SIMD vector can take significantly longer then for the others, which will result in stalling ALUs.

Program 3, Part 2: Solution
===========================

1. For the ISPC tasks I've observed the average speedup of `4.40x`. The speedup over the single-core SIMD is `1.34x`.
2. According to Intel: 
    > In general, one should launch many more tasks than there are processors in the system to ensure good load-balancing, but not so many that the overhead of scheduling and running tasks dominates the computation.

    When choosing the number of tasks one should insure that `N % (T*W) == 0`, where `N` is number of data points, `T` is namber of tass and `W` is the width of SIMD. The total number of data to be processed is `960000` and ISPC is compiled to use `4`-wide SIMD instructions, that is why I've used `32` ISPC tasks.
3. In `pthread` abstraction work distribution is done by the programmer, whereas ISPC tasks abstraction distribution is delegated to other system (ISPC compiler / OS? I'm not sure here).