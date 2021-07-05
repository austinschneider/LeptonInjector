#ifndef LI_Quaternion_H
#define LI_Quaternion_H

#include <sstream>
#include "earthmodel-service/Vector3D.h"
#include "earthmodel-service/EulerAngles.h"

namespace earthmodel {

class Vector3D;

class Quaternion
{
public:
    // constructors
    Quaternion();
    Quaternion(const double x, const double y, const double z, const double w);
    Quaternion(const Quaternion& quaternion);
    Quaternion(Quaternion&& other);
    ~Quaternion();

    void SetPosition(Vector3D const & vec);

    void GetMatrix(Matrix3D &) const;
    Matrix3D GetMatrix() const;
    void SetMatrix();

    Quaternion & invert();
    Quaternion & normalize();

    double DotProduct(Quaternion const &) const;

    static Quaternion lerp(Quaternion const &, Quaternion const &, double);
    static Quaternion slerp(Quaternion const &, Quaternion const &, double);

    void SetAxisAngle(Vector3D const &, double);
    void GetAxisAngle(Vector3D &, double &) const;
    std::tuple<Vector3D, double> GetAxisAngle() const;

    void SetAngles(EulerAngles const & euler);
    void GetAnglesEulerZXZ(double & alpha, double & beta, double & gamma) const;
    void SetAnglesEulerZXZ(double alpha, double beta, double gamma);
    void GetAnglesTaitBryanZXY(double & yaw, double & pitch, double & roll) const;
    void SetAnglesTaitBryanZXY(double yaw, double pitch, double roll);
    void GetAnglesTaitBryanZYX(double & yaw, double & pitch, double & roll) const;
    void SetAnglesTaitBryanZYX(double yaw, double pitch, double roll);

    //-------------------------------------//
    // operator functions and swap
    Quaternion& operator=(Quaternion const & quaternion);
    Quaternion& operator=(Quaternion const && quaternion);
    Quaternion& operator=(Quaternion && quaternion);
    bool operator==(const Quaternion& quaternion) const;
    bool operator!=(const Quaternion& quaternion) const;
    void swap(Quaternion& quaternion);
    friend std::ostream& operator<<(std::ostream& os, Quaternion const& quaternion);

    Quaternion operator*(Quaternion const &) const;
    Quaternion & operator*=(Quaternion const &);
    Quaternion operator*(double) const;
    Quaternion & operator*=(double);
    Quaternion operator+(Quaternion const &) const;
    Quaternion & operator+=(Quaternion const &);
    Quaternion operator+(double) const;
    Quaternion & operator+=(double);

    double GetX() const {return x_;}
    double GetY() const {return y_;}
    double GetZ() const {return z_;}
    double GetW() const {return w_;}
    void GetX(double x) {x_ = x;}
    void GetY(double y) {y_ = y;}
    void GetZ(double z) {z_ = z;}
    void GetW(double w) {w_ = w;}
private:
    double x_;
    double y_;
    double z_;
    double w_;
};

} // namespace earthmodel

#endif // LI_Quaternion_H

