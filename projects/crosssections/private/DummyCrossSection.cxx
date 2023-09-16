#include <map>
#include <set>
#include <vector>
#include <string>
#include <memory>

#include <rk/rk.hh>
#include <rk/geom3.hh>

#include <photospline/splinetable.h>
#include <photospline/cinter/splinetable.h>

#include "LeptonInjector/dataclasses/Particle.h"

#include "LeptonInjector/utilities/Random.h"

#include "LeptonInjector/crosssections/CrossSection.h"
#include "LeptonInjector/crosssections/DummyCrossSection.h"

namespace LI {
namespace crosssections {

DummyCrossSection::DummyCrossSection() {}


bool DummyCrossSection::equal(CrossSection const & other) const {
    const DummyCrossSection* x = dynamic_cast<const DummyCrossSection*>(&other);

    if(!x)
        return false;
    else
        return true;
}

double DummyCrossSection::TotalCrossSection(dataclasses::InteractionRecord const & interaction) const {
    LI::dataclasses::Particle::ParticleType primary_type = interaction.signature.primary_type;
    LI::dataclasses::Particle::ParticleType target_type = interaction.signature.target_type;
    rk::P4 p1(geom3::Vector3(interaction.primary_momentum[1], interaction.primary_momentum[2], interaction.primary_momentum[3]), interaction.primary_mass);
    rk::P4 p2(geom3::Vector3(interaction.target_momentum[1], interaction.target_momentum[2], interaction.target_momentum[3]), interaction.target_mass);
    double primary_energy;
    if(interaction.target_momentum[1] == 0 and interaction.target_momentum[2] == 0 and interaction.target_momentum[3] == 0) {
        primary_energy = interaction.primary_momentum[0];
    } else {
        rk::Boost boost_start_to_lab = p2.restBoost();
        rk::P4 p1_lab = boost_start_to_lab * p1;
        primary_energy = p1_lab.e();
    }
    return TotalCrossSection(primary_type, primary_energy, target_type);
}

double DummyCrossSection::TotalCrossSection(LI::dataclasses::Particle::ParticleType primary_type, double primary_energy, LI::dataclasses::Particle::ParticleType target_type) const {
    double interaction_length_m = 1e6;
    double interaction_length_cm = interaction_length_m * 100;
    double density = 1.0; // g/cm^3
    double mol = 6.0221415e23; // particles/g
    double per_cm2 = density * interaction_length_cm * mol; // particles / cm^2
    double cm2 = 1.0 / per_cm2; // cm2/particle
    return cm2 * primary_energy / 1e5; // Interaction length was computed for 100TeV
}


double DummyCrossSection::DifferentialCrossSection(dataclasses::InteractionRecord const & interaction) const {
    LI::dataclasses::Particle::ParticleType primary_type = interaction.signature.primary_type;
    LI::dataclasses::Particle::ParticleType target_type = interaction.signature.target_type;
    rk::P4 p1(geom3::Vector3(interaction.primary_momentum[1], interaction.primary_momentum[2], interaction.primary_momentum[3]), interaction.primary_mass);
    rk::P4 p2(geom3::Vector3(interaction.target_momentum[1], interaction.target_momentum[2], interaction.target_momentum[3]), interaction.target_mass);
    double primary_energy;
    rk::P4 p1_lab;
    rk::P4 p2_lab;
    if(interaction.target_momentum[1] == 0 and interaction.target_momentum[2] == 0 and interaction.target_momentum[3] == 0) {
        primary_energy = interaction.primary_momentum[0];
        p1_lab = p1;
        p2_lab = p2;
    } else {
        rk::Boost boost_start_to_lab = p2.restBoost();
        p1_lab = boost_start_to_lab * p1;
        p2_lab = boost_start_to_lab * p2;
        primary_energy = p1_lab.e();
    }

    return TotalCrossSection(primary_type, primary_energy, target_type);
}

double DummyCrossSection::InteractionThreshold(dataclasses::InteractionRecord const & interaction) const {
    return 0;
}

void DummyCrossSection::SampleFinalState(dataclasses::InteractionRecord& interaction, std::shared_ptr<LI::utilities::LI_random> random) const {
    rk::P4 p1(geom3::Vector3(interaction.primary_momentum[1], interaction.primary_momentum[2], interaction.primary_momentum[3]), interaction.primary_mass);
    rk::P4 p2(geom3::Vector3(interaction.target_momentum[1], interaction.target_momentum[2], interaction.target_momentum[3]), interaction.target_mass);

    // we assume that:
    // the target is stationary so its energy is just its mass
    // the incoming neutrino is massless, so its kinetic energy is its total energy
    // double s = target_mass_ * tinteraction.secondary_momentarget_mass_ + 2 * target_mass_ * primary_energy;
    // double s = std::pow(rk::invMass(p1, p2), 2);

    double primary_energy;
    rk::P4 p1_lab;
    rk::P4 p2_lab;
    if(interaction.target_momentum[1] == 0 and interaction.target_momentum[2] == 0 and interaction.target_momentum[3] == 0) {
        p1_lab = p1;
        p2_lab = p2;
        primary_energy = p1_lab.e();
    } else {
        // Rest frame of p2 will be our "lab" frame
        rk::Boost boost_start_to_lab = p2.restBoost();
        p1_lab = boost_start_to_lab * p1;
        p2_lab = boost_start_to_lab * p2;
        primary_energy = p1_lab.e();
    }

    double final_x = random->Uniform(0, 1);
    double final_y = random->Uniform(0, 1);

    interaction.interaction_parameters.resize(3);
    interaction.interaction_parameters[0] = primary_energy;
    interaction.interaction_parameters[1] = final_x;
    interaction.interaction_parameters[2] = final_y;

    double m1 = interaction.primary_mass;
    double m3 = 0;
    double E1_lab = p1_lab.e();
    double E2_lab = p2_lab.e();
    double Q2 = 2 * E1_lab * E2_lab * final_x * final_y;
    double p1x_lab = std::sqrt(p1_lab.px() * p1_lab.px() + p1_lab.py() * p1_lab.py() + p1_lab.pz() * p1_lab.pz());
    double pqx_lab = (m1*m1 + m3*m3 + 2 * p1x_lab * p1x_lab + Q2 + 2 * E1_lab * E1_lab * (final_y - 1)) / (2.0 * p1x_lab);
    double momq_lab = std::sqrt(m1*m1 + p1x_lab*p1x_lab + Q2 + E1_lab * E1_lab * (final_y * final_y - 1));
    double pqy_lab = std::sqrt(momq_lab*momq_lab - pqx_lab *pqx_lab);
    double Eq_lab = E1_lab * final_y;

    geom3::UnitVector3 x_dir = geom3::UnitVector3::xAxis();
    geom3::Vector3 p1_mom = p1_lab.momentum();
    geom3::UnitVector3 p1_lab_dir = p1_mom.direction();
    geom3::Rotation3 x_to_p1_lab_rot = geom3::rotationBetween(x_dir, p1_lab_dir);

    double phi = random->Uniform(0, 2.0 * M_PI);
    geom3::Rotation3 rand_rot(p1_lab_dir, phi);

    rk::P4 pq_lab(Eq_lab, geom3::Vector3(pqx_lab, pqy_lab, 0));
    pq_lab.rotate(x_to_p1_lab_rot);
    pq_lab.rotate(rand_rot);

    rk::P4 p3_lab((p1_lab - pq_lab).momentum(), m3);
    rk::P4 p4_lab = p2_lab + pq_lab;

    rk::P4 p3;
    rk::P4 p4;
    if(interaction.target_momentum[1] == 0 and interaction.target_momentum[2] == 0 and interaction.target_momentum[3] == 0) {
        p3 = p3_lab;
        p4 = p4_lab;
    } else {
        rk::Boost boost_lab_to_start = p2.labBoost();
        p3 = boost_lab_to_start * p3_lab;
        p4 = boost_lab_to_start * p4_lab;
    }

    interaction.secondary_momenta.resize(2);
    interaction.secondary_masses.resize(2);
    interaction.secondary_helicity.resize(2);

    size_t lepton_index = 0;
    size_t other_index = 0;

    interaction.secondary_momenta[lepton_index][0] = p3.e(); // p3_energy
    interaction.secondary_momenta[lepton_index][1] = p3.px(); // p3_x
    interaction.secondary_momenta[lepton_index][2] = p3.py(); // p3_y
    interaction.secondary_momenta[lepton_index][3] = p3.pz(); // p3_z
    interaction.secondary_masses[lepton_index] = p3.m();

    interaction.secondary_helicity[lepton_index] = interaction.primary_helicity;

    interaction.secondary_momenta[other_index][0] = p4.e(); // p4_energy
    interaction.secondary_momenta[other_index][1] = p4.px(); // p4_x
    interaction.secondary_momenta[other_index][2] = p4.py(); // p4_y
    interaction.secondary_momenta[other_index][3] = p4.pz(); // p4_z
    interaction.secondary_masses[other_index] = p4.m();

    interaction.secondary_helicity[other_index] = interaction.target_helicity;
}

double DummyCrossSection::FinalStateProbability(dataclasses::InteractionRecord const & interaction) const {
    double dxs = DifferentialCrossSection(interaction);
    double txs = TotalCrossSection(interaction);
    if(dxs == 0) {
        return 0.0;
    } else {
        return dxs / txs;
    }
}

std::vector<LI::dataclasses::Particle::ParticleType> DummyCrossSection::GetPossibleTargets() const {
    return {
        LI::dataclasses::Particle::ParticleType::Nucleon
    };
}

std::vector<LI::dataclasses::Particle::ParticleType> DummyCrossSection::GetPossibleTargetsFromPrimary(LI::dataclasses::Particle::ParticleType primary_type) const {
    return {
        LI::dataclasses::Particle::ParticleType::Nucleon
    };
}

std::vector<LI::dataclasses::Particle::ParticleType> DummyCrossSection::GetPossiblePrimaries() const {
    return {
        LI::dataclasses::Particle::ParticleType::NuE,
        LI::dataclasses::Particle::ParticleType::NuEBar,
        LI::dataclasses::Particle::ParticleType::NuMu,
        LI::dataclasses::Particle::ParticleType::NuMuBar,
        LI::dataclasses::Particle::ParticleType::NuTau,
        LI::dataclasses::Particle::ParticleType::NuTauBar
    };
}

std::vector<dataclasses::InteractionSignature> DummyCrossSection::GetPossibleSignatures() const {
    std::vector<LI::dataclasses::InteractionSignature> sigs;
    LI::dataclasses::InteractionSignature signature;
    std::vector<LI::dataclasses::Particle::ParticleType> targets = GetPossibleTargets();
    std::vector<LI::dataclasses::Particle::ParticleType> primaries = GetPossiblePrimaries();
    for(auto target : targets) {
        signature.target_type = target;
        for(auto primary : primaries) {
            signature.primary_type = primary;
            signature.secondary_types = {primary, target};
            sigs.push_back(signature);
        }
    }
    return sigs;
}

std::vector<dataclasses::InteractionSignature> DummyCrossSection::GetPossibleSignaturesFromParents(LI::dataclasses::Particle::ParticleType primary_type, LI::dataclasses::Particle::ParticleType target_type) const {
    std::vector<dataclasses::InteractionSignature> sigs = DummyCrossSection::GetPossibleSignatures();
    std::vector<dataclasses::InteractionSignature> these_sigs;
    for(auto sig : sigs) {
        if(sig.primary_type == primary_type)
            these_sigs.push_back(sig);
    }
    return these_sigs;
}

std::vector<std::string> DummyCrossSection::DensityVariables() const {
    return std::vector<std::string>{"Bjorken x", "Bjorken y"};
}

} // namespace crosssections
} // namespace LI
