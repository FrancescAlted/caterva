# Caterva

[![Build Status](https://dev.azure.com/blosc/caterva/_apis/build/status/caterva?branchName=master)](https://dev.azure.com/blosc/caterva/_build/latest?definitionId=3&branchName=master)
[![Documentation Status](https://readthedocs.org/projects/caterva/badge/?version=latest)](https://caterva.readthedocs.io/en/latest/?badge=latest)
[![Azure DevOps coverage](https://img.shields.io/azure-devops/coverage/Blosc/Caterva/3)](https://dev.azure.com/blosc/caterva/_build/latest?definitionId=5&branchName=master)


## What it is

Caterva is a C library for handling multi-dimensional, compressed datasets in an easy and convenient manner. It implements a thin metalayer on top of [C-Blosc2](https://github.com/Blosc/c-blosc2) for specifying not only the dimensionality of a dataset, but also the dimensionality of the chunks inside the dataset. In addition, Caterva adds machinery for retrieving arbitrary multi-dimensional slices (aka hyper-slices) out of the multi-dimensional containers in the most efficient way. Hence, Caterva brings the convenience of multi-dimensional and compressed containers to your application very easily.

For more info, check out the [Caterva documentation](https://caterva.readthedocs.io).
  
Here are slides of a gentle [introductory talk to Caterva](http://blosc.org/docs/Caterva-HDF5-Workshop.pdf).
