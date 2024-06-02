#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>

int main() {
    // Gọi tập lệnh Python để sinh dữ liệu trong outputpy.txt
    system("python ../../python/python2.py");

    // Đọc dữ liệu từ outputpy.txt
    int p[6] = {};
    std::ifstream inputFile("../../../data/outputpy.txt");
    std::string line;
    if (std::getline(inputFile, line)) {
        std::stringstream ss(line);
        int value;
        int i = 0;
        while (ss >> value && i < 6) {
            p[i] = value;
            i++;
        }
    }
    inputFile.close();
    // Hiển thị mảng p để kiểm tra
    for (int i = 0; i < 6; ++i) {
        std::cout << "p[" << i << "] = " << p[i] << std::endl;
    }
    return 0;
}
