#include <iostream>
#include "utils/rmMatrix.h"

int main(int, char**) {
    rm::mat_t xorTT = { 0.1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0 };
    xorTT = xorTT.resize(4);

    xorTT.print();
    xorTT = xorTT * xorTT.transpose();

    xorTT.print();


    
    return 0;
}
