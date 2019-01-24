#include <iostream>
#include <Eigen/Dense>

using namespace std;
using namespace Eigen;

auto returnV(){
    MatrixXf m = MatrixXf::Random(5,4);
    cout << "Here is the matrix m:" << endl << m << endl;
    BDCSVD<MatrixXf> svd(m, ComputeThinU | ComputeThinV);
    cout << "Its singular values are:" << endl << svd.singularValues() << endl;
    cout << "Its left singular vectors are the columns of the thin U matrix:" << endl << svd.matrixU() << endl;
    cout << "Its right singular vectors are the columns of the thin V matrix:" << endl << svd.matrixV() << endl;
    //Eigen::Map<const Eigen::Matrix4f> maper(svd.matrixV().data());
    //cout << "map matrix is " << endl << maper << endl;
    return svd.matrixV();//maper;
}

int main(){
    auto p = returnV();
    cout << "in main " << endl << p << endl;
}