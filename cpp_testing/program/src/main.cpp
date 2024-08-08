#include <iostream>
#include "my_library.h"
#include "matplotlibcpp.h"

namespace plt = matplotlibcpp;

int main() {
    std::vector<double> x = {1, 2, 3, 4, 5};
    std::vector<double> y = {1, 4, 9, 16, 25};

    plt::plot(x, y);
    plt::xlabel("x-axis");
    plt::ylabel("y-axis");
    plt::title("Simple Plot");
    plt::show();

    return 0;
}