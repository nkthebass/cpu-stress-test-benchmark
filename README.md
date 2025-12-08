# cpu-stress-test-benchmark`

This is my first public progect, I am attempting to make a simple, easy to use, and accurate CPU benchmarking, stress testing and hardware info program. 
The current release has a stress testing feture that supports 1,2,4,6,8,12,18,20,24 threaded CPUS *note on high end cpus like a r7 7700x may not be 100% utilized* the stress test is aiming to make a stressful enviorment for your cpu to test temps/ stability.
The benchmarking feture currently uses Floating-Point Operations and Integer Operations ↓↓↓

// Floating-Point Operations
f_val = Math.sqrt(f_val * 0.999999 + 0.000001);
f_val = Math.sin(f_val * Math.PI);
f_val = Math.cos(f_val / 2);

// Integer Operations (Stress the ALU)
i_val = (i_val * 77) % 2000000000;
i_val = (i_val * 13 + 5) % 2000000000;
i_val = (i_val ^ 0xDEADBEEF) % 2000000000;

I recently added a settings menu to change the color of the text/ background and will be implementing a spread sheat with data from cpus previously benchmarked to add relevancy to the score.
