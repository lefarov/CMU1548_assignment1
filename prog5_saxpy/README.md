Program 5: Solution
===================

1. Speedup is about `1.03x` on average. The problem is that program is bandwidth limited, i.e. high ratio of memory accesses to arithmetic operations. Memory system cannot keep up and latency hiding by means of ISPC tasks is not working. It can be improved (point 3).
2. According to my limited understanding of assembly, `result[i] = scale * X[i] + Y[i];` actually results in 3 vectorized memory accesses:

        movups  xmm3, xmmword ptr [r11 + rax]
        movups  xmm4, xmmword ptr [r11 + rax + 16]
        mulps   xmm4, xmm2
        mulps   xmm3, xmm2
        movups  xmm5, xmmword ptr [r10 + rax]
        addps   xmm5, xmm3
        movups  xmm3, xmmword ptr [r10 + rax + 16]
        addps   xmm3, xmm4
        movups  xmmword ptr [r9 + rax + 16], xmm3
        movups  xmmword ptr [r9 + rax], xmm5

    Maybe initialization was counted as separate memory access here. However it should be 6 then (sinse `X`, `Y` and `result` are initialized).
3. I have managed to improve the speedup from ISPC tasks to `1.34x` on average by using the streaming store operation `streaming_store`. I found [this presentation by intel](https://software.intel.com/content/www/us/en/develop/videos/advanced-single-instruction-multiple-data-simd-programming-with-intel-implicit-spmd-program.html) quite useful (starting from `0:28`).

