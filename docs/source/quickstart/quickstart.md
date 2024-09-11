# Quickstart

## Introduction

CEPCSW is developed based on the Key4hep software stack. 

It consists three major parts:
* The applications, such as physics generators, detector simulation, digitization, event reconstruction etc.
* The core software, such as the Gaudi framework, geometry description using DD4hep, event data model using EDM4hep, event data memory management using k4FWCore.
* The underlying libraries and development tools, such as ROOT, Geant4, CLHEP, CMake etc. 

## Setup at IHEP cluster

To use CEPCSW, users can use the software release deployed at IHEP's CVMFS or build from source code. 

Currently, the libraries are built in `CentOS 7`. If you are using other different operating systems, you can consider to use the container technologies, such as Docker, Apptainer. 

### SSH login
If you already have an IHEP AFS account, you can first login using an SSH client, such as OpenSSH:
```bash
$ ssh -Y username@lxlogin.ihep.ac.cn
```

###  Start CentOS 7 container
Then, you need to **start the container** with following command:
```bash
$ /cvmfs/container.ihep.ac.cn/bin/hep_container shell CentOS7
```

After that, you will see the prompt `Singularity>`. Make sure you are using the correct OS:
```bash
Singularity> cat /etc/os-release
NAME="CentOS Linux"
VERSION="7 (Core)"
ID="centos"
ID_LIKE="rhel fedora"
VERSION_ID="7"
PRETTY_NAME="CentOS Linux 7 (Core)"
ANSI_COLOR="0;31"
CPE_NAME="cpe:/o:centos:centos:7"
HOME_URL="https://www.centos.org/"
BUG_REPORT_URL="https://bugs.centos.org/"

CENTOS_MANTISBT_PROJECT="CentOS-7"
CENTOS_MANTISBT_PROJECT_VERSION="7"
REDHAT_SUPPORT_PRODUCT="centos"
REDHAT_SUPPORT_PRODUCT_VERSION="7"
```

### Use the official release
As CEPCSW is already deployed at IHEP CVMFS. You can use it as following:

```bash
Singularity> source /cvmfs/cepcsw.ihep.ac.cn/prototype/releases/tdr24.5.0/CEPCSW/setup.sh
INFO: Setup CEPCSW externals: /cvmfs/cepcsw.ihep.ac.cn/prototype/releases/externals/103.0.2/setup-103.0.2.sh
INFO: Setup CEPCSW: /cvmfs/cepcsw.ihep.ac.cn/prototype/releases/tdr24.5.0/CEPCSW/InstallArea
```

From the output, you will know the current version of external libraries is 103.0.2. This is based on the LCG 103.

To start a simulation and reconstruction with `TDR_o1_v1`:
```bash
Singularity> gaudirun.py $DETCRDROOT/scripts/TDR_o1_v01/sim.py
Singularity> gaudirun.py $DETCRDROOT/scripts/TDR_o1_v01/tracking.py
```

### Use the main branch
Due to fast iterations in CEPCSW, you need to know how to build from source code. 

Before you running the code, please make sure that you already setup the ssh public key in IHEP GitLab.

```
$ git clone git@code.ihep.ac.cn:cepc/CEPCSW.git
$ cd CEPCSW
$ source setup.sh
$ ./build.sh
$ source setup.sh # This is necessary for the first time
```

## Optional: Setup at your local computer
If you need to run CEPCSW in your local machines, please make sure that you already setup the network:
* The docker image. Need to download once.   
* The cvmfs file system. All the external libraries are accessed via `/cvmfs`. 

### Install Docker
You need to install the `Docker` runtime if it is not installed yet. 
* The Docker documentation provides instructions: https://docs.docker.com/engine/install/

After you install the `Docker`, please make sure it is able to download the images:
```bash
$ docker run hello-world

Hello from Docker!
This message shows that your installation appears to be working correctly.

To generate this message, Docker took the following steps:
 1. The Docker client contacted the Docker daemon.
 2. The Docker daemon pulled the "hello-world" image from the Docker Hub.
    (amd64)
 3. The Docker daemon created a new container from that image which runs the
    executable that produces the output you are currently reading.
 4. The Docker daemon streamed that output to the Docker client, which sent it
    to your terminal.

To try something more ambitious, you can run an Ubuntu container with:
 $ docker run -it ubuntu bash

Share images, automate workflows, and more with a free Docker ID:
 https://hub.docker.com/

For more examples and ideas, visit:
 https://docs.docker.com/get-started/
```

If you encounter a network issue, please consider to use proxy if necessary. Here is an example if you are using systemd to manage Docker:
```bash
$ cat /etc/systemd/system/docker.service.d/http-proxy.conf
[Service]
Environment="HTTP_PROXY=socks5://localhost:1080"
Environment="HTTPS_PROXY=socks5://localhost:1080"
```
You need to create this file if not exists and restart the docker.

### Two options to install CVMFS
After you are able to use docker, then you need to install the CVMFS clients. 
* CVMFS client installation guide: https://cvmfs.readthedocs.io/en/stable/cpt-quickstart.html

There are two options:
* One is that install CVMFS in your machine directly.
* Another is that install CVMFS in the Docker container. 

We provide two images for the two cases:
* `cepc/cepcsw:el7`
* `cepc/cepcsw-cvmfs:el7`

Please note that: 
* CVMFS will cache the files you accessed in the corresponding environment. 
* If you prefer the second image, then you will need to cache the data when a new container starts. 


### CVMFS in local machine

For an example, the local machine is Ubuntu 24.04 and you want to access CVMFS from your machine. Then you need to install CVMFS first:
```bash
$ wget https://ecsft.cern.ch/dist/cvmfs/cvmfs-release/cvmfs-release-latest_all.deb
$ sudo dpkg -i cvmfs-release-latest_all.deb
$ rm -f cvmfs-release-latest_all.deb
$ sudo apt-get update
$ sudo apt-get install cvmfs
```

In order to access IHEP CVMFS in your machine directly, you need to make sure keys are installed.
```bash
$ sudo mkdir /etc/cvmfs/keys/ihep.ac.cn
$ sudo curl -o /etc/cvmfs/keys/ihep.ac.cn/ihep.ac.cn.pub http://cvmfs-stratum-one.ihep.ac.cn/cvmfs/software/client_configure/ihep.ac.cn/ihep.ac.cn.pub
$ sudo curl -o /etc/cvmfs/domain.d/ihep.ac.cn.conf http://cvmfs-stratum-one.ihep.ac.cn/cvmfs/software/client_configure/ihep.ac.cn.conf
$ echo "CVMFS_REPOSITORIES='sft.cern.ch,cepcsw.ihep.ac.cn,container.ihep.ac.cn'" | sudo tee    /etc/cvmfs/default.local
$ echo "CVMFS_HTTP_PROXY=DIRECT"                                                 | sudo tee -a /etc/cvmfs/default.local
$ sudo cvmfs_config setup
```

After you see CVMFS is setup without issues, you can try to access the CEPCSW:
```bash
ls /cvmfs/cepcsw.ihep.ac.cn/prototype/releases/tdr24.5.0
CEPCSW  CEPCSWData
```

Now, you can start the container with following long command:
```bash
$ docker run --privileged --rm -i -t -v /home:/home -v /cvmfs/sft.cern.ch:/cvmfs/sft.cern.ch -v /cvmfs/geant4.cern.ch:/cvmfs/geant4.cern.ch -v /cvmfs/cepcsw.ihep.ac.cn:/cvmfs/cepcsw.ihep.ac.cn -v /cvmfs/container.ihep.ac.cn:/cvmfs/container.ihep.ac.cn cepc/cepcsw:el7 /bin/bash
```

The option `-v` allows the Docker container to access the directories in the local machine. 

### CVMFS inside Docker container
We also provide a Docker image with cvmfs client installed inside. 

```bash
$ docker run --privileged --rm -i -t -v /home:/home cepc/cepcsw-cvmfs:el7 /bin/bash
```

Now, you should be in the container. However, the cvmfs is not setup automatically. You need to mount the repositories manually. Here is an example:
```bash
[ -d /cvmfs/cepcsw.ihep.ac.cn ] || mkdir /cvmfs/cepcsw.ihep.ac.cn && mount -t cvmfs cepcsw.ihep.ac.cn /cvmfs/cepcsw.ihep.ac.cn
[ -d /cvmfs/sft.cern.ch ] || mkdir /cvmfs/sft.cern.ch && mount -t cvmfs sft.cern.ch /cvmfs/sft.cern.ch
[ -d /cvmfs/geant4.cern.ch ] || mkdir /cvmfs/geant4.cern.ch && mount -t cvmfs geant4.cern.ch /cvmfs/geant4.cern.ch
```

Then, you should be able access the necessary CVMFS repositories. 

For example, setting up an existing CEPCSW:
```bash
$ source /cvmfs/cepcsw.ihep.ac.cn/prototype/releases/tdr24.5.0/CEPCSW/setup.sh
INFO: Setup CEPCSW externals: /cvmfs/cepcsw.ihep.ac.cn/prototype/releases/externals/103.0.2/setup-103.0.2.sh
INFO: Setup CEPCSW: /cvmfs/cepcsw.ihep.ac.cn/prototype/releases/tdr24.5.0/CEPCSW/InstallArea
```

Run a simulation:
```bash
$ gaudirun.py $DETCRDROOT/scripts/TDR_o1_v01/sim.py
```