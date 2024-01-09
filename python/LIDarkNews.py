# Standard python libraries
import numpy as np
import os
import datetime
import json
import ntpath
import pickle
from scipy.interpolate import CloughTocher2DInterpolator,CubicSpline

# LeptonInjector methods
import leptoninjector as LI
from leptoninjector.crosssections import DarkNewsCrossSection,DarkNewsDecay
from leptoninjector.dataclasses import Particle

# DarkNews methods
from DarkNews import phase_space
from DarkNews.ModelContainer import ModelContainer
from DarkNews.processes import *
from DarkNews.nuclear_tools import NuclearTarget
from DarkNews.integrands import get_decay_momenta_from_vegas_samples

# Class containing all upscattering and decay modes available in DarkNews
class PyDarkNewsCrossSectionCollection:

    def __init__(self,
                 table_dir=None,
                 param_file=None,
                 tolerance=1e-6,
                 interp_tolerance=5e-2,
                 **kwargs):
        # Defines a series of upscattering and decay objects
        # Each derive from the respective LeptonInjector classes
        
        # Get our model container with all ups_case and dec_case DarkNews objects
        self.models = ModelContainer(param_file,**kwargs)
        self.table_dir = table_dir
        
        # Default table_dir settings
        if self.table_dir is None:
            self.table_dir = os.environ.get('LEPTONINJECTOR_SRC') + '/resources/CrossSectionTables/DarkNewsTables/'
            ct = datetime.datetime.now().strftime('%Y_%m_%d__%H:%M/')
            self.table_dir += ct
        
        # Make the table directory where will we store cross section integrators
        table_dir_exists = False
        if(os.path.exists(self.table_dir)):
            #print("Directory '%s' already exists"%self.table_dir)
            table_dir_exists = True
        else:
            try:
                os.makedirs(self.table_dir, exist_ok = False)
                print("Directory '%s' created successfully"%self.table_dir)
            except OSError as error:
                print("Directory '%s' cannot be created"%self.table_dir)
                exit(0)

        if table_dir_exists:
            # Ensure that the model requested matches the model file already in the dictionary
            if param_file is not None:
                # ensure the param filename already exists
                param_filename = ntpath.basename(param_file) # should be OS-independent
                assert(os.path.isfile(self.table_dir + param_filename))
            # Make sure the model arguments agree
            with open(self.table_dir+'model_parameters.json',) as f:
                _model_args_dict = json.load(f)
                assert(self.models.model_args_dict==_model_args_dict)
        else:
            # Write a file to the directory containing infomration on the parameters used to create the model
            if param_file is not None:
                # Copy the param_file to the folder
                command = 'scp ' + param_file + ' ' + self.table_dir
                os.system(command)
            # Dump the model arguments
            with open(self.table_dir+'model_parameters.json','w') as f:
                json.dump(self.models.model_args_dict,f)
        
        # Save all unique scattering processes
        self.cross_sections = []
        for ups_key,ups_case in self.models.ups_cases.items():
            table_subdirs = 'CrossSection_'
            for x in ups_key:
                if(type(x)==NuclearTarget):
                    x = x.name
                table_subdirs += '%s_'%str(x)
            table_subdirs += '/'
            self.cross_sections.append(PyDarkNewsCrossSection(ups_case,
                                                              table_dir = self.table_dir + table_subdirs,
                                                              tolerance=tolerance,
                                                              interp_tolerance=interp_tolerance))
       # Save all unique decay processes
        self.decays = []
        for dec_key,dec_case in self.models.dec_cases.items():
            table_subdirs = 'Decay_'
            for x in dec_key:
                table_subdirs += '%s_'%str(x)
            table_subdirs += '/'
            self.decays.append(PyDarkNewsDecay(dec_case,
                                               table_dir = self.table_dir + table_subdirs))
            
    def SaveCrossSectionTables(self,fill_tables_at_exit=True):
        if not fill_tables_at_exit:
            print('WARNING: Saving tables without filling PyDarkNewsCrossSection interpolation tables. Future updates to DarkNews can lead to inconsistent behavior if new entries are ever added to this table')
        for cross_section in self.cross_sections:
            if fill_tables_at_exit:
                print('Filling cross section table at %s'%cross_section.table_dir)
                num = cross_section.FillInterpolationTables()
                print('Added %d points'%num)
            cross_section.SaveInterpolationTables()
            
        


# A class representing a single ups_case DarkNews class
# Only handles methods concerning the upscattering part
class PyDarkNewsCrossSection(DarkNewsCrossSection):

    def __init__(self,
                 ups_case, # DarkNews UpscatteringProcess instance
                 table_dir=None, # table to store 
                 tolerance=1e-6, # supposed to represent machine epsilon
                 interp_tolerance=5e-2, # relative interpolation tolerance
                 interpolate_differential=False):
        
        DarkNewsCrossSection.__init__(self) # C++ constructor
        
        self.ups_case = ups_case
        self.tolerance = tolerance
        self.interp_tolerance = interp_tolerance
        self.table_dir = table_dir
        self.interpolate_differential = interpolate_differential

        # 2D table in E, sigma
        self.total_cross_section_table = np.empty((0,2),dtype=float)
        self.total_cross_section_interpolator = None
        # 3D table in E, z, dsigma/dQ2 where z = (Q2 - Q2min) / (Q2max - Q2min)
        self.differential_cross_section_table = np.empty((0,3),dtype=float)
        self.differential_cross_section_interpolator = None
        
        if table_dir is None: 
            print('No table_dir specified; disabling interpolation\nWARNING: this will siginficantly slow down event generation')
            return

        # Make the table directory where will we store cross section integrators
        table_dir_exists = False
        if(os.path.exists(self.table_dir)):
            #print("Directory '%s' already exists"%self.table_dir)
            table_dir_exists = True
        else:
            try:
                os.makedirs(self.table_dir, exist_ok = False)
                print("Directory '%s' created successfully"%self.table_dir)
            except OSError as error:
                print("Directory '%s' cannot be created"%self.table_dir)
                exit(0)
        
        # Look in table dir and check whether total/differential xsec tables exist
        if table_dir_exists:
            total_xsec_file = self.table_dir + 'total_cross_sections.npy'
            if os.path.exists(total_xsec_file):
                self.total_cross_section_table = np.load(total_xsec_file)
            diff_xsec_file = self.table_dir + 'differential_cross_sections.npy'
            if os.path.exists(diff_xsec_file):
                self.differential_cross_section_table = np.load(diff_xsec_file)
            
            self._redefine_interpolation_objects(total=True,diff=True)

    # Sorts and redefines scipy interpolation objects
    def _redefine_interpolation_objects(self,total=False,diff=False):
        if total:
            self.total_cross_section_table = np.sort(self.total_cross_section_table,axis=0)
            if len(self.total_cross_section_table) > 1:
                self.total_cross_section_interpolator = CubicSpline(self.total_cross_section_table[:,0],
                                                                    self.total_cross_section_table[:,1])
        if diff:
            self.differential_cross_section_table = np.sort(self.differential_cross_section_table,axis=0)
            if len(self.differential_cross_section_table) > 1:
                # If we only have two energy points, don't try to construct interpolator
                if len(np.unique(self.differential_cross_section_table[:,0])) <= 2: return
                self.differential_cross_section_interpolator = CloughTocher2DInterpolator(self.differential_cross_section_table[:,:2],
                                                                                          self.differential_cross_section_table[:,2],
                                                                                          rescale=True)

    # Check whether we have close-enough entries in the intrepolation tables
    def _interpolation_flags(self,inputs,mode):
        #
        # returns UseSinglePoint,Interpolate,closest_idx
        # UseSinglePoint: whether to use a single point in table
        # Interpolate: whether to interpolate bewteen different points
        # closest_idx: index of closest point in table (for UseSinglePoint)

        # Determine which table we are using
        if mode=='total':
            interp_table = self.total_cross_section_table
        elif mode=='differential':
            interp_table = self.differential_cross_section_table
        else:
            print('Invalid interpolation table mode %s'%mode)
            exit(0)

        # first check if we have saved table points already
        if len(interp_table) == 0: return 0

        # bools to keep track of whether to use a single point or interpolate
        UseSinglePoint = True
        Interpolate = True
        #prev_closest_idx = None
        # First check whether we have a close-enough single point
        closest_idx = np.argmin(np.sum(np.abs(interp_table[:,:-1] - inputs)))
        diff = (interp_table[closest_idx,:-1] - inputs)/inputs
        if np.all(np.abs(diff)<self.tolerance): 
            UseSinglePoint = True
        for i,input in enumerate(inputs):
            closest_idx = np.argmin(np.abs(interp_table[:,i] - input))
            diff = (interp_table[closest_idx,i] - input)/input # relative difference
            # # First check if we are close enough to use a single point
            # if np.abs(diff) >= self.tolerance:
            #     # We are not close enough to use one existing table point
            #     UseSinglePoint = False
            # elif UseSinglePoint:
            #     # Check whether the closest_idx found here is the same as the previous
            #     if i>0 and closest_idx != prev_closest_idx:
            #         UseSinglePoint = False
            #     prev_closest_idx = closest_idx
            # Check if we are close enough to interpolate
            if np.abs(diff) >= self.interp_tolerance:
                Interpolate = False
            else:
                # closest existing input is within interpolation range
                if diff>=0:
                    # closest existing input is above or equal to requested input
                    idx_above = closest_idx
                    # check if we are at the boundary
                    if closest_idx == 0:
                        Interpolate = False
                    else:
                        idx_below = closest_idx-1
                        diff_below = (input - interp_table[idx_below,i])/input
                        # check if the node below is also within the interpolation tolerance
                        if diff_below>=self.interp_tolerance:
                            Interpolate = False       
                elif diff<0 and -diff<self.interp_tolerance:
                    # closest existing input is below requested input
                    idx_below = closest_idx
                    # check if we are at boundary
                    if closest_idx >= len(interp_table)-1:
                        Interpolate = False
                    else:
                        idx_above = closest_idx+1
                        diff_above = (interp_table[idx_above,i] - input)/input
                        # check if the node above is also within the interpolation tolerance
                        if diff_above>=self.interp_tolerance:
                            Interpolate = False
        return UseSinglePoint, Interpolate, closest_idx
    
    # return entries in interpolation table if we have inputs 
    def _query_interpolation_table(self,inputs,mode):
        # 
        # returns:
        # 0 if we are not close enough to any points in the interpolation table
        # otherwise, returns the desired interpolated value
        
        # Determine which table we are using
        if mode=='total':
            interp_table = self.total_cross_section_table
            interpolator = self.total_cross_section_interpolator
        elif mode=='differential':
            interp_table = self.differential_cross_section_table
            interpolator = self.differential_cross_section_interpolator
        else:
            print('Invalid interpolation table mode %s'%mode)
            exit(0)

        UseSinglePoint, Interpolate, closest_idx = self._interpolation_flags(inputs,mode)

        if UseSinglePoint:
            return interp_table[closest_idx,-1]
        elif Interpolate:
            return interpolator(inputs)
        else:
            return 0    
    
    # Fills the total and differential cross section tables within interp_tolerance
    def FillInterpolationTables(self,total=True,diff=True):
        Emin = (1.0+self.tolerance)*self.ups_case.Ethreshold
        Emax = np.max(self.total_cross_section_table[:,0])
        num_added_points = 0
        if total:
            E = Emin
            while E<Emax:
                UseSinglePoint,Interpolate,_ = self._interpolation_flags([E],'total')
                if not (UseSinglePoint or Interpolate):
                    xsec = self.ups_case.scalar_total_xsec(E)
                    self.total_cross_section_table = np.append(self.total_cross_section_table,[[E,xsec]],axis=0)
                    num_added_points+=1
                E *= (1+self.interp_tolerance)
        if diff:
            # interaction record to calculate Q2 bounds
            interaction = LI.dataclasses.InteractionRecord()
            interaction.signature.primary_type = self.GetPossiblePrimaries()[0] # only one primary
            interaction.signature.target_type = self.GetPossibleTargets()[0] # only one target
            interaction.target_mass = self.ups_case.MA
            E = Emin
            zmin,zmax = self.tolerance,1
            z = zmin
            while E < Emax:
                interaction.primary_momentum = [E,0,0,0]
                Q2min = self.Q2Min(interaction)
                Q2max = self.Q2Max(interaction)
                z = zmin
                while z < zmax:
                    Q2 = Q2min + z*(Q2max-Q2min)
                    UseSinglePoint,Interpolate,_ = self._interpolation_flags([E,z],'differential')
                    if not (UseSinglePoint or Interpolate):
                        dxsec = self.ups_case.diff_xsec_Q2(E, Q2).item()
                        self.differential_cross_section_table = np.append(self.differential_cross_section_table,[[E,z,dxsec]],axis=0)
                        num_added_points+=1
                    z *= (1+self.interp_tolerance)
                E *= (1+self.interp_tolerance)
        self._redefine_interpolation_objects(total=total,diff=diff)
        return num_added_points



    
    # Saves the tables for the scipy interpolation objects
    def SaveInterpolationTables(self,total=True,diff=True):
        if total:
            self._redefine_interpolation_objects(total=True)
            with open(self.table_dir + 'total_cross_sections.npy','wb') as f:
                np.save(f,self.total_cross_section_table)
        if diff:
            self._redefine_interpolation_objects(diff=True)
            with open(self.table_dir + 'differential_cross_sections.npy','wb') as f:
                np.save(f,self.differential_cross_section_table)
    
    ##### START METHODS FOR SERIALIZATION #########
    # def get_initialized_dict(config):
    #     # do the intitialization step
    #     pddn = PyDerivedDarkNews(config)
    #     return pddn.__dict__
    #     # return the conent of __dict__ for PyDerivedDarkNews

    # @staticmethod  
    # def get_config(self):
    #     return self.config
    ##### END METHODS FOR SERIALIZATION #########

    def GetPossiblePrimaries(self):
        return [Particle.ParticleType(self.ups_case.nu_projectile.pdgid)]
    
    def GetPossibleTargetsFromPrimary(self, primary_type):
        if Particle.ParticleType(self.ups_case.nu_projectile.pdgid) == primary_type:
            return [Particle.Particle.ParticleType(self.ups_case.nuclear_target.pdgid)]
        return []

    def GetPossibleTargets(self):
        return [Particle.ParticleType(self.ups_case.nuclear_target.pdgid)]

    def GetPossibleSignatures(self):
        signature = LI.dataclasses.InteractionSignature()
        signature.primary_type = Particle.ParticleType(self.ups_case.nu_projectile.pdgid)
        signature.target_type = Particle.ParticleType(self.ups_case.nuclear_target.pdgid)
        signature.secondary_types = []
        signature.secondary_types.append(Particle.ParticleType(self.ups_case.nu_upscattered.pdgid))
        signature.secondary_types.append(Particle.ParticleType(self.ups_case.nuclear_target.pdgid))
        return [signature]

    def GetPossibleSignaturesFromParents(self, primary_type, target_type):
        if (Particle.ParticleType(self.ups_case.nu_projectile.pdgid) == primary_type) and (Particle.ParticleType(self.ups_case.nuclear_target.pdgid) == target_type):
            signature = LI.dataclasses.InteractionSignature()
            signature.primary_type = Particle.ParticleType(self.ups_case.nu_projectile.pdgid)
            signature.target_type = Particle.ParticleType(self.ups_case.nuclear_target.pdgid)
            secondary_types = []
            secondary_types.append(Particle.ParticleType(self.ups_case.nu_upscattered.pdgid))
            secondary_types.append(Particle.ParticleType(self.ups_case.nuclear_target.pdgid))
            signature.secondary_types = secondary_types
            return [signature]
        return []

    def DifferentialCrossSection(self, arg1, target=None, energy=None, Q2=None):
        if type(arg1)==LI.dataclasses.InteractionRecord:
            interaction = arg1
            # Calculate Q2 assuming we are in the target rest frame
            m1sq = interaction.primary_momentum[0]**2 - np.sum([p**2 for p in interaction.primary_momentum[1:]])
            m3sq = interaction.secondary_momenta[0][0]**2 - np.sum([p**2 for p in interaction.secondary_momenta[0][1:]])
            p1p3 = interaction.primary_momentum[0]*interaction.secondary_momenta[0][0] - np.sum(p1*p3 for p1,p3 in zip(interaction.primary_momentum[1:],interaction.secondary_momenta[0][1:]))
            Q2 = -(m1sq + m3sq - 2*p1p3)
            energy = interaction.primary_momentum[0]
        else:
            primary=arg1
            interaction = LI.dataclasses.InteractionRecord()
            interaction.signature.primary_type = primary
            interaction.signature.target_type = target
            interaction.primary_momentum = [energy,0,0,0]
            interaction.target_mass = self.ups_case.MA
        if interaction.signature.primary_type != self.ups_case.nu_projectile:
            return 0
        if interaction.primary_momentum[0] < self.InteractionThreshold(interaction):
            return 0
        Q2min = self.Q2Min(interaction)
        Q2max = self.Q2Max(interaction)
        z = (Q2-Q2min)/(Q2max-Q2min)

        if self.interpolate_differential:
            # Check if we can interpolate
            val = self._query_interpolation_table([energy,z],mode='differential')
            if val > 0:
                # we have recovered the differential cross section from the interpolation table
                return val
        
        # If we have reached this block, we must compute the differential cross section using DarkNews
        dxsec = self.ups_case.diff_xsec_Q2(energy, Q2).item()
        self.differential_cross_section_table = np.append(self.differential_cross_section_table,[[energy,z,dxsec]],axis=0)
        if self.interpolate_differential: self._redefine_interpolation_objects(diff=True)
        return dxsec
    
    def SetUpscatteringMasses(self, interaction):
        interaction.primary_mass = 0
        interaction.target_mass = self.ups_case.MA
        secondary_masses = []
        secondary_masses.append(self.ups_case.m_ups)
        secondary_masses.append(self.ups_case.MA)
        interaction.secondary_masses = secondary_masses
        self.m_ups = self.ups_case.m_ups
        self.m_target = self.ups_case.MA

    def SetUpscatteringHelicities(self, interaction):
        secondary_helicities = []
        secondary_helicities.append(self.ups_case.h_upscattered * interaction.primary_helicity)
        secondary_helicities.append(interaction.target_helicity)
        interaction.secondary_helicity = secondary_helicities
        self.h_ups = self.ups_case.m_ups
        self.h_target = self.ups_case.MA
    
    def TotalCrossSection(self, arg1, energy=None, target=None):
        # Handle overloaded arguments
        if type(arg1==LI.dataclasses.InteractionRecord):
            primary = arg1.signature.primary_type
            energy = arg1.primary_momentum[0]
            target = arg1.signature.target_type
        elif energy is not None and target is not None:
            primary = arg1
        else:
            print('Incorrect function call to TotalCrossSection!')
            exit(0)
        if primary != self.ups_case.nu_projectile:
            return 0
        interaction = LI.dataclasses.InteractionRecord()
        interaction.signature.primary_type = primary
        interaction.signature.target_type = target
        interaction.primary_momentum[0] = energy
        if energy < self.InteractionThreshold(interaction):
            return 0
        
        # Check if we can interpolate
        val = self._query_interpolation_table([energy],mode='total')
        if val > 0:
            # we have recovered the cross section from the interpolation table
            return val
        
        # If we have reached this block, we must compute the cross section using DarkNews
        xsec = self.ups_case.scalar_total_xsec(energy)
        self.total_cross_section_table = np.append(self.total_cross_section_table,[[energy,xsec]],axis=0)
        self._redefine_interpolation_objects(total=True)
        return xsec
        

    def InteractionThreshold(self, interaction):
        return self.ups_case.Ethreshold

    def Q2Min(self, interaction):
        return phase_space.upscattering_Q2min(interaction.primary_momentum[0], self.ups_case.m_ups, interaction.target_mass)

    def Q2Max(self, interaction):
        return phase_space.upscattering_Q2max(interaction.primary_momentum[0], self.ups_case.m_ups, interaction.target_mass)

# A class representing a single decay_case DarkNews class
# Only handles methods concerning the decay part
class PyDarkNewsDecay(DarkNewsDecay):

    def __init__(self,
                 dec_case,
                 table_dir=None):
        DarkNewsDecay.__init__(self) # C++ constructor
        self.dec_case = dec_case
        self.table_dir = table_dir
        
        # Some variables for storing the decay phase space integrator
        self.decay_integrator = None
        self.decay_norm = None
        self.PS_samples = None
        self.PS_weights = None
        self.PS_weights_CDF = None
        self.total_width = None

        if table_dir is None: 
            print('No table_dir specified; will sample from new VEGAS integrator for each decay')
            print('WARNING: this will siginficantly slow down event generation')
            return

        # Make the table directory where will we store cross section integrators
        table_dir_exists = False
        if(os.path.exists(self.table_dir)):
            #print("Directory '%s' already exists"%self.table_dir)
            table_dir_exists = True
        else:
            try:
                os.makedirs(self.table_dir, exist_ok = False)
                print("Directory '%s' created successfully"%self.table_dir)
            except OSError as error:
                print("Directory '%s' cannot be created"%self.table_dir)
                exit(0)
        
        # Look in table dir for existing decay integrator + normalization info
        if table_dir_exists:
            self.SetIntegratorAndNorm()

    def SetIntegratorAndNorm(self):
        # Try to find the decay integrator
        int_file = self.table_dir + 'decay_integrator.pkl'
        if os.path.isfile(int_file):
            with open(int_file,'rb') as ifile:
                _,self.decay_integrator = pickle.load(ifile)
        # Try to find the normalization information
        norm_file = self.table_dir + 'decay_norm.json'
        if os.path.isfile(norm_file):
            with open(norm_file,) as nfile:
                self.decay_norm = json.load(nfile)        

    ##### START METHODS FOR SERIALIZATION #########
    # def get_initialized_dict(config):
    #     # do the intitialization step
    #     pddn = PyDerivedDarkNews(config)
    #     return pddn.__dict__
    #     # return the conent of __dict__ for PyDerivedDarkNews

    # @staticmethod  
    # def get_config(self):
    #     return self.config
    ##### END METHODS FOR SERIALIZATION #########

    def GetPossibleSignatures(self):
        signature = LI.dataclasses.InteractionSignature()
        signature.primary_type = Particle.ParticleType(self.dec_case.nu_parent.pdgid)
        signature.target_type = Particle.ParticleType.Decay
        secondary_types = []
        secondary_types.append(Particle.ParticleType(self.dec_case.nu_daughter.pdgid))
        for secondary in self.dec_case.secondaries:
            secondary_types.append(Particle.ParticleType(secondary.pdgid))
        signature.secondary_types = secondary_types
        return [signature]

    def GetPossibleSignaturesFromParent(self, primary_type):
        if (Particle.ParticleType(self.dec_case.nu_parent.pdgid) == primary_type):
            signature = LI.dataclasses.InteractionSignature()
            signature.primary_type = Particle.ParticleType(self.dec_case.nu_parent.pdgid)
            signature.target_type = Particle.ParticleType.Decay
            secondary_types = []
            secondary_types.append(Particle.ParticleType(self.dec_case.nu_daughter.pdgid))
            for secondary in self.dec_case.secondaries:
                secondary_types.append(Particle.ParticleType(secondary.pdgid))
            signature.secondary_types = secondary_types
            return [signature]
        return []

    def DifferentialDecayWidth(self, record):

        # Momentum variables of HNL necessary for calculating decay phase space
        PN = np.array(record.primary_momentum)
        
        if type(self.dec_case)==FermionSinglePhotonDecay:
            gamma_idx = 0
            for secondary in record.signature.secondary_types:
                if secondary == LI.dataclasses.Particle.ParticleType.Gamma:
                    break
                gamma_idx += 1
            if gamma_idx >= len(record.signature.secondary_types):
                print('No gamma found in the list of secondaries!')
                exit(0)
            
            Pgamma = np.array(record.secondary_momenta[gamma_idx])
            momenta = np.expand_dims(PN,0),np.expand_dims(Pgamma,0)

        elif type(self.dec_case)==FermionDileptonDecay:
            lepminus_idx = -1
            lepplus_idx = -1
            nu_idx = -1
            for idx,secondary in enumerate(record.signature.secondary_types):
                if secondary in [LI.dataclasses.Particle.ParticleType.EMinus,
                                 LI.dataclasses.Particle.ParticleType.MuMinus,
                                 LI.dataclasses.Particle.ParticleType.TauMinus]:
                    lepminus_idx = idx
                elif secondary in [LI.dataclasses.Particle.ParticleType.EPlus,
                                   LI.dataclasses.Particle.ParticleType.MuPlus,
                                   LI.dataclasses.Particle.ParticleType.TauPlus]:
                    lepplus_idx = idx
                else:
                    nu_idx = idx
            if -1 in [lepminus_idx,lepplus_idx,nu_idx]:
                print('Couldn\'t find two leptons and a neutrino in the final state!')
                exit(0)
            Pnu = np.array(record.secondary_momenta[nu_idx])
            Plepminus = np.array(record.secondary_momenta[lepminus_idx])
            Plepplus = np.array(record.secondary_momenta[lepplus_idx])
            momenta = np.expand_dims(PN,0),np.expand_dims(Plepminus,0),np.expand_dims(Plepplus,0),np.expand_dims(Pnu,0)
        else:
            print('%s is not a valid decay class type!'%type(self.dec_case))
            exit(0)
        return self.dec_case.differential_width(momenta)
    
    def TotalDecayWidth(self, arg1):
        if type(arg1)==LI.dataclasses.InteractionRecord:
            primary = arg1.signature.primary_type
        elif type(arg1)==LI.dataclasses.Particle.ParticleType:
            primary = arg1
        else:
            print('Incorrect function call to TotalDecayWidth!')
            exit(0)
        if primary != self.dec_case.nu_parent:
            return 0
        if self.total_width is None:
            # Need to set the total width
            if type(self.dec_case) == FermionDileptonDecay and \
               (self.dec_case.vector_off_shell and self.dec_case.scalar_off_shell):
                # total width calculation requires evaluating an integral
                if (self.decay_integrator is None or self.decay_norm is None):
                        # We need to initialize a new VEGAS integrator in DarkNews
                        int_file = self.table_dir + 'decay_integrator.pkl'
                        norm_file = self.table_dir + 'decay_norm.json'
                        self.total_width = self.dec_case.total_width(savefile_norm=norm_file,savefile_dec=int_file)
                        self.SetIntegratorAndNorm()
                else:
                   self.total_width = self.decay_integrator["diff_decay_rate_0"].mean * self.decay_norm["diff_decay_rate_0"]
            else:
                self.total_width = self.dec_case.total_width()
        return self.total_width
        
    
    def TotalDecayWidthForFinalState(self,record):
        sig = self.GetPossibleSignatures()[0]
        if (record.signature.primary_type != sig.primary_type) or \
           (record.signature.target_type != sig.target_type) or \
           (len(record.signature.secondary_types) != len(sig.secondary_types)) or \
           (np.any([record.signature.secondary_types[i] != sig.secondary_types[i] for i in range(len(sig.secondary_types))])):
            return 0
        return self.dec_case.total_width()
    
    def DensityVariables(self):
        if type(self.dec_case)==FermionSinglePhotonDecay:
            return "cost"
        elif type(self.dec_case)==FermionDileptonDecay:
            if self.dec_case.vector_on_shell and self.dec_case.scalar_on_shell:
                print('Can\'t have both the scalar and vector on shell')
                exit(0)
            elif (self.dec_case.vector_on_shell and self.dec_case.scalar_off_shell) or \
                 (self.dec_case.vector_off_shell and self.dec_case.scalar_on_shell):
                return "cost"
            elif self.dec_case.vector_off_shell and self.dec_case.scalar_off_shell:
                return "t,u,c3,phi34"
        else:
            print('%s is not a valid decay class type!'%type(self.dec_case))
            exit(0)
        return ""
    
    def GetPSSample(self, random):
        
        # Make the PS weight CDF if that hasn't been done
        if self.PS_weights_CDF is None:
            self.PS_weights_CDF = np.cumsum(self.PS_weights)
        
        # Random number to determine 
        x = random.Uniform(0,self.PS_weights_CDF[-1])

        # find first instance of a CDF entry greater than x
        PSidx = np.argmax(x - self.PS_weights_CDF <= 0)
        return self.PS_samples[:,PSidx]

    def SampleRecordFromDarkNews(self,record,random):
        
        # First, make sure we have PS samples and weights
        if self.PS_samples is None or self.PS_weights is None:
            # We need to generate new PS samples
            if self.decay_integrator is None or self.decay_norm is None:
                # We need to initialize a new VEGAS integrator in DarkNews
                int_file = self.table_dir + 'decay_integrator.pkl'
                norm_file = self.table_dir + 'decay_norm.json'
                self.PS_samples, PS_weights_dict = self.dec_case.SamplePS(savefile_norm=norm_file,savefile_dec=int_file)
                self.PS_weights = PS_weights_dict['diff_decay_rate_0']
                self.SetIntegratorAndNorm()
            else:
                # We already have an integrator, we just need new PS samples
                self.PS_samples, PS_weights_dict = self.dec_case.SamplePS(existing_integrator=self.decay_integrator)
                self.PS_weights = PS_weights_dict['diff_decay_rate_0']
        
        # Now we must sample an PS point on the hypercube
        PS = self.GetPSSample(random)

        # Find the four-momenta associated with this point
        # Expand dims required to call DarkNews function on signle sample
        four_momenta = get_decay_momenta_from_vegas_samples(np.expand_dims(PS,0),self.dec_case,np.expand_dims(np.array(record.primary_momentum),0))
        
        if type(self.dec_case)==FermionSinglePhotonDecay:
            gamma_idx = 0
            for secondary in record.signature.secondary_types:
                if secondary == LI.dataclasses.Particle.ParticleType.Gamma:
                    break
                gamma_idx += 1
            if gamma_idx >= len(record.signature.secondary_types):
                print('No gamma found in the list of secondaries!')
                exit(0)
            nu_idx = 1 - gamma_idx
            secondary_momenta = []
            secondary_momenta.insert(gamma_idx,list(np.squeeze(four_momenta["P_decay_photon"])))
            secondary_momenta.insert(nu_idx,list(np.squeeze(four_momenta["P_decay_N_daughter"])))
            record.secondary_momenta = secondary_momenta
        elif type(self.dec_case)==FermionDileptonDecay:
            lepminus_idx = -1
            lepplus_idx = -1
            nu_idx = -1
            for idx,secondary in enumerate(record.signature.secondary_types):
                if secondary in [LI.dataclasses.Particle.ParticleType.EMinus,
                                 LI.dataclasses.Particle.ParticleType.MuMinus,
                                 LI.dataclasses.Particle.ParticleType.TauMinus]:
                    lepminus_idx = idx
                elif secondary in [LI.dataclasses.Particle.ParticleType.EPlus,
                                   LI.dataclasses.Particle.ParticleType.MuPlus,
                                   LI.dataclasses.Particle.ParticleType.TauPlus]:
                    lepplus_idx = idx
                else:
                    nu_idx = idx
            if -1 in [lepminus_idx,lepplus_idx,nu_idx]:
                print([lepminus_idx,lepplus_idx,nu_idx])
                print(record.signature.secondary_types)
                print('Couldn\'t find two leptons and a neutrino in the final state!')
                exit(0)
            secondary_momenta = []
            secondary_momenta.insert(lepminus_idx,list(np.squeeze(four_momenta["P_decay_ell_minus"])))
            secondary_momenta.insert(lepplus_idx,list(np.squeeze(four_momenta["P_decay_ell_plus"])))
            secondary_momenta.insert(nu_idx,list(np.squeeze(four_momenta["P_decay_N_daughter"])))
            record.secondary_momenta = secondary_momenta
        return record