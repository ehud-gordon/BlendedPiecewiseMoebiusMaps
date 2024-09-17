#include <array>
#include <complex>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Eigen/Dense>
#include <unsupported/Eigen/MatrixFunctions>
#include <cmath>

using Complex = std::complex<float>;
using Matrix2c = Eigen::Matrix2cf;
using Matrix3c = Eigen::Matrix3cf;
using Vector2c = Eigen::Vector2cf;
using cvec = glm::vec2;
using vec2 = glm::vec2;
using vec3 = glm::vec3;

struct Mat2c {
    cvec a, b, c, d;

    Mat2c() {
        a = cvec(1.0, 0.0);
        b = cvec(0.0, 0.0);
        c = cvec(0.0, 0.0);
        d = cvec(1.0, 0.0);
    }

    // Constructor to initialize all members
    Mat2c(const cvec& a, const cvec& b, const cvec& c, const cvec& d) : a(a), b(b), c(c), d(d) {}

    Mat2c(const Matrix2c& eigen_mat) {
        a = cvec(eigen_mat(0, 0).real(), eigen_mat(0, 0).imag());
        b = cvec(eigen_mat(0, 1).real(), eigen_mat(0, 1).imag());
        c = cvec(eigen_mat(1, 0).real(), eigen_mat(1, 0).imag());
        d = cvec(eigen_mat(1, 1).real(), eigen_mat(1, 1).imag());
    }

    Mat2c(float val) {
        a = cvec(val, 0.0);
        b = cvec(0.0, 0.0);
        c = cvec(0.0, 0.0);
        d = cvec(val, 0.0);
    }

    // Function to update members from a Matrix2c
    void updateFromEigen(const Matrix2c& eigen_mat) {
        a = cvec(eigen_mat(0, 0).real(), eigen_mat(0, 0).imag());
        b = cvec(eigen_mat(0, 1).real(), eigen_mat(0, 1).imag());
        c = cvec(eigen_mat(1, 0).real(), eigen_mat(1, 0).imag());
        d = cvec(eigen_mat(1, 1).real(), eigen_mat(1, 1).imag());
    }

    Matrix2c ToEigenMatrix() const {
        Matrix2c mat;
        mat << Complex(a.x, a.y), Complex(b.x, b.y),
               Complex(c.x, c.y), Complex(d.x, d.y);
        return mat;
    }

   friend std::ostream& operator<<(std::ostream& os, const Mat2c& mc) {
        os << "a: (" << mc.a.x << ", " << mc.a.y << ")\n"
           << "b: (" << mc.b.x << ", " << mc.b.y << ")\n"
           << "c: (" << mc.c.x << ", " << mc.c.y << ")\n"
           << "d: (" << mc.d.x << ", " << mc.d.y << ")\n";
        return os;
    }

    // Multiplication by a scalar (element-wise)
    template <typename T>
    Mat2c operator*(T scalar) const {
        return Mat2c{
            a * scalar,
            b * scalar,
            c * scalar,
            d * scalar
        };
    }

    // Addition of another Mat2c (element-wise)
    Mat2c operator+(const Mat2c& other) const {
        return Mat2c{
            a + other.a,
            b + other.b,
            c + other.c,
            d + other.d
        };
    }
    
    Mat2c Inv() {
        Mat2c inv;
        inv.a = d;
        inv.b = -b;
        inv.c = -c;
        inv.d = a;
        return inv;
    }

    Mat2c LogRatio(Mat2c& m) {
        Matrix2c m_eigen = m.ToEigenMatrix();
        return LogRatio(m_eigen);
    }
    
    Mat2c LogRatio(Matrix2c& m_eigen) {
        Mat2c inv = this->Inv();
        Matrix2c inv_eigen = inv.ToEigenMatrix();
        Matrix2c delta = m_eigen * inv_eigen;
        // check Frobenius sign using
        if (delta(0,0).real() + delta(1,1).real() < 0) {
            delta = -delta;
        }
        Matrix2c delta_log = delta.log();
        Mat2c result(delta_log);
        return result;
    }

    void SetCloseToZero(float epsilon = 1e-5f) {
        auto set_component_zero = [epsilon](cvec& vec) {
            if (std::abs(vec.x) < epsilon) {
                vec.x = 0.0f;
            }
            if (std::abs(vec.y) < epsilon) {
                vec.y = 0.0f;
            }
        };

        set_component_zero(a);
        set_component_zero(b);
        set_component_zero(c);
        set_component_zero(d);
    }

};

void PrintMat2c(Mat2c m);

Mat2c ComputeMobiusCoefficients(const std::array<Complex, 3>& z, const std::array<Complex, 3>& w);

Matrix2c ComputeMobiusCoefficients_Eigen(const std::array<Complex, 3>& z, const std::array<Complex, 3>& w);

Complex ApplyMobius(const Matrix2c& mobius, const Complex& z);

cvec ApplyMobius(const Matrix2c& mobius, const cvec& z);

void SetCloseToZero(Matrix2c& matrix, float epsilon = 1e-10f);