#include "SIREN/math/Quaternion.h"

#include <cmath>
#include <tuple>
#include <cstdlib>
#include <sstream>
#include <utility>
#include <iterator>
#include <algorithm>

#include "SIREN/math/Matrix3D.h"
#include "SIREN/math/Vector3D.h"
#include "SIREN/math/Conversions.h"
#include "SIREN/math/EulerAngles.h"
#include "SIREN/math/EulerQuaternionConversions.h"

using namespace siren::math;

//----------------------------------------------------------------------//
//------------------------- Constructors -------------------------------//
//----------------------------------------------------------------------//

// default constructor
Quaternion::Quaternion() :
    x_(0.0),
    y_(0.0),
    z_(0.0),
    w_(1.0)
{
}

Quaternion::Quaternion(
    double x,
    double y,
    double z,
    double w
) :
    x_(x),
    y_(y),
    z_(z),
    w_(w)
{
}

// copy constructor
Quaternion::Quaternion(const Quaternion & quaternion) :
    x_(quaternion.x_),
    y_(quaternion.y_),
    z_(quaternion.z_),
    w_(quaternion.w_)
{
}

Quaternion::Quaternion(const Vector3D &  vec) :
    x_(vec.GetX()),
    y_(vec.GetY()),
    z_(vec.GetZ()),
    w_(0)
{
}

Quaternion::Quaternion(Quaternion&& other) :
    x_(std::move(other.x_)),
    y_(std::move(other.y_)),
    z_(std::move(other.z_)),
    w_(std::move(other.w_))
{
}

// destructor
Quaternion::~Quaternion() {}

//----------------------------------------------------------------------//
//-----------------------operator functions and swap--------------------//
//----------------------------------------------------------------------//

Quaternion& Quaternion::operator=(Quaternion const & quaternion) {
    if (this != &quaternion)
    {
        Quaternion tmp(quaternion);
        swap(tmp);
    }
    return *this;
}

Quaternion& Quaternion::operator=(Quaternion && other) {
    x_ = std::move(other.x_);
    y_ = std::move(other.y_);
    z_ = std::move(other.z_);
    w_ = std::move(other.w_);
    return *this;
}

Quaternion& Quaternion::operator=(Quaternion const && other) {
    x_ = other.x_;
    y_ = other.y_;
    z_ = other.z_;
    w_ = other.w_;
    return *this;
}

bool Quaternion::operator==(const Quaternion& quaternion) const
{
    return (this == &quaternion) or (
        x_ == quaternion.x_ and
        y_ == quaternion.y_ and
        z_ == quaternion.z_ and
        w_ == quaternion.w_);
}

bool Quaternion::operator!=(const Quaternion& quaternion) const
{
    return !(*this == quaternion);
}

bool Quaternion::operator<(const Quaternion& quaternion) const {
    return (this != &quaternion) and
        std::tie(x_, y_, z_, w_)
        <
        std::tie(quaternion.x_, quaternion.y_, quaternion.z_, quaternion.w_);
}

void Quaternion::swap(Quaternion& quaternion)
{
    using std::swap;

    swap(x_, quaternion.x_);
    swap(y_, quaternion.y_);
    swap(z_, quaternion.z_);
    swap(w_, quaternion.w_);
}

namespace siren {
namespace math {
std::ostream& operator<<(std::ostream& os, Quaternion const& quaternion)
{
    std::stringstream ss;
    ss << "Quaternion (" << &quaternion << ")\n" ;
    ss << quaternion.x_ << "\t" << quaternion.y_ << "\t" << quaternion.z_ << "\t" << quaternion.w_;
    os << ss.str() << '\n';
    return os;
}
} // namespace math
} // namespace siren

Quaternion Quaternion::operator*(Quaternion const & other) const
{
    Quaternion product;
    product.x_ = (other.w_ * x_) + (other.x_ * w_) + (y_ * other.z_) - (z_ * other.y_);
    product.y_ = (other.w_ * y_) + (other.y_ * w_) + (z_ * other.x_) - (x_ * other.z_);
    product.z_ = (other.w_ * z_) + (other.z_ * w_) + (x_ * other.y_) - (y_ * other.x_);
    product.w_ = (other.w_ * w_) - (other.x_ * x_) - (y_ * other.y_) - (z_ * other.z_);
    return product;
}

Quaternion & Quaternion::operator*=(Quaternion const & other)
{
    return (*this = other * (*this));
}

Quaternion Quaternion::operator*(double factor) const
{
    return Quaternion(factor * x_, factor * y_, factor * z_, factor * w_);
}

Quaternion & Quaternion::operator*=(double factor)
{
    x_ *= factor;
    y_ *= factor;
    z_ *= factor;
    w_ *= factor;
    return (*this);
}

Quaternion Quaternion::operator+(Quaternion const & other) const
{
    Quaternion sum;
    sum.x_ = other.x_ + x_;
    sum.y_ = other.y_ + y_;
    sum.z_ = other.z_ + z_;
    sum.w_ = other.w_ + w_;
    return sum;
}

Quaternion & Quaternion::operator+=(Quaternion const & other)
{
    (*this) = other + (*this);
    return (*this);
}

Quaternion Quaternion::operator+(double factor) const
{
    return Quaternion(factor * x_, factor * y_, factor * z_, factor * w_);
}

Quaternion & Quaternion::operator+=(double factor)
{
    x_ += factor;
    y_ += factor;
    z_ += factor;
    w_ += factor;
    return (*this);
}

Quaternion Quaternion::operator~() const {
    return conjugated();
}

Quaternion Quaternion::operator!() const {
    return inverted();
}

Quaternion Quaternion::rotate(Quaternion const & p, bool inv = false) const
{
    double w,x,y,z;

    double w0, x0, y0, z0;

    double norm = magnitude();

    if(inv) {
        w0 = w_ / norm;
        x0 = -x_ / norm;
        y0 = -y_ / norm;
        z0 = -z_ / norm;
    }
    else {
        w0 = w_ / norm;
        x0 = x_ / norm;
        y0 = y_ / norm;
        z0 = z_ / norm;
    }

    double w1 = p.GetW();
    double x1 = p.GetX();
    double y1 = p.GetY();
    double z1 = p.GetZ();

    double xx = x0 * x1, xy = x0 * y1, xz = x0 * z1;
    double yx = y0 * x1, yy = y0 * y1, yz = y0 * z1;
    double zx = z0 * x1, zy = z0 * y1, zz = z0 * z1;

    double w2 = w0 * w0;
    double x2 = x0 * x0;
    double y2 = y0 * y0;
    double z2 = z0 * z0;

    w = w1 * (w2 + x2 + y2 + z2);
    x = x1 * (w2 + x2 - y2 - z2) + 2 * (x0 * (yy + zz) + w0 * (yz - zy));
    y = y1 * (w2 - x2 + y2 - z2) + 2 * (y0 * (xx + zz) + w0 * (zx - xz));
    z = z1 * (w2 - x2 - y2 + z2) + 2 * (z0 * (xx + yy) + w0 * (xy - yx));
    return Quaternion(x, y, z, w);
}

Vector3D Quaternion::rotate(Vector3D const & p, bool inv = false) const
{
    double x, y, z;

    double w0, x0, y0, z0;

    double norm = magnitude();

    if(inv) {
        w0 = w_ / norm;
        x0 = -x_ / norm;
        y0 = -y_ / norm;
        z0 = -z_ / norm;
    }
    else {
        w0 = w_ / norm;
        x0 = x_ / norm;
        y0 = y_ / norm;
        z0 = z_ / norm;
    }

    double x1 = p.GetX();
    double y1 = p.GetY();
    double z1 = p.GetZ();

    double xx = x0 * x1, xy = x0 * y1, xz = x0 * z1;
    double yx = y0 * x1, yy = y0 * y1, yz = y0 * z1;
    double zx = z0 * x1, zy = z0 * y1, zz = z0 * z1;

    double w2 = w0 * w0;
    double x2 = x0 * x0;
    double y2 = y0 * y0;
    double z2 = z0 * z0;

    x = x1 * (w2 + x2 - y2 - z2) + 2 * (x0 * (yy + zz) + w0 * (yz - zy));
    y = y1 * (w2 - x2 + y2 - z2) + 2 * (y0 * (xx + zz) + w0 * (zx - xz));
    z = z1 * (w2 - x2 - y2 + z2) + 2 * (z0 * (xx + yy) + w0 * (xy - yx));
    return Vector3D(x, y, z);
}

void Quaternion::GetMatrix(Matrix3D & dest) const
{
    dest.SetXX(1 - 2 * y_ * y_ - 2 * z_ * z_);
    dest.SetXY(2 * x_ * y_ - 2 * z_ * w_);
    dest.SetXZ(2 * x_ * z_ + 2 * y_ * w_);
    dest.SetYX(2 * x_ * y_ + 2 * z_ * w_);
    dest.SetYY(1 - 2 * x_ * x_ - 2 * z_ * z_);
    dest.SetYZ(2 * z_ * y_ - 2 * x_ * w_);
    dest.SetZX(2 * x_ * z_ - 2 * y_ * w_);
    dest.SetZY(2 * z_ * y_ + 2 * x_ * w_);
    dest.SetZZ(1 - 2 * x_ * x_ - 2 * y_ * y_);
}

Matrix3D Quaternion::GetMatrix() const
{
    Matrix3D mat;
    GetMatrix(mat);
    return mat;
}

void Quaternion::SetMatrix(Matrix3D const & mat) {
    double trace = mat.GetXX() + mat.GetYY() + mat.GetZZ();
    double M = std::max(std::max(mat.GetXX(), mat.GetYY()), std::max(mat.GetZZ(), trace));
    double qmax = 2.0 * std::sqrt(1.0 - trace + 2 * M);

    if(M == mat.GetXX()) {
        x_ = 0.25 * qmax;
        y_ = (mat.GetXY() + mat.GetYX()) / qmax;
        z_ = (mat.GetZX() + mat.GetXZ()) / qmax;
        w_ = (mat.GetZY() - mat.GetYZ()) / qmax;
    } else if(M == mat.GetYY()) {
        x_ = (mat.GetXY() + mat.GetYX()) / qmax;
        y_ = 0.25 * qmax;
        z_ = (mat.GetYZ() + mat.GetZY()) / qmax;
        w_ = (mat.GetXZ() - mat.GetZX()) / qmax;
    } else if(M == mat.GetZZ()) {
        x_ = (mat.GetXZ() + mat.GetZX()) / qmax;
        y_ = (mat.GetYZ() + mat.GetZY()) / qmax;
        z_ = 0.25 * qmax;
        w_ = (mat.GetYX() - mat.GetXY()) / qmax;
    } else {
        x_ = (mat.GetZY() - mat.GetYZ()) / qmax;
        y_ = (mat.GetXZ() - mat.GetZX()) / qmax;
        z_ = (mat.GetYX() - mat.GetXY()) / qmax;
        w_ = 0.25 * qmax;
    }
}

Quaternion & Quaternion::conjugate() {
    // w_ = w_;
    x_ = -x_;
    y_ = -y_;
    z_ = -z_;
    return *this;
}

Quaternion Quaternion::conjugated() const{
    Quaternion res(*this);
    res.conjugate();
    return res;
}

Quaternion & Quaternion::invert() {
    double norm2 = magnitudesq();
    w_ = w_ / norm2;
    x_ = -x_ / norm2;
    y_ = -y_ / norm2;
    z_ = -z_ / norm2;
    return *this;
}

Quaternion Quaternion::inverted() const{
    Quaternion res(*this);
    res.invert();
    return res;
}

void Quaternion::SetPosition(Vector3D const & vec)
{
    w_ = 0.0;
    x_ = vec.GetX();
    y_ = vec.GetY();
    z_ = vec.GetZ();
}

Quaternion & Quaternion::normalize()
{
    double norm = x_ * x_ + y_ * y_ + z_ * z_ + w_ * w_;

    if(norm == 1)
        return *this;

    return (*this *= 1.0 / std::sqrt(norm));
}

Quaternion Quaternion::normalized() const {
    Quaternion res(*this);
    res.normalize();
    return res;
}

double Quaternion::magnitudesq() const
{
    return w_ * w_ + x_ * x_ + y_ * y_ + z_ * z_;
}

double Quaternion::magnitude() const
{
    return std::sqrt(magnitudesq());
}

double Quaternion::DotProduct(Quaternion const & qu) const
{
    return x_ * qu.x_ + y_ * qu.y_ + z_ * qu.z_ + w_ * qu.w_;
}

Quaternion Quaternion::lerp(Quaternion const & q1, Quaternion const & q2, double t)
{
    const double s = 1.0 - t;
    return (q1 * s) + (q2 * t);
}

Quaternion Quaternion::slerp(Quaternion const & q1, Quaternion const & q2, double t) {
    double dotproduct = q1.DotProduct(q2);
	double theta, st, sut, sout, coeff1, coeff2;

	theta = (double) acos(dotproduct);
	if (theta<0.0) theta = -theta;

	st = (double) sin(theta);
	sut = (double) sin(t*theta);
	sout = (double) sin((1.0 - t)*theta);
	coeff1 = sout/st;
	coeff2 = sut/st;

    return (q1 * coeff1 + q2 * coeff2).normalized();
}

void Quaternion::SetAxisAngle(Vector3D const & axis, double angle)
{
    Vector3D dir = axis.normalized();
    double x = angle / 2.0;
    double s = sin(x);

    x_ = s * dir.GetX();
    y_ = s * dir.GetY();
    z_ = s * dir.GetZ();
    w_ = cos(x);
}

void Quaternion::GetAxisAngle(Vector3D & axis, double & angle) const
{
    double scale = sqrt(x_ * x_ + y_ * y_ + z_ * z_);
    if(scale == 0 or w_ > 1.0 or w_ < -1.0) {
        angle = 0;
        axis.SetCartesianCoordinates(0, 0, 1);
    } else {
        angle = std::atan2(scale, w_) * 2.0;
        axis.SetCartesianCoordinates(x_ / scale, y_ / scale, z_ / scale);
    }
}

std::tuple<Vector3D, double> Quaternion::GetAxisAngle() const
{
    std::tuple<Vector3D, double> result;
    GetAxisAngle(std::get<0>(result), std::get<1>(result));
    return result;
}

void Quaternion::SetEulerAngles(EulerAngles const & euler)
{
    (*this) = siren::math::QuaternionFromEulerAngles(euler);
}

void Quaternion::GetEulerAngles(EulerAngles & euler, EulerOrder order) const
{
    euler = siren::math::EulerAnglesFromQuaternion(*this, order);
}

void Quaternion::GetEulerAnglesZXZr(double & alpha, double & beta, double & gamma) const
{
    EulerAngles euler = siren::math::ZXZrFromQ(*this);
    alpha = euler.GetAlpha();
    beta = euler.GetBeta();
    gamma = euler.GetGamma();
}

void Quaternion::SetEulerAnglesZXZr(double alpha, double beta, double gamma)
{
    (*this) = siren::math::QFromZXZr(alpha, beta, gamma);
}

void Quaternion::GetEulerAnglesXYZs(double & alpha, double & beta, double & gamma) const
{
    EulerAngles euler = siren::math::XYZsFromQ(*this);
    alpha = euler.GetAlpha();
    beta = euler.GetBeta();
    gamma = euler.GetGamma();
}

void Quaternion::SetEulerAnglesXYZs(double alpha, double beta, double gamma)
{
    (*this) = siren::math::QFromXYZs(alpha, beta, gamma);
}

Quaternion siren::math::rotation_between(Vector3D const & v0, Vector3D const & v1) {
    Vector3D dir0 = v0.normalized();
    Vector3D dir1 = v1.normalized();
    double dot = scalar_product(dir0, dir1);
    if(dot == -1.0) {
        // Special case for 180deg rotation
        // Find most perpendicular axis vector
        Vector3D vecs[3] = {
            Vector3D(1,0,0),
            Vector3D(0,1,0),
            Vector3D(0,0,1)
        };
        double dots[3] = {
            std::abs(scalar_product(vecs[0], dir0)),
            std::abs(scalar_product(vecs[1], dir0)),
            std::abs(scalar_product(vecs[2], dir0))
        };
        // Find axis vector most perpendicular to original direction
        size_t idx = std::distance(&(dots), std::min_element(&(dots), &(dots)+3));
        // Initialize quaternion with unit vector perpendicular to original direction and selected axis
        return Quaternion(cross_product(vecs[idx], dir0).normalized());
    } else {
        Vector3D cross = cross_product(dir0, dir1);
        Quaternion rot(cross);
        rot.SetW(1.0 + dot);
        rot.normalize();
        return rot;
    }
}

