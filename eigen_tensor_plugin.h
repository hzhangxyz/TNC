Eigen::array<Leg, DerivedTraits::NumDimensions> leg_info;// = default_leg_info();

EIGEN_DEVICE_FUNC
EIGEN_STRONG_INLINE void set_leg(const Eigen::array<Leg, DerivedTraits::NumDimensions>& new_leg){
    this->leg_info = new_leg;
}

template<typename OtherDerived, std::size_t ContractNum> EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
EIGEN_DEVICE_FUNC const TensorContractionOp<const Eigen::array<Eigen::IndexPair<Index>, ContractNum>, const Derived, const OtherDerived, const NoOpOutputKernel>
node_contract(const OtherDerived& other,
              const Eigen::array<Leg, ContractNum>& leg1,
              const Eigen::array<Leg, ContractNum>& leg2,
              std::map<Leg,Leg> map1=std::map<Leg,Leg>{},
              std::map<Leg,Leg> map2=std::map<Leg,Leg>{}) const {
    Eigen::array<Index, ContractNum> index1 = this->get_index_from_leg(leg1);
    Eigen::array<Index, ContractNum> index2 = other.get_index_from_leg(leg2);
    Eigen::array<Eigen::IndexPair<Index>, ContractNum> dims;
    for(int i=0;i<ContractNum;i++){
        dims[i].first = index1[i];
        dims[i].second = index2[i];
    }
    auto res = this->contract(other, dims);
    auto i=0;
    for(auto j=0;j<this->leg_info.size();j++){
        auto index = std::find(leg1.begin(), leg1.end(), this->leg_info[j]);
        if(index==leg1.end()){
            auto leg = map1.find(this->leg_info[j]);
            if(leg==map1.end()){
                res.leg_info[i++] = this->leg_info[j];
            }else{
                res.leg_info[i++] = leg->second;
            }
        }
    }
    for(auto j=0;j<other.leg_info.size();j++){
        auto index = std::find(leg2.begin(), leg2.end(), other.leg_info[j]);
        if(index==leg2.end()){
            auto leg = map2.find(other.leg_info[j]);
            if(leg==map2.end()){
                res.leg_info[i++] = other.leg_info[j];
            }else{
                res.leg_info[i++] = leg->second;
            }
        }
    }
    return res;
}

template<std::size_t ContractNum>// 迷，不用unsigned long int 的话template不能匹配
EIGEN_DEVICE_FUNC Eigen::array<Index, ContractNum> get_index_from_leg(const Eigen::array<Leg, ContractNum>& legs) const {
    /* 输入一个leg array, 返回一个对应的index array， 不存在的leg对应-1(!!!) */
    Eigen::array<Index, ContractNum> res;
    for(auto i=0;i<ContractNum;i++){
        auto index = std::find(this->leg_info.begin(), this->leg_info.end(), legs[i]);
        if(index==this->leg_info.end()){
            res[i] = -1;
        }else{
            res[i] = std::distance(this->leg_info.begin(), index);
        }
    }
    return res;
}
