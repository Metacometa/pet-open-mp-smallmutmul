#include <iostream>
#include <omp.h>
#include <chrono>
#include <vector>
#include <time.h>
#include <cassert>

struct Measurer {
    std::chrono::system_clock::time_point begin;

    Measurer() {
        begin = std::chrono::system_clock::now();
    }

    void Measure() {
        auto end = std::chrono::system_clock::now();

        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);

        std::cout << "The time: " << elapsed_ms.count() << " ms" << std::endl;
    }
};

struct Matrix {
    Matrix(const int&m, const int&n) {
        this->m = m;
        this->n = n;

        for (size_t i = 0; i < m; ++i) {
            matrix.emplace_back(std::vector<int>(n));
            for (auto& el : matrix[i]) {
                el = rand() % 10;
            }
        }
    }

    Matrix(const int&m, const int&n, const int&filler) {
        this->m = m;
        this->n = n;

        for (size_t i = 0; i < m; ++i) {
            matrix.emplace_back(std::vector<int>(n));
            for (auto& el : matrix[i]) {
                el = filler;
            }
        }
    }

    void print() {
        std::cout << "Size: " << m << "x" << n << std::endl;
        
        for (size_t i = 0; i < m; ++i) {
            matrix.emplace_back(std::vector<int>(n));
            for (auto& el : matrix[i]) {
                std::cout << el << " ";
            }   

            std::cout << std::endl;
        }

        std::cout << std::endl;
    }

    Matrix operator*(const Matrix&rhs) {
        assert(this->n == rhs.m);

        Matrix multiplied{this->m, rhs.n, 0};

        for (int row = 0; row < multiplied.m; row++) {
            for (int column = 0; column < multiplied.n; column++) {
                for (int k = 0; k < this->n; k++) {
                    multiplied(row, column) += matrix[row][k] * rhs(k, column);
                }
            }
        }

        return multiplied;
    }

    Matrix mutmul(const Matrix&rhs) {
        assert(this->n == rhs.m);

        Matrix multiplied{this->m, rhs.n, 0};

        //#pragma omp for 
        for (int row = 0; row < multiplied.m; row++) {
            //#pragma omp for 
            for (int column = 0; column < multiplied.n; column++) {
                int local_mult = 0;

                #pragma omp parallel reduction (+:sum) 
                {
                    #pragma omp for 
                    for (int k = 0; k < this->n; k++) {
                        local_mult += matrix[row][k] * rhs(k, column);
                    }
                }

                multiplied(row, column) = local_mult;
            }
        }


        return multiplied;
    }
    
    const int& operator()(int x, int y) const {
        assert(x >= 0 && x < m);
        assert(y >= 0 && y < n);

        return matrix[x][y];
    }
    int& operator()(int x, int y) {
        assert(x >= 0 && x < m);
        assert(y >= 0 && y < n);

        return matrix[x][y];
    }

private: 
    std::vector<std::vector<int>> matrix;
    int m = 0;
    int n = 0;
};

int main() {
    srand(time(0));

    Measurer measurer1;

    Matrix mm1{100, 100};
    Matrix mm2{100, 100};

    Matrix mm3 = mm1 * mm2;

    measurer1.Measure();


    Measurer measurer2;

    Matrix mm4 = mm1.mutmul(mm2);
    measurer2.Measure();
}