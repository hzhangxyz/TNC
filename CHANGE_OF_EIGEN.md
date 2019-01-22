- put eigen in the same directory, to let `Makefile` can include it as it saying `-I./eigen3/unsupported`
- add the following line in `TensorBase.h` at the end of `class TensorBase` public domain
```
    #ifdef EIGEN_TENSOR_PLUGIN
    #include EIGEN_TENSOR_PLUGIN
    #endif
``` 
- `Tensor.h` 中的constructor和operator=需要各种复制`leg_info`, 两个来源，一个运算，一个复制(只有在需要resize的时候需要)
- leg的维护需要手动对各种op做wrap，所以除非在外面设定`leg_info`，只能使用plugin中的几个函数