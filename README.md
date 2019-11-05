# LeptonInjector

LeptonInjector is a group of modules used to create events in IceCube. This code represents a standalone version of the original LeptonInjector which has been trimmed of the proprietary Icetray dependencies. It is currently a work in progress, and is *not* in a functional state! 

The code is very nearly in a functional state and is in the final stages of debugging. 

To use it, you will

    1. Prepare a Injector object (or list of Injector objects). These are represented as `MinimalInjectionConfiguration` objects in the c++ code. 

    2. Use one injector object, along with several generation parameters, to create a Controller object. These were called MultiLeptonInjector in the original code. 

    3. Add more injectors to the controller object using the add injector function. Verify that the controller is properly configured.

    4. Call Controller.Execute(). This will run the simulation. 

# Dependencies

All of the dependencies are already installed on the CVMFS environments on the IceCube Cobalt testbeds. For local installations, you may need to install the below. You only 'may' need to, because you might already have these. All three are definitely necessary. 

This code requires the `HDF5` C libraries. Read more about it here: https://portal.hdfgroup.org/display/support. These libraries are, of course, used to save the data files. 

It requires `BOOST`, which can be installed as easily as typing `sudo apt-get install libboost-all-dev` on linux machines. There's probably a homebrew version for mac. Boost is primarily needed to compile the python bindings or this software. 

It also requires Photospline to create and to read cross sections. Read more about it, and its installation at https://github.com/IceCubeOpenSource/photospline. Note that Photospline has dependencies that you will need that are not listed here. 

Photospline requries `SuiteSparse`, which can be downloaded here http://faculty.cse.tamu.edu/davis/suitesparse.html

# Included Dependencies

These are not part of LeptonInjector, but are included for its functionality. 

* I3CrossSections: provies the tools for sampling DIS and GR cross sections. 

* Earthmodel Services: provides the PREM for column depth calculations. 

# Compilation and Installation

This project uses cmake to compile. First, go to a folder where you would like to build and compile lepton injector. Run 

`git clone git@github.com:IceCubeOpenSource/LeptonInjector.git`

to download the source code. Then, `mv` the folder that is created to rename it to `source`. We will be trying to keep our source, build, and install directories separate. Now

`mkdir build` and `mkdir install`

to prepare. `cd ./build` to move into the build directory. Now, we call cmake

`cmake -DCMAKE_INSTALL_PREFIX=../install ../source`

This tells cmake to install the shared objects in the `install` directory we just made. Cmake prepares a `Makefile` which calls the `g++` compiler with the necessary instructions to... compile. So now you'll call

`make -j4 && make install`

to build the project and install the project. Now you need to set all the environmental variables so this actually works. You will be adding this to your .bashrc or .bash_profile. 

`export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/your/install/path/

# Structure
The code base is divided into several files. 
* Constants: a header defining various constants. 
* Controller: implements a class for managing the simulation
* DataWriter: writes event properties and MCTrees to an HDF5 file
* EventProps: implements a few structures used to write events in the hdf5 file. 
* h5write: may be renamed soon. This will be used to write the configurations onto a file
* LeptonInjector (the file): defines the Injector objects described above in addition to several configuration objects and event generators 
* Particle: simple implementation of particles. Includes a big enum. 
* Random: object for random number sampling.

# Cross Sections
More to come on this front later. You will need cross sections, saved as fits files. Maybe we should upload examples XS to the repo?
