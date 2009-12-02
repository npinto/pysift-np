from distutils.core import setup, Extension

exts = [Extension("sift", sources=["siftmodule.cpp","sift.cpp"])]

# on mac do not forget if you have a 32 bits Python
# export CFLAGS=-m32

setup(name="pysift",
      version="0.1",
      description="Python SIFT",
      author="Dylan Sale, Alexandre Gramfort, Nicolas Pinto",
      maintainer="Alexandre Gramfort",
      maintainer_email="alexandre.gramfort@gmail.com",
      ext_modules=exts)
