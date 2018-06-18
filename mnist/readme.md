Work log:

* Programming fundalmentals
  * [P1] const Matrix view
  * [P2] Write more unit tests for Tensor. e.g. operator+. 
  * [P1] Associate gradient with the Tensor 
* [P0] Performance optimizations - so I can train more iterations faster so I can easily tune the algorithm;
  * [P0] Optimize convolve
     * Add multithreading
       * Loss operator takes an explicit weight setting
     * Add consider other techniques
  * [P2] O(n^2.8) matrix multiplication algorithm
  * [P1] Try accessing RHS matrix row wise and measure cache misses. 
  * [P2] Layout matrix in cache friendly ways
  * [P0] Consider running the whole forward and backward pass in parallel. 
    * [P1] Also need to ensure we speed up the periodical loss evaluation run (not the test error rate run). 
    * [P0] Ideas of how to do this:
    * We can break down a batch into M batches. 
    * M batches can do forward and backward pass independently (the final scaler needs to sum jointly).
    * This is because each term from the M batch can have their own gradients, which are then summed together.
    * Implementation ideas include thread local or explicit thread (sub-batch) only state. We should aim for simplicity - execution structure should be hidden from the expression structure.  
* Model architecture
  * CNN 
    * Multi-channel pooling (e.g. invariant to rotations)
    * Out of boundary pixel values: does the choice of default value matter?
    * CNN layer weight gradient seems to be very small. 
  * Different families of ReLu
    * DReLu: https://openreview.net/forum?id=H1DGha1CZ
* Numeric stability
  * Overflow problem: can gradient be a very large number (e.g. would capping gradient computation, e.g. matrix multiplication, still be correct?)
    * Can be diagnosed by printing gradient norm / parameter norm. So far there are no problems. 
  * [P0] double vs. float: conclusion: we should use double. Why unnecessarily restrict ourselves? 
    * Using double resolves the overflowing problem. 
* Optimization algorithm improvement:
  * [P1] Try AdaGrad on the current optimization problem 
  * [P1] LARS: https://arxiv.org/pdf/1708.03888.pdf
  * [P0] Input normalization
    * Batch normalization
      * How would we apply normalization during inference? 
    * Input normalization
  * Initialization
    * Gaussian fill for NN weights (may not work because training error was already low).
* Tooling
  * [P1] Model loading: also prints architecture so can more easily recover from past runs.
    * Include the arch description in the model output so we know exactly the run that produced the model. 
  * [P2] Config supports reading multiple entries to perform multiple runs.
  * [P0] Learning curve
    * [P1] Activation stats 

Notes:
  * Time a fixed run so we can measure performance improvements [done]
  * Useful links
    * https://matplotlib.org/users/image_tutorial.html
    * MNIST dataset reader in Python: https://github.com/hsjeong5/MNIST-for-Numpy/blob/master/mnist.py
