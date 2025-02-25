##############################################################################
# Docker image for CEPCSW
##############################################################################

# # Instruction
# To build the docker image:
#   $ docker build -t cepc/cepcsw:el9 .
# Or with CVMFS installed also
#   $ docker build -t cepc/cepcsw-cvmfs:el9 . --build-arg CVMFSMOD=INSIDE
#
# To publish it to DockerHub:
#   $ docker push cepc/cepcsw:el9
#   $ docker push cepc/cepcsw-cvmfs:el9

FROM almalinux:9

ARG CVMFSMOD

RUN dnf install -y 'dnf-command(config-manager)'
RUN dnf config-manager --set-enabled crb

RUN dnf install -y epel-release \
 && dnf install -y almalinux-release-devel \
 && dnf install -y sudo \
 && dnf install -y wget

##############################################################################
# CVMFS client
##############################################################################
# this will install the cvmfs client inside the container
# User needs to run with --privileged mode.
RUN if [ "$CVMFSMOD" = "INSIDE" ]; then \
  sudo yum install -y https://ecsft.cern.ch/dist/cvmfs/cvmfs-release/cvmfs-release-latest.noarch.rpm \
  && sudo yum install -y cvmfs \
  && sudo mkdir /etc/cvmfs/keys/ihep.ac.cn \
  && sudo curl -o /etc/cvmfs/keys/ihep.ac.cn/ihep.ac.cn.pub http://cvmfs-stratum-one.ihep.ac.cn/cvmfs/software/client_configure/ihep.ac.cn/ihep.ac.cn.pub \
  && sudo curl -o /etc/cvmfs/domain.d/ihep.ac.cn.conf http://cvmfs-stratum-one.ihep.ac.cn/cvmfs/software/client_configure/ihep.ac.cn.conf \
  && echo "CVMFS_REPOSITORIES='sft.cern.ch,cepcsw.ihep.ac.cn,geant4.cern.ch'" | sudo tee    /etc/cvmfs/default.local \
  && echo "CVMFS_HTTP_PROXY=DIRECT"                                                 | sudo tee -a /etc/cvmfs/default.local \
  && cat /etc/cvmfs/default.local \
  && sudo mkdir -p /cvmfs/sft.cern.ch \
  && sudo mkdir -p /cvmfs/cepcsw.ihep.ac.cn \
  && sudo mkdir -p /cvmfs/geant4.cern.ch; \
  fi

# START Container:
# # docker run  --privileged  --rm -i -t cepc/cepcsw-cvmfs:el9 /bin/bash
# Due to the fuse issue, following commands need to be run inside container when --privileged is specified
# $ mount -t cvmfs sft.cern.ch /cvmfs/sft.cern.ch
# $ mount -t cvmfs geant4.cern.ch /cvmfs/geant4.cern.ch
# $ mount -t cvmfs cepcsw.ihep.ac.cn /cvmfs/cepcsw.ihep.ac.cn

##############################################################################
# Install necessary packages
##############################################################################

RUN dnf install -y git \
 && dnf install -y libglvnd-devel \
 && dnf install -y mesa-libGLU-devel \
 && dnf install -y libXmu-devel \
 && dnf install -y motif-devel \
 && dnf install -y make \
 && dnf install -y gcc-c++ gcc binutils \
 && dnf install -y libuuid-devel \
 && dnf clean -y all \
 && rm -rf /var/cache/dnf

