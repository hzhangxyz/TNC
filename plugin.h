std::vector<Leg> leg_info = {Phy1,Phy2,Phy3,Phy4};

EIGEN_STRONG_INLINE void set_leg(const std::vector<Leg>& new_leg){
    leg_info = new_leg;
}

EIGEN_STRONG_INLINE void unsafe_set_leg(std::vector<Leg>& new_leg){
    leg_info.swap(new_leg);
}

template<typename OtherDerived, typename Dimensions, int ContractNum> EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
const TensorContractionOp<const Dimensions, const Derived, const OtherDerived, const NoOpOutputKernel>
node_contract(const OtherDerived& other, const Eigen::array<Eigen::IndexPair<Leg>, ContractNum> legs ) const {
    Eigen::array<Eigen::IndexPair<int>, 1> dims = { Eigen::IndexPair<int>(0, 1) };
    return contract(other, dims);
}