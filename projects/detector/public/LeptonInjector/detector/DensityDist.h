/******************************************************************************
 *                                                                            *
 * This file is part of the simulation tool PROPOSAL.                         *
 *                                                                            *
 * Copyright (C) 2017 TU Dortmund University, Department of Physics,          *
 *                    Chair Experimental Physics 5b                           *
 *                                                                            *
 * This software may be modified and distributed under the terms of a         *
 * modified GNU Lesser General Public Licence version 3 (LGPL),               *
 * copied verbatim in the file "LICENSE".                                     *
 *                                                                            *
 * Modifcations to the LGPL License:                                          *
 *                                                                            *
 *      1. The user shall acknowledge the use of PROPOSAL by citing the       *
 *         following reference:                                               *
 *                                                                            *
 *         J.H. Koehne et al.  Comput.Phys.Commun. 184 (2013) 2070-2090 DOI:  *
 *         10.1016/j.cpc.2013.04.001                                          *
 *                                                                            *
 *      2. The user should report any bugs/errors or improvments to the       *
 *         current maintainer of PROPOSAL or open an issue on the             *
 *         GitHub webpage                                                     *
 *                                                                            *
 *         "https://github.com/tudo-astroparticlephysics/PROPOSAL"            *
 *                                                                            *
 ******************************************************************************/

#pragma once
#ifndef LI_DensityDist_H
#define LI_DensityDist_H
#include <memory>
#include <string>
#include <exception>
#include <functional>

#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/base_class.hpp>

#include "LeptonInjector/math/Vector3D.h"
#include "LeptonInjector/math/Polynomial.h"
#include "LeptonInjector/detector/EarthModelCalculator.h"

namespace LI {
namespace detector {

using LI::math::Vector3D;
using LI::math::Polynom;

class DensityException : public std::exception {
   public:
    DensityException(const char* m) : message_(m){};
    const char* what() const throw() { return message_.c_str(); };

   private:
    std::string message_;
};

class DensityDistribution {
friend cereal::access;
public:
    DensityDistribution();
    virtual ~DensityDistribution() = default;
    DensityDistribution(const DensityDistribution&);

    bool operator==(const DensityDistribution& dens_distr) const;
    bool operator!=(const DensityDistribution& dens_distr) const;
    virtual bool compare(const DensityDistribution& dens_distr) const = 0;


    virtual DensityDistribution* clone() const = 0;
    virtual std::shared_ptr<const DensityDistribution> create() const = 0;

    virtual double Derivative(const Vector3D& xi,
                              const Vector3D& direction) const = 0;
    virtual double AntiDerivative(const Vector3D& xi,
                                  const Vector3D& direction) const = 0;
    virtual double Integral(const Vector3D& xi,
                            const Vector3D& direction,
                            double distance) const = 0;
    virtual double Integral(const Vector3D& xi,
                            const Vector3D& xj) const = 0;
    virtual double InverseIntegral(const Vector3D& xi,
                                   const Vector3D& direction,
                                   double integral,
                                   double max_distance) const = 0;
    virtual double Evaluate(const Vector3D& xi) const = 0;

    template<class Archive>
    void serialize(Archive & archive, std::uint32_t const version) {};
};

class Distribution1D {
friend cereal::access;
public:
    virtual ~Distribution1D() = default;
    bool operator==(const Distribution1D& dist) const;
    bool operator!=(const Distribution1D& dist) const;
    virtual bool compare(const Distribution1D& dist) const = 0;
    virtual Distribution1D* clone() const = 0;
    virtual std::shared_ptr<const Distribution1D> create() const = 0;
    virtual double Derivative(double x) const = 0;
    virtual double AntiDerivative(double x) const = 0;
    virtual double Evaluate(double x) const = 0;

    template<class Archive>
    void serialize(Archive & archive, std::uint32_t const version) {};
};

class ConstantDistribution1D : public Distribution1D {
public:
    ConstantDistribution1D();
    ConstantDistribution1D(const ConstantDistribution1D&);
    ConstantDistribution1D(double val);
    bool compare(const Distribution1D& dist) const override;
    Distribution1D* clone() const override { return new ConstantDistribution1D(*this); };
    std::shared_ptr<const Distribution1D> create() const override {
        return std::shared_ptr<const Distribution1D>(new ConstantDistribution1D(*this));
    };
    double Derivative(double x) const override;
    double AntiDerivative(double x) const override;
    double Evaluate(double x) const override;
    template<class Archive>
    void serialize(Archive & archive, std::uint32_t const version) {
        if(version == 0) {
            archive(::cereal::make_nvp("Value", val_));
            archive(cereal::virtual_base_class<Distribution1D>(this));
        } else {
            throw std::runtime_error("ConstantDistribution1D only supports version <= 0");
        }
    };
protected:
    double val_;
};

class PolynomialDistribution1D : public Distribution1D {
friend cereal::access;
protected:
public:
    PolynomialDistribution1D();
    PolynomialDistribution1D(const PolynomialDistribution1D&);
    PolynomialDistribution1D(const Polynom&);
    PolynomialDistribution1D(const std::vector<double>&);
    bool compare(const Distribution1D& dist) const override;
    Distribution1D* clone() const override { return new PolynomialDistribution1D(*this); };
    std::shared_ptr<const Distribution1D> create() const override {
        return std::shared_ptr<const Distribution1D>(new PolynomialDistribution1D(*this));
    };
    double Derivative(double x) const override;
    double AntiDerivative(double x) const override;
    double Evaluate(double x) const override;
    template<class Archive>
    void serialize(Archive & archive, std::uint32_t const version) {
        if(version == 0) {
            archive(::cereal::make_nvp("Polynomial", polynom_));
            archive(::cereal::make_nvp("PolynomialIntegral", Ipolynom_));
            archive(::cereal::make_nvp("PolynomialDerivative", dpolynom_));
            archive(cereal::virtual_base_class<Distribution1D>(this));
        } else {
            throw std::runtime_error("PolynomialDistribution1D only supports version <= 0");
        }
    };
protected:
    Polynom polynom_;
    Polynom Ipolynom_;
    Polynom dpolynom_;
};

class ExponentialDistribution1D : public Distribution1D {
friend cereal::access;
public:
    ExponentialDistribution1D();
    ExponentialDistribution1D(const ExponentialDistribution1D&);
    ExponentialDistribution1D(double sigma);
    bool compare(const Distribution1D& dist) const override;
    Distribution1D* clone() const override { return new ExponentialDistribution1D(*this); };
    std::shared_ptr<const Distribution1D> create() const override {
        return std::shared_ptr<const Distribution1D>(new ExponentialDistribution1D(*this));
    };
    double Derivative(double x) const override;
    double AntiDerivative(double x) const override;
    double Evaluate(double x) const override;
    template<class Archive>
    void serialize(Archive & archive, std::uint32_t const version) {
        if(version == 0) {
            archive(::cereal::make_nvp("Sigma", sigma_));
            archive(cereal::virtual_base_class<Distribution1D>(this));
        } else {
            throw std::runtime_error("ExponentialDistribution1D only supports version <= 0");
        }
    };
protected:
    double sigma_;
};

class Axis1D {
friend cereal::access;
public:
    Axis1D();
    Axis1D(const Vector3D& fAxis, const Vector3D& fp0);
    Axis1D(const Axis1D&);

    virtual ~Axis1D() {};

    bool operator==(const Axis1D& axis) const;
    bool operator!=(const Axis1D& axis) const;
    virtual bool compare(const Axis1D& dens_distr) const = 0;

    virtual Axis1D* clone() const = 0;
    virtual std::shared_ptr<const Axis1D> create() const = 0;

    virtual double GetX(const Vector3D& xi) const = 0;
    virtual double GetdX(const Vector3D& xi, const Vector3D& direction) const = 0;

    Vector3D GetAxis() const { return fAxis_; };
    Vector3D GetFp0() const { return fp0_; };

    template<class Archive>
    void serialize(Archive & archive, std::uint32_t const version) {
        if(version == 0) {
            archive(::cereal::make_nvp("Axis", fAxis_));
            archive(::cereal::make_nvp("Origin", fp0_));
        } else {
            throw std::runtime_error("Axis1D only supports version <= 0");
        }
    };
   protected:
    Vector3D fAxis_;
    Vector3D fp0_;
};

class RadialAxis1D : public Axis1D {
friend cereal::access;
public:
    RadialAxis1D();
    RadialAxis1D(const Vector3D& fAxis, const Vector3D& fp0);
    RadialAxis1D(const Vector3D& fp0);
    ~RadialAxis1D() {};

    bool compare(const Axis1D& dens_distr) const override;

    Axis1D* clone() const override { return new RadialAxis1D(*this); };
    std::shared_ptr<const Axis1D> create() const override {
        return std::shared_ptr<const Axis1D>(new RadialAxis1D(*this));
    };

    double GetX(const Vector3D& xi) const override;
    double GetdX(const Vector3D& xi, const Vector3D& direction) const override;

    template<class Archive>
    void serialize(Archive & archive, std::uint32_t const version) {
        if(version == 0) {
            archive(cereal::virtual_base_class<Axis1D>(this));
        } else {
            throw std::runtime_error("RadialAxis1D only supports version <= 0");
        }
    };
};

class CartesianAxis1D : public Axis1D {
   public:
    CartesianAxis1D();
    CartesianAxis1D(const Vector3D& fAxis, const Vector3D& fp0);
    ~CartesianAxis1D() {};

    bool compare(const Axis1D& dens_distr) const override;

    Axis1D* clone() const override { return new CartesianAxis1D(*this); };
    std::shared_ptr<const Axis1D> create() const override {
        return std::shared_ptr<const Axis1D>(new CartesianAxis1D(*this));
    };

    double GetX(const Vector3D& xi) const override;
    double GetdX(const Vector3D& xi, const Vector3D& direction) const override;

    template<class Archive>
    void serialize(Archive & archive, std::uint32_t const version) {
        if(version == 0) {
            archive(cereal::virtual_base_class<Axis1D>(this));
        } else {
            throw std::runtime_error("CartesianAxis1D only supports version <= 0");
        }
    };
};

template <typename AxisT, typename DistributionT, class E = typename std::enable_if<std::is_base_of<Axis1D, AxisT>::value && std::is_base_of<Distribution1D, DistributionT>::value>::type>
class DensityDistribution1D
    : public DensityDistribution {
    using T = DensityDistribution1D<AxisT,DistributionT>;
private:
    AxisT axis;
    DistributionT dist;
public:
    DensityDistribution1D() : axis(), dist() {};
    DensityDistribution1D(const AxisT& axis, const DistributionT& dist)
        : axis(axis), dist(dist) {};
    DensityDistribution1D(const DensityDistribution1D& other)
        : axis(other.axis), dist(other.dist) {};

    bool compare(const DensityDistribution& d) const override {
        const DensityDistribution1D* d_1d = dynamic_cast<const DensityDistribution1D*>(&d);
        if(!d_1d)
            return false;
        if(axis != d_1d->axis or dist != d_1d->dist)
            return false;
        return true;
    };

    DensityDistribution* clone() const override { return new T(*this); };
    std::shared_ptr<const DensityDistribution> create() const override {
        return std::shared_ptr<const DensityDistribution>(new T(*this));
    };

    double Derivative(const Vector3D& xi,
                      const Vector3D& direction) const override {
        return dist.Derivative(axis.GetX(xi))*axis.GetdX(xi, direction);
    };

    double AntiDerivative(const Vector3D& xi,
                          const Vector3D& direction) const override {
        Vector3D cap = xi - (xi*direction)*direction;
        return Integral(cap, direction, xi*direction);
    };

    double Integral(const Vector3D& xi,
                    const Vector3D& direction,
                    double distance) const override {
        std::function<double(double)> f = [&](double x)->double {
            return Evaluate(xi+x*direction);
        };
        return Integration::rombergIntegrate(f, 0, distance, 1e-6);
    }

    double Integral(const Vector3D& xi,
                    const Vector3D& xj) const override {
        Vector3D direction = xj-xi;
        double distance = direction.magnitude();
        direction.normalize();
        return Integral(xi, direction, distance);
    };

    double InverseIntegral(const Vector3D& xi,
                           const Vector3D& direction,
                           double integral,
                           double max_distance) const override {
        std::function<double(double)> F = [&](double x)->double {
            return Integral(xi, direction, x) - integral;
        };

        std::function<double(double)> dF = [&](double x)->double {
            return Evaluate(xi+direction*x);
        };

        double res;
        try {
            double init = max_distance / 2.0;
            if(std::isinf(init)) {
                init = dF(0.0);
            }
            res = LI::math::NewtonRaphson(F, dF, 0, max_distance, init);
        } catch(LI::math::MathException& e) {
            res = -1;
        }
        return res;
    };

    double Evaluate(const Vector3D& xi) const override {
        return dist.Evaluate(axis.GetX(xi));
    };

    template<class Archive>
    void serialize(Archive & archive, std::uint32_t const version) {
        if(version == 0) {
            archive(::cereal::make_nvp("Axis", axis));
            archive(::cereal::make_nvp("Distribution", dist));
            archive(cereal::virtual_base_class<DensityDistribution>(this));
        } else {
            throw std::runtime_error("DensityDistribution1D only supports version <= 0");
        }
    };
};

template<typename AxisT>
class DensityDistribution1D<AxisT, ConstantDistribution1D, typename std::enable_if<std::is_base_of<Axis1D, AxisT>::value>::type> : public DensityDistribution {
    using DistributionT = ConstantDistribution1D;
    using T = DensityDistribution1D<AxisT, ConstantDistribution1D>;
   private:
    AxisT axis;
    ConstantDistribution1D dist;
   public:
    DensityDistribution1D() : axis(), dist() {};
    DensityDistribution1D(const AxisT& axis, const ConstantDistribution1D& dist)
        : axis(axis), dist(dist) {};
    DensityDistribution1D(const AxisT& axis, double val)
        : axis(axis), dist(val) {};
    DensityDistribution1D(double val)
        : axis(), dist(val) {};
    DensityDistribution1D(const DensityDistribution1D& other)
        : axis(other.axis), dist(other.dist) {};

    bool compare(const DensityDistribution& d) const override {
        const T* d_1d = dynamic_cast<const T*>(&d);
        if(!d_1d)
            return false;
        if(axis != d_1d->axis or dist != d_1d->dist)
            return false;
        return true;
    };

    DensityDistribution* clone() const override { return new T(*this); };
    std::shared_ptr<const DensityDistribution> create() const override {
        return std::shared_ptr<const DensityDistribution>(new T(*this));
    };

    double Derivative(const Vector3D& xi,
                      const Vector3D& direction) const override {
        return 0;
    };

    double AntiDerivative(const Vector3D& xi,
                          const Vector3D& direction) const override {
        //return (xi*axis.GetAxis())/(direction*axis.GetAxis()) * dist.Evaluate;
        return (xi*direction)*dist.Evaluate(0);
    };

    double Integral(const Vector3D& xi,
                    const Vector3D& direction,
                    double distance) const override {
        (void)direction;
        return distance*dist.Evaluate(0);
    }

    double Integral(const Vector3D& xi,
                    const Vector3D& xj) const override {
        double distance = (xj-xi).magnitude();
        return distance*dist.Evaluate(0);
    };

    double InverseIntegral(const Vector3D& xi,
                           const Vector3D& direction,
                           double integral,
                           double max_distance) const override {
        (void)direction;
        double distance = integral / dist.Evaluate(0);
        if(distance > max_distance) {
            distance = -1;
        }
        return distance;
    };

    double Evaluate(const Vector3D& xi) const override {
        (void)xi;
        return dist.Evaluate(0);
    };

    template<class Archive>
    void serialize(Archive & archive, std::uint32_t const version) {
        if(version == 0) {
            archive(::cereal::make_nvp("Axis", axis));
            archive(::cereal::make_nvp("Distribution", dist));
            archive(cereal::virtual_base_class<DensityDistribution>(this));
        } else {
            throw std::runtime_error("DensityDistribution1D only supports version <= 0");
        }
    };
};

template<typename DistributionT>
class DensityDistribution1D<CartesianAxis1D, DistributionT, typename std::enable_if<std::is_base_of<Distribution1D, DistributionT>::value && !std::is_same<ConstantDistribution1D,DistributionT>::value>::type> : public DensityDistribution {
    using AxisT = CartesianAxis1D;
    using T = DensityDistribution1D<CartesianAxis1D, DistributionT>;
   private:
    AxisT axis;
    DistributionT dist;
   public:
    DensityDistribution1D() : axis(), dist() {};
    DensityDistribution1D(const AxisT& axis, const DistributionT& dist)
        : axis(axis), dist(dist) {};
    DensityDistribution1D(const DensityDistribution1D& other)
        : axis(other.axis), dist(other.dist) {};

    bool compare(const DensityDistribution& d) const override {
        const T* d_1d = dynamic_cast<const T*>(&d);
        if(!d_1d)
            return false;
        if(axis != d_1d->axis or dist != d_1d->dist)
            return false;
        return true;
    };

    DensityDistribution* clone() const override { return new T(*this); };
    std::shared_ptr<const DensityDistribution> create() const override {
        return std::shared_ptr<const DensityDistribution>(new T(*this));
    };

    double Derivative(const Vector3D& xi,
                      const Vector3D& direction) const override {
        return dist.Derivative(axis.GetX(xi))*axis.GetdX(xi, direction);
    };

    double AntiDerivative(const Vector3D& xi,
                          const Vector3D& direction) const override {
        double a = 0;
        double b = axis.GetX(xi);
        double dxdt = axis.GetdX(xi, direction);
        return (dist.AntiDerivative(b) - dist.AntiDerivative(a)) / dxdt;
    };

    double Integral(const Vector3D& xi,
                    const Vector3D& direction,
                    double distance) const override {
        double a = axis.GetX(xi);
        Vector3D xj = xi + direction*distance;
        double b = axis.GetX(xj);
        double dxdt = axis.GetdX(xi, direction);
        return (dist.AntiDerivative(b) - dist.AntiDerivative(a)) / dxdt;
    }

    double Integral(const Vector3D& xi,
                    const Vector3D& xj) const override {
        double a = axis.GetX(xi);
        double b = axis.GetX(xj);
        Vector3D direction = xj-xi;
        direction.normalize();
        double dxdt = axis.GetdX(xi, direction);
        return (dist.AntiDerivative(b) - dist.AntiDerivative(a)) / dxdt;
    };

    double InverseIntegral(const Vector3D& xi,
                           const Vector3D& direction,
                           double integral,
                           double max_distance) const override {
        double a = axis.GetX(xi);
        double b = axis.GetX(xi + direction*max_distance);
        double dxdt = axis.GetdX(xi, direction);

        double dist_integral = integral * dxdt;

        double Ia = dist.AntiDerivative(a);
        std::function<double(double)> F = [&](double x)->double {
            return (dist.AntiDerivative(x) - Ia) - dist_integral;
        };

        std::function<double(double)> dF = [&](double x)->double {
            return dist.Evaluate(x);
        };

        try {
            double b_res = LI::math::NewtonRaphson(F, dF, a, b, (a+b)/2.0);
            return (b_res - a)/dxdt;
        } catch(LI::math::MathException& e) {
            return -1;
        }

    };

    double Evaluate(const Vector3D& xi) const override {
        return dist.Evaluate(axis.GetX(xi));
    };

    template<class Archive>
    void serialize(Archive & archive, std::uint32_t const version) {
        if(version == 0) {
            archive(::cereal::make_nvp("Axis", axis));
            archive(::cereal::make_nvp("Distribution", dist));
            archive(cereal::virtual_base_class<DensityDistribution>(this));
        } else {
            throw std::runtime_error("DensityDistribution1D only supports version <= 0");
        }
    };
};

template <>
class DensityDistribution1D<RadialAxis1D,PolynomialDistribution1D>
    : public DensityDistribution {
    using AxisT = RadialAxis1D;
    using DistributionT = PolynomialDistribution1D;
    using T = DensityDistribution1D<AxisT,DistributionT>;
   private:
    AxisT axis;
    DistributionT dist;
   public:
    DensityDistribution1D() : axis(), dist() {};
    DensityDistribution1D(const AxisT& axis, const DistributionT& dist)
        : axis(axis), dist(dist) {};
    DensityDistribution1D(const AxisT& axis, const Polynom& poly)
        : axis(axis), dist(poly) {};
    DensityDistribution1D(const AxisT& axis, const std::vector<double>& poly)
        : axis(axis), dist(poly) {};
    DensityDistribution1D(const T& other)
        : axis(other.axis), dist(other.dist) {};

    bool compare(const DensityDistribution& d) const override {
        const DensityDistribution1D* d_1d = dynamic_cast<const DensityDistribution1D*>(&d);
        if(!d_1d)
            return false;
        if(axis != d_1d->axis or dist != d_1d->dist)
            return false;
        return true;
    };

    DensityDistribution* clone() const override { return new DensityDistribution1D(*this); };
    std::shared_ptr<const DensityDistribution> create() const override {
        return std::shared_ptr<const DensityDistribution>(new DensityDistribution1D(*this));
    };

    double Derivative(const Vector3D& xi,
                      const Vector3D& direction) const override {
        return dist.Derivative(axis.GetX(xi))*axis.GetdX(xi, direction);
    };

    double AntiDerivative(const Vector3D& xi,
                          const Vector3D& direction) const override {
        Vector3D cap = xi - (xi*direction)*direction;
        return Integral(cap, direction, xi*direction);
    };

    double Integral(const Vector3D& xi,
                    const Vector3D& direction,
                    double distance) const override {
        // TODO implement analytic integral
        std::function<double(double)> f = [&](double x)->double {
            return Evaluate(xi+x*direction);
        };
        return Integration::rombergIntegrate(f, 0, distance, 1e-6);
    }

    double Integral(const Vector3D& xi,
                    const Vector3D& xj) const override {
        // TODO implement analytic integral
        Vector3D direction = xj-xi;
        double distance = direction.magnitude();
        direction.normalize();
        return Integral(xi, direction, distance);
    };

    double InverseIntegral(const Vector3D& xi,
                           const Vector3D& direction,
                           double integral,
                           double max_distance) const override {
        // TODO implement analytic integral
        std::function<double(double)> F = [&](double x)->double {
            return Integral(xi, direction, x) - integral;
        };

        std::function<double(double)> dF = [&](double x)->double {
            return Evaluate(xi+direction*x);
        };

        double res;
        try {
            double init = max_distance / 2.0;
            if(std::isinf(init)) {
                init = dF(0.0);
            }
            res = LI::math::NewtonRaphson(F, dF, 0, max_distance, init);
        } catch(LI::math::MathException& e) {
            res = -1;
        }
        return res;
    };

    double Evaluate(const Vector3D& xi) const override {
        return dist.Evaluate(axis.GetX(xi));
    };

    template<class Archive>
    void serialize(Archive & archive, std::uint32_t const version) {
        if(version == 0) {
            archive(::cereal::make_nvp("Axis", axis));
            archive(::cereal::make_nvp("Distribution", dist));
            archive(cereal::virtual_base_class<DensityDistribution>(this));
        } else {
            throw std::runtime_error("DensityDistribution1D only supports version <= 0");
        }
    };
};

} // namespace detector
} // namespace LI

CEREAL_CLASS_VERSION(LI::detector::DensityDistribution, 0);

CEREAL_CLASS_VERSION(LI::detector::Distribution1D, 0);

CEREAL_CLASS_VERSION(LI::detector::ConstantDistribution1D, 0);
CEREAL_REGISTER_TYPE(LI::detector::ConstantDistribution1D);
CEREAL_REGISTER_POLYMORPHIC_RELATION(LI::detector::Distribution1D, LI::detector::ConstantDistribution1D);

CEREAL_CLASS_VERSION(LI::detector::PolynomialDistribution1D, 0);
CEREAL_REGISTER_TYPE(LI::detector::PolynomialDistribution1D);
CEREAL_REGISTER_POLYMORPHIC_RELATION(LI::detector::Distribution1D, LI::detector::PolynomialDistribution1D);

CEREAL_CLASS_VERSION(LI::detector::ExponentialDistribution1D, 0);
CEREAL_REGISTER_TYPE(LI::detector::ExponentialDistribution1D);
CEREAL_REGISTER_POLYMORPHIC_RELATION(LI::detector::Distribution1D, LI::detector::ExponentialDistribution1D);

CEREAL_CLASS_VERSION(LI::detector::Axis1D, 0);

CEREAL_CLASS_VERSION(LI::detector::RadialAxis1D, 0);
CEREAL_REGISTER_TYPE(LI::detector::RadialAxis1D);
CEREAL_REGISTER_POLYMORPHIC_RELATION(LI::detector::Axis1D, LI::detector::RadialAxis1D);

CEREAL_CLASS_VERSION(LI::detector::CartesianAxis1D, 0);
CEREAL_REGISTER_TYPE(LI::detector::CartesianAxis1D);
CEREAL_REGISTER_POLYMORPHIC_RELATION(LI::detector::Axis1D, LI::detector::CartesianAxis1D);

#define COMMA ,

CEREAL_CLASS_VERSION(LI::detector::DensityDistribution1D<LI::detector::RadialAxis1D COMMA LI::detector::ConstantDistribution1D>, 0);
CEREAL_REGISTER_TYPE(LI::detector::DensityDistribution1D<LI::detector::RadialAxis1D COMMA LI::detector::ConstantDistribution1D>);
CEREAL_REGISTER_POLYMORPHIC_RELATION(LI::detector::DensityDistribution, LI::detector::DensityDistribution1D<LI::detector::RadialAxis1D COMMA LI::detector::ConstantDistribution1D>);

CEREAL_CLASS_VERSION(LI::detector::DensityDistribution1D<LI::detector::CartesianAxis1D COMMA LI::detector::ConstantDistribution1D>, 0);
CEREAL_REGISTER_TYPE(LI::detector::DensityDistribution1D<LI::detector::CartesianAxis1D COMMA LI::detector::ConstantDistribution1D>);
CEREAL_REGISTER_POLYMORPHIC_RELATION(LI::detector::DensityDistribution, LI::detector::DensityDistribution1D<LI::detector::CartesianAxis1D COMMA LI::detector::ConstantDistribution1D>);

CEREAL_CLASS_VERSION(LI::detector::DensityDistribution1D<LI::detector::RadialAxis1D COMMA LI::detector::PolynomialDistribution1D>, 0);
CEREAL_REGISTER_TYPE(LI::detector::DensityDistribution1D<LI::detector::RadialAxis1D COMMA LI::detector::PolynomialDistribution1D>);
CEREAL_REGISTER_POLYMORPHIC_RELATION(LI::detector::DensityDistribution, LI::detector::DensityDistribution1D<LI::detector::RadialAxis1D COMMA LI::detector::PolynomialDistribution1D>);

CEREAL_CLASS_VERSION(LI::detector::DensityDistribution1D<LI::detector::CartesianAxis1D COMMA LI::detector::PolynomialDistribution1D>, 0);
CEREAL_REGISTER_TYPE(LI::detector::DensityDistribution1D<LI::detector::CartesianAxis1D COMMA LI::detector::PolynomialDistribution1D>);
CEREAL_REGISTER_POLYMORPHIC_RELATION(LI::detector::DensityDistribution, LI::detector::DensityDistribution1D<LI::detector::CartesianAxis1D COMMA LI::detector::PolynomialDistribution1D>);

CEREAL_CLASS_VERSION(LI::detector::DensityDistribution1D<LI::detector::RadialAxis1D COMMA LI::detector::ExponentialDistribution1D>, 0);
CEREAL_REGISTER_TYPE(LI::detector::DensityDistribution1D<LI::detector::RadialAxis1D COMMA LI::detector::ExponentialDistribution1D>);
CEREAL_REGISTER_POLYMORPHIC_RELATION(LI::detector::DensityDistribution, LI::detector::DensityDistribution1D<LI::detector::RadialAxis1D COMMA LI::detector::ExponentialDistribution1D>);

CEREAL_CLASS_VERSION(LI::detector::DensityDistribution1D<LI::detector::CartesianAxis1D COMMA LI::detector::ExponentialDistribution1D>, 0);
CEREAL_REGISTER_TYPE(LI::detector::DensityDistribution1D<LI::detector::CartesianAxis1D COMMA LI::detector::ExponentialDistribution1D>);
CEREAL_REGISTER_POLYMORPHIC_RELATION(LI::detector::DensityDistribution, LI::detector::DensityDistribution1D<LI::detector::CartesianAxis1D COMMA LI::detector::ExponentialDistribution1D>);

#endif // LI_DensityDist_H
