# -*- coding: utf-8 -*-

"""
LightTwinSVM Program - Simple and Fast
Version: 0.2.0-alpha - 2018-05-30
Developer: Mir, A. (mir-am@hotmail.com)
License: GNU General Public License v3.0

Module: setup.py
Building C++ extension module for Windows OS using Cython

Externel dependencies:
- Armadillo C++ Linear Agebra Library (http://arma.sourceforge.net)
- LAPACK and BLAS libaray (http://www.netlib.org/lapack)

"""

from distutils.core import setup, Extension
from Cython.Build import cythonize
import numpy as np


setup(ext_modules = cythonize(Extension(
        "clippdcd",
        sources=["clippdcd.pyx", "clippdcd_opt.cpp"],
        language="c++",
        libraries=['lapack' ,'blas'],
        library_dirs=['.\lib'],
        )), include_dirs=[np.get_include(), './armadillo-code/include'])
