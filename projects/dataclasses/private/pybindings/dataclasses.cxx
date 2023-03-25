
#include <vector>
#include <set>

#include "../../public/LeptonInjector/dataclasses/Particle.h"
#include "../../public/LeptonInjector/dataclasses/InteractionSignature.h"
#include "../../public/LeptonInjector/dataclasses/InteractionRecord.h"
#include "../../public/LeptonInjector/dataclasses/InteractionTree.h"

#include <pybind11/pybind11.h>



using namespace pybind11;

PYBIND11_MODULE(Dataclasses,m) {
  using namespace LI::dataclasses;

  class_<Particle> particle(m, "Particle");
  
  particle.def(init<>())
          .def(init<Particle::ParticleType>())
          .def_readwrite("type",&Particle::type)
          .def_readwrite("energy",&Particle::energy)
          .def("GetMass",&Particle::GetMass)
          .def("HasMass",&Particle::HasMass)
          .def("GetTypeString",&Particle::GetTypeString);

  enum_<Particle::ParticleType>(particle, "ParticleType")
          .value("unknown", Particle::ParticleType::unknown)
          .value("Gamma", Particle::ParticleType::Gamma)
          .value("EPlus", Particle::ParticleType::EPlus)
          .value("EMinus", Particle::ParticleType::EMinus)
          .value("MuPlus", Particle::ParticleType::MuPlus)
          .value("MuMinus", Particle::ParticleType::MuMinus)
          .value("Pi0", Particle::ParticleType::Pi0)
          .value("PiPlus", Particle::ParticleType::PiPlus)
          .value("PiMinus", Particle::ParticleType::PiMinus)
          .value("K0_Long", Particle::ParticleType::K0_Long)
          .value("KPlus", Particle::ParticleType::KPlus)
          .value("KMinus", Particle::ParticleType::KMinus)
          .value("Neutron", Particle::ParticleType::Neutron)
          .value("PPlus", Particle::ParticleType::PPlus)
          .value("PMinus", Particle::ParticleType::PMinus)
          .value("K0_Short", Particle::ParticleType::K0_Short)
          .value("Eta", Particle::ParticleType::Eta)
          .value("Lambda", Particle::ParticleType::Lambda)
          .value("SigmaPlus", Particle::ParticleType::SigmaPlus)
          .value("Sigma0", Particle::ParticleType::Sigma0)
          .value("SigmaMinus", Particle::ParticleType::SigmaMinus)
          .value("Xi0", Particle::ParticleType::Xi0)
          .value("XiMinus", Particle::ParticleType::XiMinus)
          .value("OmegaMinus", Particle::ParticleType::OmegaMinus)
          .value("NeutronBar", Particle::ParticleType::NeutronBar)
          .value("LambdaBar", Particle::ParticleType::LambdaBar)
          .value("SigmaMinusBar", Particle::ParticleType::SigmaMinusBar)
          .value("Sigma0Bar", Particle::ParticleType::Sigma0Bar)
          .value("SigmaPlusBar", Particle::ParticleType::SigmaPlusBar)
          .value("Xi0Bar", Particle::ParticleType::Xi0Bar)
          .value("XiPlusBar", Particle::ParticleType::XiPlusBar)
          .value("OmegaPlusBar", Particle::ParticleType::OmegaPlusBar)
          .value("DPlus", Particle::ParticleType::DPlus)
          .value("DMinus", Particle::ParticleType::DMinus)
          .value("D0", Particle::ParticleType::D0)
          .value("D0Bar", Particle::ParticleType::D0Bar)
          .value("DsPlus", Particle::ParticleType::DsPlus)
          .value("DsMinusBar", Particle::ParticleType::DsMinusBar)
          .value("LambdacPlus", Particle::ParticleType::LambdacPlus)
          .value("WPlus", Particle::ParticleType::WPlus)
          .value("WMinus", Particle::ParticleType::WMinus)
          .value("Z0", Particle::ParticleType::Z0)
          .value("NuE", Particle::ParticleType::NuE)
          .value("NuEBar", Particle::ParticleType::NuEBar)
          .value("NuMu", Particle::ParticleType::NuMu)
          .value("NuMuBar", Particle::ParticleType::NuMuBar)
          .value("TauPlus", Particle::ParticleType::TauPlus)
          .value("TauMinus", Particle::ParticleType::TauMinus)
          .value("NuTau", Particle::ParticleType::NuTau)
          .value("NuTauBar", Particle::ParticleType::NuTauBar)
          .value("HNucleus", Particle::ParticleType::HNucleus)
          .value("H2Nucleus", Particle::ParticleType::H2Nucleus)
          .value("He3Nucleus", Particle::ParticleType::He3Nucleus)
          .value("He4Nucleus", Particle::ParticleType::He4Nucleus)
          .value("Li6Nucleus", Particle::ParticleType::Li6Nucleus)
          .value("Li7Nucleus", Particle::ParticleType::Li7Nucleus)
          .value("Be9Nucleus", Particle::ParticleType::Be9Nucleus)
          .value("B10Nucleus", Particle::ParticleType::B10Nucleus)
          .value("B11Nucleus", Particle::ParticleType::B11Nucleus)
          .value("C12Nucleus", Particle::ParticleType::C12Nucleus)
          .value("C13Nucleus", Particle::ParticleType::C13Nucleus)
          .value("N14Nucleus", Particle::ParticleType::N14Nucleus)
          .value("N15Nucleus", Particle::ParticleType::N15Nucleus)
          .value("O16Nucleus", Particle::ParticleType::O16Nucleus)
          .value("O17Nucleus", Particle::ParticleType::O17Nucleus)
          .value("O18Nucleus", Particle::ParticleType::O18Nucleus)
          .value("F19Nucleus", Particle::ParticleType::F19Nucleus)
          .value("Ne20Nucleus", Particle::ParticleType::Ne20Nucleus)
          .value("Ne21Nucleus", Particle::ParticleType::Ne21Nucleus)
          .value("Ne22Nucleus", Particle::ParticleType::Ne22Nucleus)
          .value("Na23Nucleus", Particle::ParticleType::Na23Nucleus)
          .value("Mg24Nucleus", Particle::ParticleType::Mg24Nucleus)
          .value("Mg25Nucleus", Particle::ParticleType::Mg25Nucleus)
          .value("Mg26Nucleus", Particle::ParticleType::Mg26Nucleus)
          .value("Al26Nucleus", Particle::ParticleType::Al26Nucleus)
          .value("Al27Nucleus", Particle::ParticleType::Al27Nucleus)
          .value("Si28Nucleus", Particle::ParticleType::Si28Nucleus)
          .value("Si29Nucleus", Particle::ParticleType::Si29Nucleus)
          .value("Si30Nucleus", Particle::ParticleType::Si30Nucleus)
          .value("Si31Nucleus", Particle::ParticleType::Si31Nucleus)
          .value("Si32Nucleus", Particle::ParticleType::Si32Nucleus)
          .value("P31Nucleus", Particle::ParticleType::P31Nucleus)
          .value("P32Nucleus", Particle::ParticleType::P32Nucleus)
          .value("P33Nucleus", Particle::ParticleType::P33Nucleus)
          .value("S32Nucleus", Particle::ParticleType::S32Nucleus)
          .value("S33Nucleus", Particle::ParticleType::S33Nucleus)
          .value("S34Nucleus", Particle::ParticleType::S34Nucleus)
          .value("S35Nucleus", Particle::ParticleType::S35Nucleus)
          .value("S36Nucleus", Particle::ParticleType::S36Nucleus)
          .value("Cl35Nucleus", Particle::ParticleType::Cl35Nucleus)
          .value("Cl36Nucleus", Particle::ParticleType::Cl36Nucleus)
          .value("Cl37Nucleus", Particle::ParticleType::Cl37Nucleus)
          .value("Ar36Nucleus", Particle::ParticleType::Ar36Nucleus)
          .value("Ar37Nucleus", Particle::ParticleType::Ar37Nucleus)
          .value("Ar38Nucleus", Particle::ParticleType::Ar38Nucleus)
          .value("Ar39Nucleus", Particle::ParticleType::Ar39Nucleus)
          .value("Ar40Nucleus", Particle::ParticleType::Ar40Nucleus)
          .value("Ar41Nucleus", Particle::ParticleType::Ar41Nucleus)
          .value("Ar42Nucleus", Particle::ParticleType::Ar42Nucleus)
          .value("K39Nucleus", Particle::ParticleType::K39Nucleus)
          .value("K40Nucleus", Particle::ParticleType::K40Nucleus)
          .value("K41Nucleus", Particle::ParticleType::K41Nucleus)
          .value("Ca40Nucleus", Particle::ParticleType::Ca40Nucleus)
          .value("Ca41Nucleus", Particle::ParticleType::Ca41Nucleus)
          .value("Ca42Nucleus", Particle::ParticleType::Ca42Nucleus)
          .value("Ca43Nucleus", Particle::ParticleType::Ca43Nucleus)
          .value("Ca44Nucleus", Particle::ParticleType::Ca44Nucleus)
          .value("Ca45Nucleus", Particle::ParticleType::Ca45Nucleus)
          .value("Ca46Nucleus", Particle::ParticleType::Ca46Nucleus)
          .value("Ca47Nucleus", Particle::ParticleType::Ca47Nucleus)
          .value("Ca48Nucleus", Particle::ParticleType::Ca48Nucleus)
          .value("Sc44Nucleus", Particle::ParticleType::Sc44Nucleus)
          .value("Sc45Nucleus", Particle::ParticleType::Sc45Nucleus)
          .value("Sc46Nucleus", Particle::ParticleType::Sc46Nucleus)
          .value("Sc47Nucleus", Particle::ParticleType::Sc47Nucleus)
          .value("Sc48Nucleus", Particle::ParticleType::Sc48Nucleus)
          .value("Ti44Nucleus", Particle::ParticleType::Ti44Nucleus)
          .value("Ti45Nucleus", Particle::ParticleType::Ti45Nucleus)
          .value("Ti46Nucleus", Particle::ParticleType::Ti46Nucleus)
          .value("Ti47Nucleus", Particle::ParticleType::Ti47Nucleus)
          .value("Ti48Nucleus", Particle::ParticleType::Ti48Nucleus)
          .value("Ti49Nucleus", Particle::ParticleType::Ti49Nucleus)
          .value("Ti50Nucleus", Particle::ParticleType::Ti50Nucleus)
          .value("V48Nucleus", Particle::ParticleType::V48Nucleus)
          .value("V49Nucleus", Particle::ParticleType::V49Nucleus)
          .value("V50Nucleus", Particle::ParticleType::V50Nucleus)
          .value("V51Nucleus", Particle::ParticleType::V51Nucleus)
          .value("Cr50Nucleus", Particle::ParticleType::Cr50Nucleus)
          .value("Cr51Nucleus", Particle::ParticleType::Cr51Nucleus)
          .value("Cr52Nucleus", Particle::ParticleType::Cr52Nucleus)
          .value("Cr53Nucleus", Particle::ParticleType::Cr53Nucleus)
          .value("Cr54Nucleus", Particle::ParticleType::Cr54Nucleus)
          .value("Mn52Nucleus", Particle::ParticleType::Mn52Nucleus)
          .value("Mn53Nucleus", Particle::ParticleType::Mn53Nucleus)
          .value("Mn54Nucleus", Particle::ParticleType::Mn54Nucleus)
          .value("Mn55Nucleus", Particle::ParticleType::Mn55Nucleus)
          .value("Fe54Nucleus", Particle::ParticleType::Fe54Nucleus)
          .value("Fe55Nucleus", Particle::ParticleType::Fe55Nucleus)
          .value("Fe56Nucleus", Particle::ParticleType::Fe56Nucleus)
          .value("Fe57Nucleus", Particle::ParticleType::Fe57Nucleus)
          .value("Fe58Nucleus", Particle::ParticleType::Fe58Nucleus)
          .value("Cu63Nucleus", Particle::ParticleType::Cu63Nucleus)
          .value("Cu65Nucleus", Particle::ParticleType::Cu65Nucleus)
          .value("Pb208Nucleus", Particle::ParticleType::Pb208Nucleus)
          .value("Qball", Particle::ParticleType::Qball)
          .value("NuF4", Particle::ParticleType::NuF4)
          .value("NuF4Bar", Particle::ParticleType::NuF4Bar)
          .value("Nucleon", Particle::ParticleType::Nucleon)
          .value("CherenkovPhoton", Particle::ParticleType::CherenkovPhoton)
          .value("Nu", Particle::ParticleType::Nu)
          .value("Monopole", Particle::ParticleType::Monopole)
          .value("Brems", Particle::ParticleType::Brems)
          .value("DeltaE", Particle::ParticleType::DeltaE)
          .value("PairProd", Particle::ParticleType::PairProd)
          .value("NuclInt", Particle::ParticleType::NuclInt)
          .value("MuPair", Particle::ParticleType::MuPair)
          .value("Hadrons", Particle::ParticleType::Hadrons)
          .value("Decay", Particle::ParticleType::Decay)
          .value("ContinuousEnergyLoss", Particle::ParticleType::ContinuousEnergyLoss)
          .value("FiberLaser", Particle::ParticleType::FiberLaser)
          .value("N2Laser", Particle::ParticleType::N2Laser)
          .value("YAGLaser", Particle::ParticleType::YAGLaser)
          .value("STauPlus", Particle::ParticleType::STauPlus)
          .value("STauMinus", Particle::ParticleType::STauMinus)
          .value("SMPPlus", Particle::ParticleType::SMPPlus)
          .value("SMPMinus", Particle::ParticleType::SMPMinus)
          .export_values();
  
  class_<InteractionSignature>(m, "InteractionSignature")
          .def(init<>())
          .def_readwrite("primary_type",&InteractionSignature::primary_type)
          .def_readwrite("target_type",&InteractionSignature::target_type)
          .def_readwrite("secondary_types",&InteractionSignature::secondary_types);
  
  class_<InteractionRecord>(m, "InteractionRecord")
          .def(init<>())
          .def_readwrite("signature",&InteractionRecord::signature)
          .def_readwrite("primary_mass",&InteractionRecord::primary_mass)
          .def_readwrite("primary_momentum",&InteractionRecord::primary_momentum)
          .def_readwrite("primary_helicity",&InteractionRecord::primary_helicity)
          .def_readwrite("target_mass",&InteractionRecord::target_mass)
          .def_readwrite("target_momentum",&InteractionRecord::target_momentum)
          .def_readwrite("target_helicity",&InteractionRecord::target_helicity)
          .def_readwrite("interaction_vertex",&InteractionRecord::interaction_vertex)
          .def_readwrite("secondary_masses",&InteractionRecord::secondary_masses)
          .def_readwrite("secondary_momenta",&InteractionRecord::secondary_momenta)
          .def_readwrite("secondary_helicity",&InteractionRecord::secondary_helicity)
          .def_readwrite("interaction_parameters",&InteractionRecord::interaction_parameters);

  class_<InteractionTreeDatum>(m, "InteractionTreeDatum")
          .def(init<InteractionRecord&>())
          .def_readwrite("record",&InteractionTreeDatum::record)
          .def_readwrite("parent",&InteractionTreeDatum::parent)
          .def_readwrite("daughters",&InteractionTreeDatum::daughters)
          .def("depth",&InteractionTreeDatum::depth);
  
  class_<InteractionTree>(m, "InteractionTree")
          .def(init<>())
          .def_readwrite("tree",&InteractionTree::tree)
          .def("add_entry",static_cast<std::shared_ptr<InteractionTreeDatum> (InteractionTree::*)(InteractionTreeDatum&,std::shared_ptr<InteractionTreeDatum>)>(&InteractionTree::add_entry))
          .def("add_entry",static_cast<std::shared_ptr<InteractionTreeDatum> (InteractionTree::*)(InteractionRecord&,std::shared_ptr<InteractionTreeDatum>)>(&InteractionTree::add_entry));

}
