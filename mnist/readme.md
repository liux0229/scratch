Work log:

* Programming fundalmentals
  * [P1] const Matrix view
  * [P2] Write more unit tests for Tensor. e.g. operator+. 
  * [P1] Associate gradient with the Tensor 
* [P0] Performance optimizations - so I can train more iterations faster so I can easily tune the algorithm;
  * [P0] Optimize convolve
     * Add consider other techniques. This requires further improvements. 
  * [P2] O(n^2.8) matrix multiplication algorithm
  * [P1] Try accessing RHS matrix row wise and measure cache misses. 
  * [P2] Layout matrix in cache friendly ways
* Model architecture
  * CNN 
    * Multi-channel pooling (e.g. invariant to rotations)
    * Out of boundary pixel values: does the choice of default value matter?
    * Initialization strategy
    * Static config access
      * Is this still necessary?
    * Check the first layer learned edges.
  * Different families of ReLu
    * DReLu: https://openreview.net/forum?id=H1DGha1CZ
* Numeric stability
  * Overflow problem: can gradient be a very large number (e.g. would capping gradient computation, e.g. matrix multiplication, still be correct?)
* Optimization algorithm improvement:
  * [P1] Try AdaGrad on the current optimization problem 
  * [P1] LARS: https://arxiv.org/pdf/1708.03888.pdf
  * [P0] Input normalization
    * Batch normalization
      * How would we apply normalization during inference? 
    * Input normalization
  * Initialization
    * Gaussian fill for NN weights (may not work because training error was already low).
* Tooling & program control
  * [P1] Model loading: also prints architecture so can more easily recover from past runs.
    * Include the arch description in the model output so we know exactly the run that produced the model. 
  * [P2] Config supports reading multiple entries to perform multiple runs.
  * [P0] Learning curve
    * [P1] Activation stats 
  * Shut down training through C-c

Notes:
  * Useful links
    * https://matplotlib.org/users/image_tutorial.html
    * MNIST dataset reader in Python: https://github.com/hsjeong5/MNIST-for-Numpy/blob/master/mnist.py
