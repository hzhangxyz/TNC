## Eigen上的custom修改
- at the end of `class TensorBase` public domain, 加入了leg_info的成员, 以及leg_rename函数
- `Tensor.h` 中的constructor和operator=需要各种复制`leg_info`, 两个来源，一个运算，一个复制(只有在需要resize的时候需要)
- 在cwise那些东西里加了leg info，从而实现scalar功能

## Eigen_Node接口
- leg的维护需要手动对各种op做wrap，所以除非在外面设定`leg_info`，只能使用plugin中的几个函数
- scalar在里面实现, 可以直接用cwise操作
- leg_rename在里面实现的

### 外部操作
- contract
- svd, qr
- transpose
- multiple
- max_normalize, normalize

### what else
- debug_tensor, Leg's var

## 一些与Eigen相关的问题(效率问题)

- contract什么的做不到静态，所以contract什么的都需要动态malloc
- 可能浪费的copy有：svd和qr中的shuffle，svd和qr的结果再copy回tensor:

|         |shuffle          |copy回tensor             |
|---------|-----------------|-------------------------|
|qr       |被qr inplace利用  |顺便处理Q，R矩阵的post处理  |
|svd      |真的浪费了         |顺便cut                  |

## SVD，QR选择的问题
- (D=10)×2×2=40大概是svd的量级，是16的两倍多，所以就用BDCSVD吧
- svd的时候,如何转置最快?(这里贪心策略吧)