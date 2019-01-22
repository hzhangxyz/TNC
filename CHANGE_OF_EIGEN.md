- put eigen in the same directory, to let `Makefile` can include it as it saying `-I./eigen3/unsupported`
- add the following line in `TensorBase.h` at the end of `class TensorBase` public domain
```
    #ifdef EIGEN_TENSOR_PLUGIN
    #include EIGEN_TENSOR_PLUGIN
    #endif
``` 