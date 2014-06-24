aces4
=====

Parallel Computational Chemistry Application

Built in C, C++ & Fortran & Java.

Successor to ACESIII [http://www.qtp.ufl.edu/]. 

The ACESIII system is described in the paper titled "A Block-Oriented Language and Runtime System for Tensor Algebra with Very Large Arrays" [http://dx.doi.org/10.1109/SC.2010.3]

See the Wiki (https://github.com/UFParLab/aces4/wiki) for more information


Prerequisites
-------------
```
java
GNU (gcc > v4.8, g++, gfortran, libblas-dev, liblapack-dev) or
Intel compilers & Math Libraries
```

If checking out from git:
```
automake
autoconf
libtool
```

You will need ```doxygen ``` to make documentation.

Timer resolution can be improved with ```PAPI``` (http://icl.cs.utk.edu/papi/). 
Can also use ```TAU``` (http://www.cs.uoregon.edu/research/tau) for detailed profile & trace information.

Google tests v1.7 is required to run tests. 
Get it from http://googletest.googlecode.com/files/gtest-1.7.0.zip. 
Unzip it into the ```test/``` directory.


To Compile & Test
-----------------

In the root of the project, say:
```BASH
./autogen.sh
mkdir BUILD
cd BUILD
../configure
make 
```
To run the tests (in the BUILD directory):
```BASH
make check
```
To make doxygen documentation pdf & html (in the BUILD directory):
```BASH
make doxygen-run
```


MPI Version
-----------
There are 2 versions of ACES4 - ```a single node``` and a ```MPI multinode version```.
By default, if an MPI compiler can be detected, the mpi version will be compiled. 
To disable this default behaviour, use the ```--with-mpi=no``` switch when configuring
To forcefully build the multinode version, use the ```--with-mpi=yes``` switch.


PAPI Timers
-----------
Super instructions can be profiled with PAPI wall time timers. To enable their use, while configuring, use the ```--with-papi=PATH``` option, for instance on the UF HiperGator machine: ```../configure --with-papi=/apps/papi/5.1.0```

If PAPI is not "installed" by your admin, you can download the tarball in your home directory, build it and "install" it in your home directory and point to that directory when configuring.


TAU Timers
----------
SIAL Programs & Super Instructions can be profiled & Traced with TAU. To enable them, use the --with-tau option. For instace, on the UF HiperGator machine:
```BASH
../configure --with-tau=/apps/tau/2.21.4/openmpi-1.6 # (for MPI)
../configure --with-tau=/apps/tau/2.21.4/serial      # (for Serial).
```
If TAU is not "installed", you can download the tarball, compile (and install) it and have the ```--with-tau``` point to that directory.

Please set the Environment variable ```TAU_CALLPATH``` to ```0``` to only view time spent per superinstruction. This will hide the C++/Fortran call stack associated with a super-instruction. Setting ```TAU_PROFILE``` to ```1``` will collect profile information and setting ```TAU_TRACE``` to ```1``` will collect trace data. Documentation for TAU can be found here : ```http://www.cs.uoregon.edu/research/tau/docs.php```.

Note that if TAU is used, builtin aces4 profile information collection is disabled.


CUDA Support
------------
Certain super-instructions are accelerated through CUDA. To enable their use, configure with the ```--with-cuda``` switch. For instance, on the UF HiperGator machine: ```../configure --with-cuda=/opt/cuda```

Please note that on your system, you may need to load in the module first and figure out the path. You can also just say ``` ../configure --with-cuda ``` and the configure script will attempt to find the cuda header and libraries in default locations. It will complain if it cant, in which case, you'll need to use the ```--with-cuda=PATH``` switch.


Developer Mode (More logging)
-----------------------------
Use the ```--enable-development``` switch to configure to get extra logging.


Tools
-----
Please explore the ```tool/``` directory.
