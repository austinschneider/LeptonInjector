
#include <cmath>
#include <random>
#include <iostream>

#include <gtest/gtest.h>

#include "earthmodel-service/Matrix3D.h"
#include "earthmodel-service/Vector3D.h"
#include "earthmodel-service/Quaternion.h"

using namespace earthmodel;

std::mt19937 rng_;
std::uniform_real_distribution<double> uniform_distribution(0.0, 1.0);

double RandomDouble() {
    return uniform_distribution(rng_);
}

TEST(Constructor, Default)
{
    ASSERT_NO_THROW(Quaternion());
}

TEST(Constructor, Coordinates)
{
    ASSERT_NO_THROW(Quaternion(0.0, 0.0, 0.0, 0.0));
    ASSERT_NO_THROW(Quaternion(1.0, 1.0, 1.0, 1.0));
    ASSERT_NO_THROW(Quaternion(-1.0, -1.0, -1.0, -1.0));
}

TEST(Constructor, ConstRef)
{
    Quaternion const A;
    ASSERT_NO_THROW(Quaternion(A));
}

TEST(Constructor, RRef)
{
    Quaternion const A;
    ASSERT_NO_THROW(Quaternion(Quaterion()));
}

TEST(Constructor, DefaultCheckWXYZ)
{
    Quaternion A;
    EXPECT_DOUBLE_EQ(1.0, A.GetW());
    EXPECT_DOUBLE_EQ(0.0, A.GetX());
    EXPECT_DOUBLE_EQ(0.0, A.GetY());
    EXPECT_DOUBLE_EQ(0.0, A.GetZ());
}

TEST(Constructor, CoordinatesCheckWXYZ)
{
    double w = 1.0;
    double x = 2.0;
    double y = 3.0;
    double z = 4.0;
    Quaternion A(x, y, z, w);
    EXPECT_DOUBLE_EQ(w, A.GetW());
    EXPECT_DOUBLE_EQ(x, A.GetX());
    EXPECT_DOUBLE_EQ(y, A.GetY());
    EXPECT_DOUBLE_EQ(z, A.GetZ());
}

TEST(Constructor, ConstRefCheckWXYZ)
{
    double w = 1.0;
    double x = 2.0;
    double y = 3.0;
    double z = 4.0;
    Quaternion const A(x, y, z, w);
    Quaternion B(A);
    EXPECT_DOUBLE_EQ(w, B.GetW());
    EXPECT_DOUBLE_EQ(x, B.GetX());
    EXPECT_DOUBLE_EQ(y, B.GetY());
    EXPECT_DOUBLE_EQ(z, B.GetZ());
}

TEST(Constructor, RRefCheckWXYZ)
{
    double w = 1.0;
    double x = 2.0;
    double y = 3.0;
    double z = 4.0;
    Quaternion A(Quaternion(x, y, z, w));
    EXPECT_DOUBLE_EQ(w, A.GetW());
    EXPECT_DOUBLE_EQ(x, A.GetX());
    EXPECT_DOUBLE_EQ(y, A.GetY());
    EXPECT_DOUBLE_EQ(z, A.GetZ());
}

TEST(SetGet, CheckWXYZ)
{
    double w = 1.0;
    double x = 2.0;
    double y = 3.0;
    double z = 4.0;
    Quaternion A;
    A.SetX(x);
    A.SetY(y);
    A.SetZ(z);
    A.SetW(w);
    EXPECT_DOUBLE_EQ(w, A.GetW());
    EXPECT_DOUBLE_EQ(x, A.GetX());
    EXPECT_DOUBLE_EQ(y, A.GetY());
    EXPECT_DOUBLE_EQ(z, A.GetZ());
}

TEST(SetGet, SetPosition)
{
    double w = 0.0;
    double x = 2.0;
    double y = 3.0;
    double z = 4.0;
    Vector3D vec(x, y, z);
    Quaternion A;
    A.SetPosition(vec);
    EXPECT_DOUBLE_EQ(w, A.GetW());
    EXPECT_DOUBLE_EQ(x, A.GetX());
    EXPECT_DOUBLE_EQ(y, A.GetY());
    EXPECT_DOUBLE_EQ(z, A.GetZ());
}

TEST(Matrix, IdentityRoundtrip)
{
    Quaternion A;
    Matrix3D B;
    B = A.GetMatrix();
    EXPECT_DOUBLE_EQ(1.0, B.GetXX());
    EXPECT_DOUBLE_EQ(0.0, B.GetXY());
    EXPECT_DOUBLE_EQ(0.0, B.GetXZ());
    EXPECT_DOUBLE_EQ(0.0, B.GetYX());
    EXPECT_DOUBLE_EQ(1.0, B.GetYY());
    EXPECT_DOUBLE_EQ(0.0, B.GetYZ());
    EXPECT_DOUBLE_EQ(0.0, B.GetZX());
    EXPECT_DOUBLE_EQ(0.0, B.GetZY());
    EXPECT_DOUBLE_EQ(1.0, B.GetZZ());

    A.SetMatrix(B);

    EXPECT_DOUBLE_EQ(1.0, A.GetW());
    EXPECT_DOUBLE_EQ(0.0, A.GetX());
    EXPECT_DOUBLE_EQ(0.0, A.GetY());
    EXPECT_DOUBLE_EQ(0.0, A.GetZ());
}

TEST(Quaternion, Magnitude)
{
    unsigned int n_rand = 100;
    for(unsigned int i=0; i<n_rand; ++i) {
        double w = RandomDouble() * 20 - 10;
        double x = RandomDouble() * 20 - 10;
        double y = RandomDouble() * 20 - 10;
        double z = RandomDouble() * 20 - 10;
        Quaternion A(x, y, z, w);
        double norm = A.magnitude();
        EXPECT_DOUBLE_EQ(std::sqrt(w*w + x*x + y*y + z*z), norm);

        Quaternion B = A.normalized();
        EXPECT_DOUBLE_EQ(1.0, B.magnitude());

        A.normalize();
        EXPECT_DOUBLE_EQ(1.0, A.magnitude());
    }
}

TEST(Quaternion, Invert)
{
    unsigned int n_rand = 100;
    for(unsigned int i=0; i<n_rand; ++i) {
        double w = RandomDouble() * 20 - 10;
        double x = RandomDouble() * 20 - 10;
        double y = RandomDouble() * 20 - 10;
        double z = RandomDouble() * 20 - 10;
        double norm2 = w*w + x*x + y*y + z*z;
        Quaternion A(x, y, z, w);

        Quaternion B = A.inverted();
        EXPECT_DOUBLE_EQ(w/norm2, B.GetW());
        EXPECT_DOUBLE_EQ(-x/norm2, B.GetX());
        EXPECT_DOUBLE_EQ(-y/norm2, B.GetY());
        EXPECT_DOUBLE_EQ(-z/norm2, B.GetZ());

        Quaternion C = A*B;
        EXPECT_NEAR(1.0, C.GetW(), 1e-12);
        EXPECT_NEAR(0.0, C.GetX(), 1e-12);
        EXPECT_NEAR(0.0, C.GetY(), 1e-12);
        EXPECT_NEAR(0.0, C.GetZ(), 1e-12);

        A.invert();
        EXPECT_DOUBLE_EQ(w/norm2, A.GetW());
        EXPECT_DOUBLE_EQ(-x/norm2, A.GetX());
        EXPECT_DOUBLE_EQ(-y/norm2, A.GetY());
        EXPECT_DOUBLE_EQ(-z/norm2, A.GetZ());
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

