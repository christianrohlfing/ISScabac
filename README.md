# General information

This software package provides
* code for reproducing the experimental results of the ICASSP 2018 paper "Adaptive coding of non-negative factorization parameters with application to informed source separation" by Max Bläser, Christian Rohlfing, Yingbo Gao and Mathias Wien.
* additionally a MATLAB interface to the context-based adaptive binary arithmetic coding (CABAC) engine
  1. We extracted the CABAC engine out of the [HEVC test model (HM)](https://hevc.hhi.fraunhofer.de/svn/svn_HEVCSoftware/) by the Joint Collaborative Team on Video Coding (JCT-VC).
  2. Building on this, we provide a MATLAB MEX interface.
  3. Finally, we provide a MATLAB CABAC class for easy usage.

# Install
1. Clone or download this repository.
2. Under linux, run MATLAB with `export LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libstdc++.so.6 [INSERT_MATLAB_PATH_HERE]/bin/matlab &`.
3. To run our code of our proposed ISS method, go to the `ISS` folder and run `ISS.m`.
4. To run a simple demo explaining the basic usage of CABAC, go directly to the `CABAC` folder and run `cabacDemo.m`.
5. We provided already compiled MEX files for Windows (tested on Windows 10) and Linux (tested on Ubuntu 16.04). However, if you need to compile the MEX file again, use the following command `mex CXXFLAGS="\$CXXFLAGS -std=c++11" SimpleCABACMex.cpp`.

# Publication
If you use this software, please reference the following publication:

    @inproceedings{blaeser2018cabac,
        TITLE = {Adaptive coding of non-negative factorization parameters with application to informed source separation},
        AUTHOR = {Bl{\"a}ser, M., Rohlfing, C., Gao, Y., Wien, M.},
        BOOKTITLE = {{43rd International Conference on Acoustics, Speech and Signal Processing (ICASSP)}},
        ADDRESS = {Seoul, South Korea},
        PUBLISHER = {{IEEE}},
        SERIES = {Proceedings of the 43rd International Conference on Acoustics, Speech and Signal Processing (ICASSP)},
        YEAR = {2018},
        COMMENT = {submitted},
    }
