#pragma once
#ifndef LI_LeptonInjector_H
#define LI_LeptonInjector_H

#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <stdexcept>

#include <cereal/cereal.hpp>
#include <cereal/access.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/types/utility.hpp>

#include "LeptonInjector/serialization/array.h"

#include "LeptonInjector/detector/EarthModel.h"

#include "LeptonInjector/crosssections/CrossSection.h"

#include "LeptonInjector/injection/Distributions.h"

namespace LI {
// #include "earthmodel-service/Vector3D.h"
namespace math {
class Vector3D;
}

//#include "LeptonInjector/Random.h"
namespace utilities {
class LI_random;
}
} // namespace LeptonInjector

namespace LeptonInjector {

class InjectorBase {
friend cereal::access;
protected:
    unsigned int events_to_inject = 0;
    unsigned int injected_events = 0;
    std::shared_ptr<LI::utilities::LI_random> random;
    std::shared_ptr<PrimaryInjector> primary_injector;
    std::shared_ptr<LI::crosssections::CrossSectionCollection> cross_sections;
    std::shared_ptr<LI::detector::EarthModel> earth_model;
    std::vector<std::shared_ptr<InjectionDistribution>> distributions;
    InjectorBase();
public:
    InjectorBase(unsigned int events_to_inject, std::shared_ptr<PrimaryInjector> primary_injector, std::vector<std::shared_ptr<LI::crosssections::CrossSection>> cross_sections, std::shared_ptr<LI::detector::EarthModel> earth_model, std::vector<std::shared_ptr<InjectionDistribution>> distributions, std::shared_ptr<LI::utilities::LI_random> random);
    InjectorBase(unsigned int events_to_inject, std::shared_ptr<PrimaryInjector> primary_injector, std::vector<std::shared_ptr<LI::crosssections::CrossSection>> cross_sections, std::shared_ptr<LI::detector::EarthModel> earth_model, std::shared_ptr<LI::utilities::LI_random> random);
    InjectorBase(unsigned int events_to_inject, std::shared_ptr<LI::crosssections::CrossSectionCollection> cross_sections);
    virtual LI::crosssections::InteractionRecord NewRecord() const;
    void SetRandom(std::shared_ptr<LI::utilities::LI_random> random);
    virtual void SampleCrossSection(LI::crosssections::InteractionRecord & record) const;
    virtual void SampleSecondaryDecay(LI::crosssections::InteractionRecord const & interaction, LI::crosssections::DecayRecord & decay, double width, double alpha_gen, double alpha_phys, LI::geometry::Geometry *fiducial, double buffer) const;
    virtual void SamplePairProduction(LI::crosssections::DecayRecord const & decay, LI::crosssections::InteractionRecord & pairprod) const;
    LI::crosssections::InteractionRecord GenerateEvent();
    virtual std::string Name() const;
    virtual double GenerationProbability(LI::crosssections::InteractionRecord const & record) const;
    virtual std::set<std::vector<std::string>> DensityVariables() const;
    virtual std::pair<LI::math::Vector3D, LI::math::Vector3D> InjectionBounds(LI::crosssections::InteractionRecord const & interaction) const;
    virtual std::vector<std::shared_ptr<InjectionDistribution>> GetInjectionDistributions() const;
    virtual std::shared_ptr<LI::detector::EarthModel> GetEarthModel() const;
    virtual std::shared_ptr<LI::crosssections::CrossSectionCollection> GetCrossSections() const;
    unsigned int InjectedEvents() const;
    unsigned int EventsToInject() const;
    operator bool() const;

    template<typename Archive>
    void save(Archive & archive, std::uint32_t const version) const {
        if(version == 0) {
            archive(::cereal::make_nvp("EventsToInject", events_to_inject));
            archive(::cereal::make_nvp("InjectedEvents", injected_events));
            archive(::cereal::make_nvp("PrimaryInjector", primary_injector));
            archive(::cereal::make_nvp("CrossSections", cross_sections));
            archive(::cereal::make_nvp("EarthModel", earth_model));
            archive(::cereal::make_nvp("InjectionDistributions", distributions));
        } else {
            throw std::runtime_error("InjectorBase only supports version <= 0!");
        }
    }

    template<typename Archive>
    void load(Archive & archive, std::uint32_t const version) {
        if(version == 0) {
            archive(::cereal::make_nvp("EventsToInject", events_to_inject));
            archive(::cereal::make_nvp("InjectedEvents", injected_events));
            archive(::cereal::make_nvp("PrimaryInjector", primary_injector));
            archive(::cereal::make_nvp("CrossSections", cross_sections));
            archive(::cereal::make_nvp("EarthModel", earth_model));
            archive(::cereal::make_nvp("InjectionDistributions", distributions));
        } else {
            throw std::runtime_error("InjectorBase only supports version <= 0!");
        }
    }
};

class RangedLeptonInjector : public InjectorBase {
friend cereal::access;
protected:
    std::shared_ptr<PrimaryEnergyDistribution> energy_distribution;
    std::shared_ptr<PrimaryDirectionDistribution> direction_distribution;
    std::shared_ptr<TargetMomentumDistribution> target_momentum_distribution;
    std::shared_ptr<RangeFunction> range_func;
    std::shared_ptr<PrimaryNeutrinoHelicityDistribution> helicity_distribution;
    double disk_radius;
    double endcap_length;
    std::shared_ptr<RangePositionDistribution> position_distribution;
    RangedLeptonInjector();
public:
    RangedLeptonInjector(unsigned int events_to_inject, std::shared_ptr<PrimaryInjector> primary_injector, std::vector<std::shared_ptr<LI::crosssections::CrossSection>> cross_sections, std::shared_ptr<LI::detector::EarthModel> earth_model, std::shared_ptr<LI::utilities::LI_random> random, std::shared_ptr<PrimaryEnergyDistribution> edist, std::shared_ptr<PrimaryDirectionDistribution> ddist, std::shared_ptr<TargetMomentumDistribution> target_momentum_distribution, std::shared_ptr<RangeFunction> range_func, double disk_radius, double endcap_length, std::shared_ptr<PrimaryNeutrinoHelicityDistribution> helicity_distribution);
    std::string Name() const override;
    virtual std::pair<LI::math::Vector3D, LI::math::Vector3D> InjectionBounds(LI::crosssections::InteractionRecord const & interaction) const override;

    template<typename Archive>
    void save(Archive & archive, std::uint32_t const version) const {
        if(version == 0) {
            archive(::cereal::make_nvp("TargetMomentumDistribution", target_momentum_distribution));
            archive(::cereal::make_nvp("EnergyDistribution", energy_distribution));
            archive(::cereal::make_nvp("HelicityDistribution", helicity_distribution));
            archive(::cereal::make_nvp("DirectionDistribution", direction_distribution));
            archive(::cereal::make_nvp("RangeFunction", range_func));
            archive(::cereal::make_nvp("DiskRadius", disk_radius));
            archive(::cereal::make_nvp("EndcapLength", endcap_length));
            archive(::cereal::make_nvp("PositionDistribution", position_distribution));
            archive(cereal::virtual_base_class<InjectorBase>(this));
        } else {
            throw std::runtime_error("RangedLeptonInjector only supports version <= 0!");
        }
    }

    template<typename Archive>
    void load(Archive & archive, std::uint32_t const version) {
        if(version == 0) {
            archive(::cereal::make_nvp("TargetMomentumDistribution", target_momentum_distribution));
            archive(::cereal::make_nvp("EnergyDistribution", energy_distribution));
            archive(::cereal::make_nvp("HelicityDistribution", helicity_distribution));
            archive(::cereal::make_nvp("DirectionDistribution", direction_distribution));
            archive(::cereal::make_nvp("RangeFunction", range_func));
            archive(::cereal::make_nvp("DiskRadius", disk_radius));
            archive(::cereal::make_nvp("EndcapLength", endcap_length));
            archive(::cereal::make_nvp("PositionDistribution", position_distribution));
            archive(cereal::virtual_base_class<InjectorBase>(this));
        } else {
            throw std::runtime_error("RangedLeptonInjector only supports version <= 0!");
        }
    }
};

class ColumnDepthLeptonInjector : public InjectorBase {
friend cereal::access;
protected:
    std::shared_ptr<PrimaryEnergyDistribution> energy_distribution;
    std::shared_ptr<PrimaryDirectionDistribution> direction_distribution;
    std::shared_ptr<TargetMomentumDistribution> target_momentum_distribution;
    std::shared_ptr<DepthFunction> depth_func;
    std::shared_ptr<PrimaryNeutrinoHelicityDistribution> helicity_distribution;
    double disk_radius;
    double endcap_length;
    std::shared_ptr<ColumnDepthPositionDistribution> position_distribution;
    ColumnDepthLeptonInjector();
public:
    ColumnDepthLeptonInjector(unsigned int events_to_inject, std::shared_ptr<PrimaryInjector> primary_injector, std::vector<std::shared_ptr<LI::crosssections::CrossSection>> cross_sections, std::shared_ptr<LI::detector::EarthModel> earth_model, std::shared_ptr<LI::utilities::LI_random> random, std::shared_ptr<PrimaryEnergyDistribution> edist, std::shared_ptr<PrimaryDirectionDistribution> ddist, std::shared_ptr<TargetMomentumDistribution> target_momentum_distribution, std::shared_ptr<DepthFunction> depth_func, double disk_radius, double endcap_length, std::shared_ptr<PrimaryNeutrinoHelicityDistribution> helicity_distribution);
    std::string Name() const override;
    virtual std::pair<LI::math::Vector3D, LI::math::Vector3D> InjectionBounds(LI::crosssections::InteractionRecord const & interaction) const override;

    template<typename Archive>
    void save(Archive & archive, std::uint32_t const version) const {
        if(version == 0) {
            archive(::cereal::make_nvp("TargetMomentumDistribution", target_momentum_distribution));
            archive(::cereal::make_nvp("EnergyDistribution", energy_distribution));
            archive(::cereal::make_nvp("HelicityDistribution", helicity_distribution));
            archive(::cereal::make_nvp("DirectionDistribution", direction_distribution));
            archive(::cereal::make_nvp("DepthFunction", depth_func));
            archive(::cereal::make_nvp("DiskRadius", disk_radius));
            archive(::cereal::make_nvp("EndcapLength", endcap_length));
            archive(::cereal::make_nvp("PositionDistribution", position_distribution));
            archive(cereal::virtual_base_class<InjectorBase>(this));
        } else {
            throw std::runtime_error("ColumnDepthLeptonInjector only supports version <= 0!");
        }
    }

    template<typename Archive>
    void load(Archive & archive, std::uint32_t const version) {
        if(version == 0) {
            archive(::cereal::make_nvp("TargetMomentumDistribution", target_momentum_distribution));
            archive(::cereal::make_nvp("EnergyDistribution", energy_distribution));
            archive(::cereal::make_nvp("HelicityDistribution", helicity_distribution));
            archive(::cereal::make_nvp("DirectionDistribution", direction_distribution));
            archive(::cereal::make_nvp("DepthFunction", depth_func));
            archive(::cereal::make_nvp("DiskRadius", disk_radius));
            archive(::cereal::make_nvp("EndcapLength", endcap_length));
            archive(::cereal::make_nvp("PositionDistribution", position_distribution));
            archive(cereal::virtual_base_class<InjectorBase>(this));
        } else {
            throw std::runtime_error("ColumnDepthLeptonInjector only supports version <= 0!");
        }
    }
};

class DecayRangeLeptonInjector : public InjectorBase {
friend cereal::access;
protected:
    std::shared_ptr<PrimaryEnergyDistribution> energy_distribution;
    std::shared_ptr<PrimaryDirectionDistribution> direction_distribution;
    std::shared_ptr<TargetMomentumDistribution> target_momentum_distribution;
    std::shared_ptr<DecayRangeFunction> range_func;
    std::shared_ptr<PrimaryNeutrinoHelicityDistribution> helicity_distribution;
    double disk_radius;
    double endcap_length;
    std::shared_ptr<DecayRangePositionDistribution> position_distribution;
    DecayRangeLeptonInjector();
public:
    DecayRangeLeptonInjector(unsigned int events_to_inject, std::shared_ptr<PrimaryInjector> primary_injector, std::vector<std::shared_ptr<LI::crosssections::CrossSection>> cross_sections, std::shared_ptr<LI::detector::EarthModel> earth_model, std::shared_ptr<LI::utilities::LI_random> random, std::shared_ptr<PrimaryEnergyDistribution> edist, std::shared_ptr<PrimaryDirectionDistribution> ddist, std::shared_ptr<TargetMomentumDistribution> target_momentum_distribution, std::shared_ptr<DecayRangeFunction> range_func, double disk_radius, double endcap_length, std::shared_ptr<PrimaryNeutrinoHelicityDistribution> helicity_distribution);
    std::string Name() const override;
    virtual std::pair<LI::math::Vector3D, LI::math::Vector3D> InjectionBounds(LI::crosssections::InteractionRecord const & interaction) const override;
    template<typename Archive>
    void save(Archive & archive, std::uint32_t const version) const {
        if(version == 0) {
            archive(::cereal::make_nvp("TargetMomentumDistribution", target_momentum_distribution));
            archive(::cereal::make_nvp("EnergyDistribution", energy_distribution));
            archive(::cereal::make_nvp("HelicityDistribution", helicity_distribution));
            archive(::cereal::make_nvp("DirectionDistribution", direction_distribution));
            archive(::cereal::make_nvp("RangeFunction", range_func));
            archive(::cereal::make_nvp("DiskRadius", disk_radius));
            archive(::cereal::make_nvp("EndcapLength", endcap_length));
            archive(::cereal::make_nvp("PositionDistribution", position_distribution));
            archive(cereal::virtual_base_class<InjectorBase>(this));
        } else {
            throw std::runtime_error("DecayRangeLeptonInjector only supports version <= 0!");
        }
    }

    template<typename Archive>
    void load(Archive & archive, std::uint32_t const version) {
        if(version == 0) {
            archive(::cereal::make_nvp("TargetMomentumDistribution", target_momentum_distribution));
            archive(::cereal::make_nvp("EnergyDistribution", energy_distribution));
            archive(::cereal::make_nvp("HelicityDistribution", helicity_distribution));
            archive(::cereal::make_nvp("DirectionDistribution", direction_distribution));
            archive(::cereal::make_nvp("RangeFunction", range_func));
            archive(::cereal::make_nvp("DiskRadius", disk_radius));
            archive(::cereal::make_nvp("EndcapLength", endcap_length));
            archive(::cereal::make_nvp("PositionDistribution", position_distribution));
            archive(cereal::virtual_base_class<InjectorBase>(this));
        } else {
            throw std::runtime_error("DecayRangeLeptonInjector only supports version <= 0!");
        }
    }
};

class VolumeLeptonInjector : public InjectorBase {
friend cereal::access;
protected:
    std::shared_ptr<PrimaryEnergyDistribution> energy_distribution;
    std::shared_ptr<PrimaryDirectionDistribution> direction_distribution;
    std::shared_ptr<TargetMomentumDistribution> target_momentum_distribution;
    std::shared_ptr<CylinderVolumePositionDistribution> position_distribution;
    std::shared_ptr<PrimaryNeutrinoHelicityDistribution> helicity_distribution;
    VolumeLeptonInjector();
public:
    VolumeLeptonInjector(unsigned int events_to_inject, std::shared_ptr<PrimaryInjector> primary_injector, std::vector<std::shared_ptr<LI::crosssections::CrossSection>> cross_sections, std::shared_ptr<LI::detector::EarthModel> earth_model, std::shared_ptr<LI::utilities::LI_random> random, std::shared_ptr<PrimaryEnergyDistribution> edist, std::shared_ptr<PrimaryDirectionDistribution> ddist, std::shared_ptr<TargetMomentumDistribution> target_momentum_distribution, LI::geometry::Cylinder cylinder, std::shared_ptr<PrimaryNeutrinoHelicityDistribution> helicity_distribution);
    std::string Name() const override;
    virtual std::pair<LI::math::Vector3D, LI::math::Vector3D> InjectionBounds(LI::crosssections::InteractionRecord const & interaction) const override;
    template<typename Archive>
    void save(Archive & archive, std::uint32_t const version) const {
        if(version == 0) {
            archive(::cereal::make_nvp("TargetMomentumDistribution", target_momentum_distribution));
            archive(::cereal::make_nvp("EnergyDistribution", energy_distribution));
            archive(::cereal::make_nvp("HelicityDistribution", helicity_distribution));
            archive(::cereal::make_nvp("DirectionDistribution", direction_distribution));
            archive(::cereal::make_nvp("PositionDistribution", position_distribution));
            archive(cereal::virtual_base_class<InjectorBase>(this));
        } else {
            throw std::runtime_error("VolumeLeptonInjector only supports version <= 0!");
        }
    }

    template<typename Archive>
    void load(Archive & archive, std::uint32_t const version) {
        if(version == 0) {
            archive(::cereal::make_nvp("TargetMomentumDistribution", target_momentum_distribution));
            archive(::cereal::make_nvp("EnergyDistribution", energy_distribution));
            archive(::cereal::make_nvp("HelicityDistribution", helicity_distribution));
            archive(::cereal::make_nvp("DirectionDistribution", direction_distribution));
            archive(::cereal::make_nvp("PositionDistribution", position_distribution));
            archive(cereal::virtual_base_class<InjectorBase>(this));
        } else {
            throw std::runtime_error("VolumeLeptonInjector only supports version <= 0!");
        }
    }
};

} //namespace LeptonInjector

CEREAL_CLASS_VERSION(LeptonInjector::InjectorBase, 0);

CEREAL_CLASS_VERSION(LeptonInjector::RangedLeptonInjector, 0);
CEREAL_REGISTER_TYPE(LeptonInjector::RangedLeptonInjector);
CEREAL_REGISTER_POLYMORPHIC_RELATION(LeptonInjector::InjectorBase, LeptonInjector::RangedLeptonInjector);

CEREAL_CLASS_VERSION(LeptonInjector::DecayRangeLeptonInjector, 0);
CEREAL_REGISTER_TYPE(LeptonInjector::DecayRangeLeptonInjector);
CEREAL_REGISTER_POLYMORPHIC_RELATION(LeptonInjector::InjectorBase, LeptonInjector::DecayRangeLeptonInjector);

CEREAL_CLASS_VERSION(LeptonInjector::VolumeLeptonInjector, 0);
CEREAL_REGISTER_TYPE(LeptonInjector::VolumeLeptonInjector);
CEREAL_REGISTER_POLYMORPHIC_RELATION(LeptonInjector::InjectorBase, LeptonInjector::VolumeLeptonInjector);

#endif // LI_LeptonInjector_H
