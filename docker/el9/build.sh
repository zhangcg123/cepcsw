#!/bin/bash

docker build -t cepc/cepcsw-cvmfs:el9 . --build-arg CVMFSMOD=INSIDE && docker push cepc/cepcsw-cvmfs:el9
docker build -t cepc/cepcsw:el9 . && docker push cepc/cepcsw:el9
