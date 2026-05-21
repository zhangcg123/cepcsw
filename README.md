# [CEPCSW](https://cepc.github.io/CEPCSW/)

[![pipeline status at GitLab](https://code.ihep.ac.cn/cepc/CEPCSW/badges/master/pipeline.svg)](https://code.ihep.ac.cn/cepc/CEPCSW/-/commits/master)
[![CI at GitHub](https://github.com/cepc/CEPCSW/workflows/CI/badge.svg?branch=master)](https://github.com/cepc/CEPCSW/actions)

CEPC offline software prototype based on [Key4hep](https://github.com/key4hep).

## Quick start

SSH to lxlogin (Alma Linux 9).

Before run following commands, please make sure you setup the CVMFS:

```
$ git clone git@code.ihep.ac.cn:cepc/CEPCSW.git
$ cd CEPCSW
$ git checkout master # branch name
$ source setup.sh
$ ./build.sh
$ source setup.sh
$ ./run.sh Examples/options/helloalg.py
```

## Packages

* Examples: For new comers and users

* Detector: Geometry

* Generator: Physics Generator

* Simulation: Detector Simulation

* Digitization: Digitization

* Reconstruction: Reconstruction

