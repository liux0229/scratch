Work log:

* Programming fundalmentals
  * [P1] const Matrix view
  * [P2] Write more unit tests for Tensor. e.g. operator+. 
  * [P1] Associate gradient with the Tensor 
* [P0] Performance optimizations - so I can train more iterations faster so I can easily tune the algorithm;
  * [P0] Optimize convolve ConvolutionalLayer (further work; requires new profiling)
    * Possible ideas:
      * Unroll smaller patch 
      * Express convolve as weighted and shifted additions - can we make the access 1d?
      * Matrix access no checking 
  * [P2] O(n^2.8) matrix multiplication algorithm
  * [P1] Try accessing RHS matrix row wise and measure cache misses. 
  * [P2] Layout matrix in cache friendly ways
  * [P0] Why is average Tensor size 30 M but theoretically it should be only 30 * 40 * 28 * 28 * 8 = 8 M?
  * [P0] Avoid the excessive vector creation?
* Model architecture
  * CNN 
    * Multi-channel pooling (e.g. invariant to rotations)
    * Out of boundary pixel values: does the choice of default value matter?
    * Initialization strategy
    * Static config access
      * Is this still necessary?
    * Check the first layer learned edges.
      * http://cs231n.github.io/understanding-cnn/
      * https://jacobgil.github.io/deeplearning/filter-visualizations
        * Perform gradient ascent to find an input image that can maximize the output of a filter (can take the average of the output image pixels)
      * https://www.youtube.com/watch?v=cNBBNAxC8l4&index=22&list=PLZbbT5o_s2xq7LwI2y8_QtvuXZedL6tQU
        * Visualize the output from an input image
        * Find out the input impage that results in the maximum layer output
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
  * [P0] Incorporate memory profiling debugging: https://fb.quip.com/MkODA3sHC4PO
  * [P1] Model loading: also prints architecture so can more easily recover from past runs.
    * Include the arch description in the model output so we know exactly the run that produced the model. 
  * [P2] Config supports reading multiple entries to perform multiple runs.
  * [P0] Learning curve
    * [P1] Activation stats 
  * Shut down training through C-c
  * Dead ReLu
  * Display the feature maps from the CNN layer

Notes:
  * Useful links
    * https://matplotlib.org/users/image_tutorial.html
    * MNIST dataset reader in Python: https://github.com/hsjeong5/MNIST-for-Numpy/blob/master/mnist.py
