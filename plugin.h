std::vector<Leg> leg_info = {Phy1,Phy2,Phy3,Phy4};// default is hamiltonian's leg since it is usually const

EIGEN_STRONG_INLINE void set_leg(const std::vector<Leg>& new_leg){
    leg_info = new_leg;
}

EIGEN_STRONG_INLINE void unsafe_set_leg(std::vector<Leg>& new_leg){
    leg_info.swap(new_leg);
}

//Eigen::array<Eigen::IndexPair<int>, 1> dims = { Eigen::IndexPair<int>(3, 2) };

template<typename OtherDerived, unsigned long ContractNum> EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
const TensorContractionOp<const Eigen::array<Eigen::IndexPair<Index>, ContractNum>, const Derived, const OtherDerived, const NoOpOutputKernel>
node_contract(const OtherDerived& other, const Eigen::array<Leg, ContractNum>& leg1, const Eigen::array<Leg, ContractNum>& leg2) const {
    Eigen::array<Index, ContractNum> index1 = this->get_index_from_leg(leg1);
    Eigen::array<Index, ContractNum> index2 = other.get_index_from_leg(leg2);
    Eigen::IndexPair<Index> pair {2, 1};
    Eigen::array<Eigen::IndexPair<Index>, ContractNum> dims;
    for(int i=0;i<ContractNum;i++){
        dims[i].first = index1[i];
        dims[i].second = index2[i];
    }
    return this->contract(other, dims);
}

template<unsigned long ContractNum>// 迷，不用unsigned long int 的话template不能匹配
Eigen::array<Index, ContractNum> get_index_from_leg(const Eigen::array<Leg, ContractNum>& legs) const {
    /* 输入一个leg array, 返回一个对应的index array， 不存在的leg对应-1(!!!) */
    Eigen::array<Index, ContractNum> res;
    for(int i=0;i<ContractNum;i++){
        std::vector<Leg>::const_iterator index = std::find(this->leg_info.begin(), this->leg_info.end(), legs[i]);
        if(index==this->leg_info.end()){
            res[i] = -1;
        }else{
            res[i] = std::distance(this->leg_info.begin(), index);
        }
    }
    return res;
}

// 问题1：是不是需要吧int改成其他更好的类型？
// 问题2：leg_info的定长