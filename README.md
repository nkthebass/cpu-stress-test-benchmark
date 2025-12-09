# cpu-stress-test-benchmark`

This is my first public project, I am attempting to make a simple, easy to use, and accurate CPU benchmarking, stress testing and hardware info program. 
The current release has a stress testing feture that supports 1,2,4,6,8,12,18,20,24 threaded CPUS the stress test is aiming to make a stressful enviorment for your cpu to test temps/ stability.
The benchmarking feture currently uses Floating-Point Operations and Integer Operations and more! ↓↓↓

 result[i] = std::sqrt(a * a + b * b); - Square root of sum of squares (Pythagorean)

  result[i] += std::sin(a) * std::cos(b); - Trigonometric operations

  result[i] *= std::exp(std::log(a + 1.0)); - Exponential and logarithm

  intVal = (intVal * 0x5DEECE66DLL + 0xBLL) & ((1LL << 48) - 1); - Integer multiplication (LCG algorithm)

   intData[i] = static_cast<int>(intVal >> 16); - Bit shifting

   std::swap(result[i], result[(i + 128) % size]); - Memory operations

   std::swap(intData[i], intData[(i + 256) % size]); - Cache stress

   

I recently added a settings menu to change the color of the text/ background and will be implementing a spread sheat with data from cpus previously benchmarked to add relevancy to the score.
