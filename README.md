## Eigen上的custom修改
- add the following line in `TensorBase.h` at the end of `class TensorBase` public domain
```
Eigen::array<Leg, DerivedTraits::NumDimensions> leg_info = DefaultLeg<DerivedTraits::NumDimensions>::value;
``` 
- `Tensor.h` 中的constructor和operator=需要各种复制`leg_info`, 两个来源，一个运算，一个复制(只有在需要resize的时候需要)
- leg的维护需要手动对各种op做wrap，所以除非在外面设定`leg_info`，只能使用plugin中的几个函数
- 有一个lapacke.h定义了I的macro引起的冲突bug，重命名即可

## 一些与Eigen相关的问题
- contract什么的做不到静态，所以contract什么的都需要动态malloc
- 因为没有asMatrix, asTensor所以svd那些函数中现在多一层复制，不过也因此cut比较方便，但是如果以后有静态的svd的话，这里也是个问题