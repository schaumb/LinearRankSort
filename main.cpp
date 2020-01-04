#include <iostream>

#include "linearRankSort.hpp"


int main() {
    std::string s = "The quickbrownfxjmpsvtlazydg";
    bxlx::linearRankSort(s.begin(), s.begin() + 8);
    bxlx::linearRankSort(s.begin() + 8, s.begin() + 18);
    bxlx::linearRankSort(s.begin() + 18, s.begin() + 28);
    std::cout << "Sorted parts: " << s.substr(0, 8) << std::endl;
    std::cout << s.substr(0, 8) << std::endl;
    std::cout << s.substr(8, 10) << std::endl;
    std::cout << s.substr(18, 10) << std::endl;
}
