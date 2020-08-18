Program 1: Solution
===================

2. Speedup is not linear in the number of coures becuase spatial decomposition leads to the unbalanced workload. Computation of the bright regions requres much more iterations then the dark ones. If work is decomposed spatially, some threads operating on much harder regions of the space. The speedup for the view `2` is higher since spatial decompostions results in better balanced assignment than view `1`.
3. Time required for the completion per thread:

        [mandelbrot thread 3 worktime]:         [38.539] ms
        [mandelbrot thread 0 worktime]:         [54.738] ms
        [mandelbrot thread 2 worktime]:         [170.309] ms
        [mandelbrot thread 1 worktime]:         [202.695] ms

4. Interleaved assignment results in better workload balance and linear speedup. Run `python run_comparison.py` to plot the speedup over the number of threads.

    ![speedup plot][plot]

[plot]: speedup_comparison.svg
