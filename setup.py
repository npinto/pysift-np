from distribute_setup import use_setuptools
use_setuptools()
from setuptools import setup, find_packages


from distutils.core import setup, Extension

exts = [Extension("sift/sift", sources=["sift/siftmodule.cpp",
                                        "sift/sift.cpp"]
                  )
        ]

# on mac do not forget if you have a 32 bits Python
# export CFLAGS=-m32

setup(
    name="pysift",
    version="0.1-np",
    description="Python SIFT",

    author="Dylan Sale, Alexandre Gramfort, Nicolas Pinto",
    maintainer="Alexandre Gramfort and Nicolas Pinto",
    maintainer_email="alexandre.gramfort@gmail.com and pinto@mit.edu",
    
    ext_modules=exts,
    )


