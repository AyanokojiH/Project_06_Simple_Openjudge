#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <bits/stdc++.h>


void createDirectory(const std::string& dir_name) {
    struct stat info;
    if (stat(dir_name.c_str(), &info) != 0) {
        std::cout << "Creating directory: " << dir_name << std::endl;
        int res = mkdir(dir_name.c_str(), 0777);
        if (res != 0) {
            std::cerr << "Error creating directory!" << std::endl;
            exit(1);
        }
    } else if (info.st_mode & S_IFDIR) {
        std::cout << "Directory already exists." << std::endl;
    } else {
        std::cerr << "Path exists but is not a directory!" << std::endl;
        exit(1);
    }
}

void writeFile(const std::string& file_name, int n) {
    std::ofstream file(file_name);
    if (!file) {
        std::cerr << "Error opening file: " << file_name << std::endl;
        return;
    }
    file<< n <<std::endl;
    std::vector<int> numbers(n);
    for (int i = 0; i < n; ++i) {
        numbers[i] = i + 1;
    }

    // 打乱顺序
    std::random_shuffle(numbers.begin(), numbers.end());

    // 写入文件
    for (int num : numbers) {
        file << num << " ";
    }

    file.close();
}

int main() {
    srand(static_cast<unsigned int>(time(0)));
    int filename;
    std::cout << "Enter the question.no: ";
    std::cin >> filename;
    std::string dir_name = "info" + std::__cxx11::to_string(filename)+"/std_input";
    createDirectory(dir_name);

    if (chdir(dir_name.c_str()) != 0) {
        std::cerr << "Error changing directory!" << std::endl;
        return 1;
    }

    int lower_bound, upper_bound;
    std::cout << "Enter the lower bound: ";
    std::cin >> lower_bound;
    std::cout << "Enter the upper bound: ";
    std::cin >> upper_bound;

    // 每个文件写入的随机数数量
    int num_random_numbers[] = {5, 12, 18, 1044, 1300, 10000, 17708, 19500, 88888, 99999};
    int num_random_k[] = {2, 3, 4, 10, 56, 100, 1444, 1000, 11111, 22222};

    for (int i = 0; i < 10; ++i) {
        int n = 1 + rand() % 100;
        std::string file_name = "stdin" + std::to_string(i + 1) + ".txt";
        writeFile(file_name, n);
    }

    return 0;
}
