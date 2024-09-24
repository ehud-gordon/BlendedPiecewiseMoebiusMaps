#include "BPM/Mobius.h"

#include <Eigen/Core>


Mat2c ComputeMobiusCoefficients(const std::array<Complex, 3>& z, const std::array<Complex, 3>& w) {
    // Create matrices matA, matB, matC, and matD
    Matrix3c matA;
    Matrix3c matB;
    Matrix3c matC;
    Matrix3c matD;

    // Fill the matrices with the corresponding values
    matA << z[0] * w[0], w[0], Complex(1, 0),
            z[1] * w[1], w[1], Complex(1, 0),
            z[2] * w[2], w[2], Complex(1, 0);

    matB << z[0] * w[0], z[0], w[0],
            z[1] * w[1], z[1], w[1],
            z[2] * w[2], z[2], w[2];

    matC << z[0], w[0], Complex(1, 0),
            z[1], w[1], Complex(1, 0),
            z[2], w[2], Complex(1, 0);

    matD << z[0] * w[0], z[0], Complex(1, 0),
            z[1] * w[1], z[1], Complex(1, 0),
            z[2] * w[2], z[2], Complex(1, 0);

    // Compute the determinants
    Complex a = matA.determinant();
    Complex b = matB.determinant();
    Complex c = matC.determinant();
    Complex d = matD.determinant();    

    // Normalize the coefficients
    Complex norm_factor = std::sqrt(a * d - b * c);
    a /= norm_factor;
    b /= norm_factor;
    c /= norm_factor;
    d /= norm_factor;
    
    // convert the coefficients to Mat2c struct
    Mat2c coefficients;
    coefficients.a = cvec(a.real(), a.imag());
    coefficients.b = cvec(b.real(), b.imag());
    coefficients.c = cvec(c.real(), c.imag());
    coefficients.d = cvec(d.real(), d.imag());
    return coefficients;
}

void PrintMat2c(Mat2c m) {
    std::cout << "a: (" << m.a.x << ", " << m.a.y << ")\n"
              << "b: (" << m.b.x << ", " << m.b.y << ")\n"
              << "c: (" << m.c.x << ", " << m.c.y << ")\n"
              << "d: (" << m.d.x << ", " << m.d.y << ")\n\n";
}


Matrix2c ComputeMobiusCoefficients_Eigen(const std::array<Complex, 3>& z, const std::array<Complex, 3>& w) {
    // Create matrices matA, matB, matC, and matD
    Matrix3c matA;
    Matrix3c matB;
    Matrix3c matC;
    Matrix3c matD;

    // Fill the matrices with the corresponding values
    matA << z[0] * w[0], w[0], Complex(1, 0),
            z[1] * w[1], w[1], Complex(1, 0),
            z[2] * w[2], w[2], Complex(1, 0);

    matB << z[0] * w[0], z[0], w[0],
            z[1] * w[1], z[1], w[1],
            z[2] * w[2], z[2], w[2];

    matC << z[0], w[0], Complex(1, 0),
            z[1], w[1], Complex(1, 0),
            z[2], w[2], Complex(1, 0);

    matD << z[0] * w[0], z[0], Complex(1, 0),
            z[1] * w[1], z[1], Complex(1, 0),
            z[2] * w[2], z[2], Complex(1, 0);

    // Compute the determinants
    Complex a = matA.determinant();
    Complex b = matB.determinant();
    Complex c = matC.determinant();
    Complex d = matD.determinant();    

    // Normalize the coefficients
    Complex norm_factor = std::sqrt(a * d - b * c);
    a /= norm_factor;
    b /= norm_factor;
    c /= norm_factor;
    d /= norm_factor;

    // Return the coefficients as a 2x2 complex matrix
    Matrix2c coefficients;
    coefficients << a, b,
                    c, d;

    return coefficients;
}

Complex ApplyMobius(const Matrix2c& mobius, const Complex& z) {
    Complex num = mobius(0, 0) * z + mobius(0, 1);
    Complex den = mobius(1, 0) * z + mobius(1, 1);
    return num / den;
}

cvec ApplyMobius(const Matrix2c& mobius, const cvec& z) {
    Complex z_complex = Complex(z.x, z.y);
    Complex result = ApplyMobius(mobius, z_complex);
    return cvec(result.real(), result.imag());
}
