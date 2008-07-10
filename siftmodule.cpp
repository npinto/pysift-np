// file:        siftmodule.cpp
// author:      Dylan Sale
// description: Sift python interface
// Year: 2008
// License: This code licensed under the following:
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions
//are met:
//1. Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
//2. Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
//3. The name of the author may not be used to endorse or promote products
//   derived from this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
//IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
//INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
//NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
//THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include <Python.h>
#include "sift.hpp"
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

/* keypoint list */
typedef vector< pair<VL::Sift::Keypoint,VL::float_t> > Keypoints ;

/* predicate used to order keypoints by increasing scale */
bool cmpKeypoints (Keypoints::value_type const&a,
				   Keypoints::value_type const&b) {
					   return a.first.sigma < b.first.sigma ;
}


static PyObject *
sift_foo(PyObject *self, PyObject *args)
{
	printf("Hello, world\n");
	Py_INCREF(Py_None);
	return Py_None;
}

const int maxval = 255; //the max value that a pixel can have

static float log2(float x)
{
	static float ln2 = log(2.0);
	return log(x)/ln2;
}


void insertDescriptor(PyObject* outList, VL::float_t x,VL::float_t y,VL::float_t sig,VL::float_t ang, VL::float_t const * descr_pt,bool fp)
{
	PyObject* descList = PyList_New(132);
	
	PyList_SetItem(descList,0,PyFloat_FromDouble(x));
	PyList_SetItem(descList,1,PyFloat_FromDouble(y));
	PyList_SetItem(descList,2,PyFloat_FromDouble(sig));
	PyList_SetItem(descList,3,PyFloat_FromDouble(ang));	
	
	for(int i=0; i<128; i++)
	{
		if(fp)
			PyList_SetItem(descList,i+4,PyFloat_FromDouble(descr_pt[i]));
		else
			PyList_SetItem(descList,i+4,PyInt_FromLong((long)(512.0f*descr_pt[i])));
	}

	PyList_Append(outList,descList);

}

struct tempKeypoint {
	tempKeypoint(float x_,float y_,float sig_,float th_):x(x_),y(y_),sig(sig_),th(th_){}
	float x,y,sig,th;
};

typedef std::vector<tempKeypoint> keyVector;

//list is a list of (x,y,sigma,theta) tuples
static int keypointsFromList(PyObject* list, void* out)
{	
	keyVector* keypointsOut = (keyVector *)out;

	VL::float_t x,y,sigma,th ;

	if (!PyList_Check(list))
	{
		PyErr_BadArgument();
		return 0;
	}

	int length = PyList_Size(list);
	
	for(int i=0;i<length;i++)
	{
		PyObject* element = PyList_GetItem(list,i);

		if(!PyTuple_Check(element))
		{
			printf("Probably should figure out how to use an exception here, but ERROR!!! keyword must be a list of tuples.\n");
			PyErr_BadArgument();
			return 0;
		}


		if (PyTuple_Size(element) != 4)
		{
			printf("Probably should figure out how to use an exception here, but ERROR!!! keyword length must be 4.\n");
			PyErr_BadArgument();
			return 0;
		}

		//should probably check if its floats, but meh
		
		x = (float)PyFloat_AsDouble( PyTuple_GetItem(element,0));
		y = (float)PyFloat_AsDouble( PyTuple_GetItem(element,1));
		sigma = (float)PyFloat_AsDouble( PyTuple_GetItem(element,2));
		th = (float)PyFloat_AsDouble( PyTuple_GetItem(element,3));

		tempKeypoint k(x,y,sigma,th);

		keypointsOut->push_back(k);
	}

	return 1;
}


//allocates memory for an image
static int imageFromList(PyObject* list, void* out)
{
	if (!PyList_Check(list))
	{
		PyErr_BadArgument();
		return 0;
	}

	PyObject* dims = PyList_GetItem(list,0);
	if (!PyTuple_Check(dims))
	{
		PyErr_BadArgument();
		return 0;
	}

	int width, height;
	int ok = PyArg_ParseTuple(dims,"ii",&width,&height);
	if(!ok)
	{
		PyErr_BadArgument();
		return 0;
	}

	int dimsSize = width*height;

	if(PyList_Size(list)-1 != dimsSize)
	{
		printf("Probably should figure out how to use an exception here, but ERROR!!! the width*height isnt the size of the list-1.\n");
		PyErr_BadArgument();
		return 0;
	}

	VL::PgmBuffer* buffer = (VL::PgmBuffer*)out;
  
	buffer->width = width;
	buffer->height = height;

	buffer->data = new VL::pixel_t [ dimsSize ];

	for(int i=0;i<dimsSize;i++)
	{
		buffer->data[i] = ((VL::pixel_t) PyInt_AsLong(PyList_GetItem(list,i+1)))/maxval;
		if(buffer->data[i] > 1.0)
		{
			delete[] buffer->data;
			buffer->data = NULL;
			printf("Probably should figure out how to use an exception here, but ERROR!!! We only support 8bit images (max value of 255 per pixel).\n");
			PyErr_BadArgument();
			return 0;
		}
		//printf("data[%d] = %f\n",i,buffer->data[i]);
	}

	return 1;
}


//TODO: ALTERNATIVELY, use a bufferObject or PyCObject from PIL or something to jus get the image memory directly

const int initLevels = 3;

static PyObject *
sift_run(PyObject *self, PyObject *args, PyObject *keywds)
{
	int    first          = -1 ;
	int    octaves        = -1 ;
	int    levels         = initLevels ;
	float  threshold      = 0.04f / levels / 2.0f ;
	float  edgeThreshold  = 10.0f;
	float  magnif         = 3.0 ;
	int    nodescr        = 0 ;
	int    noorient       = 0 ;
	int    stableorder    = 0 ;
	int    savegss        = 0 ;
	int    verbose        = 0 ;
	int    binary         = 0 ;
	int    haveKeypoints  = 0 ;
	int    unnormalized   = 0 ;
	int    fp             = 0 ; //if the output should be floating point


	VL::PgmBuffer buffer;
	buffer.data = NULL;
	
	keyVector keypointList;


	static char *kwlist[] = {"image","keypoints","octaves", "levels", "first","fp","verbose","magnif", "unnormalized","stableorder","noorient","threshold","edgeThreshold",NULL};


	int ok = PyArg_ParseTupleAndKeywords(args,keywds,"O&|O&iiiiifiiiff:run",kwlist,imageFromList,&buffer,keypointsFromList,&keypointList,&octaves,&levels,&first,&fp,&verbose,&magnif,&unnormalized,&stableorder,&noorient,&threshold,&edgeThreshold);

	if(keypointList.size() > 0)
	{
		haveKeypoints = true;
		if(verbose) printf("keypoints parsed\n");
	}
	
	if(levels != initLevels && threshold == 0.04f / initLevels / 2.0f) //only recalculate it if it wasnt set manually
		threshold = 0.04f / levels / 2.0f ;

	if(!ok)
		return NULL;

	PyObject* outList = PyList_New(0);
	//Py_INCREF(outList); //not sure if you have to do this?

	int         O      = octaves ;    
	int const   S      = levels ;
	int const   omin   = first ;
	float const sigman = .5 ;
	float const sigma0 = 1.6 * powf(2.0f, 1.0f / S) ;


	// optionally autoselect the number number of octaves
	// we downsample up to 8x8 patches
	if(O < 1) {
		O = std::max
		(int
		 (std::floor
		  (log2
		   (std::min(buffer.width,buffer.height))) - omin -3), 1) ;
	}

	if(verbose) printf("init scale space\n");

	// initialize scalespace
	VL::Sift sift(buffer.data, buffer.width, buffer.height, 
		sigman, sigma0,
		O, S,
		omin, -1, S+1) ;

	if(!haveKeypoints)
	{
		if(verbose) printf("detecting keypoints\n");

		sift.detectKeypoints(threshold, edgeThreshold) ;
	}

	// -------------------------------------------------------------
	//                  Run SIFT orientation detector and descriptor
	// -------------------------------------------------------------    

	/* set descriptor options */
	sift.setNormalizeDescriptor( ! unnormalized ) ;
	sift.setMagnification( magnif ) ;

	if(verbose) printf("compute descriptors\n");

	if( haveKeypoints ) {
		if(verbose) printf("using keypoints\n");
		// -------------------------------------------------------------
		//                 Reads keypoint from file, compute descriptors
		// -------------------------------------------------------------
		Keypoints keypoints ;

		for(keyVector::iterator keypointIterator = keypointList.begin();
			keypointIterator != keypointList.end();
			keypointIterator ++)
		{
			tempKeypoint k = *keypointIterator;

			/* compute integer components */
			VL::Sift::Keypoint key 
				= sift.getKeypoint(k.x,k.y,k.sig) ;

			Keypoints::value_type entry ;
			entry.first  = key ;
			entry.second = k.th ;
			keypoints.push_back(entry) ;
		}

		/* sort keypoints by scale if not required otherwise */
		if(! stableorder)
			sort(keypoints.begin(), keypoints.end(), cmpKeypoints) ;

		// process in batch
		for(Keypoints::const_iterator iter = keypoints.begin() ;
			iter != keypoints.end() ;
			++iter) {
				VL::Sift::Keypoint const& key = iter->first ;
				VL::float_t th = iter->second ;

				///* write keypoint */
				//out << setprecision(2) << key.x     << " "
				//	<< setprecision(2) << key.y     << " "
				//	<< setprecision(2) << key.sigma << " "
				//	<< setprecision(3) << th ;

				/* compute descriptor */
				VL::float_t descr [128] ;
				sift.computeKeypointDescriptor(descr, key, th) ;

				if( ! nodescr ) {
					insertDescriptor(outList, key.x,key.y,key.sigma, th ,descr, fp) ;
				}
		} // next keypoint

	} else {
		// -------------------------------------------------------------
		//            Run detector, compute orientations and descriptors
		// -------------------------------------------------------------
		for( VL::Sift::KeypointsConstIter iter = sift.keypointsBegin() ;
			iter != sift.keypointsEnd() ; ++iter ) {

				// detect orientations
				VL::float_t angles [4] ;
				int nangles ;
				if( ! noorient ) {
					nangles = sift.computeKeypointOrientations(angles, *iter) ;
				} else {
					nangles = 1;
					angles[0] = VL::float_t(0) ;
				}

				// compute descriptors
				for(int a = 0 ; a < nangles ; ++a) {

					//std::cout << std::setprecision(2) << iter->x << ' '
					//	<< std::setprecision(2) << iter->y << ' '
					//	<< std::setprecision(2) << iter->sigma << ' ' 
					//	<< std::setprecision(3) << angles[a] ;

					/* compute descriptor */
					VL::float_t descr_pt [128] ;
					sift.computeKeypointDescriptor(descr_pt, *iter, angles[a]) ;

					/* save descriptor to to appropriate file */	      
					if( ! nodescr ) {
						insertDescriptor(outList, iter->x,iter->y,iter->sigma,angles[a],descr_pt, fp) ;
					}
				} // next angle
		} // next keypoint
	}


	if(buffer.data != NULL)
		delete[] buffer.data;

	return outList;
}


//setup python methods
static PyMethodDef sift_methods[] = {
	{"sift", (PyCFunction)sift_run, METH_VARARGS | METH_KEYWORDS, "sift(imageList,[keypoints, octaves,levels,first,fp,verbose,magnif,unnormalized,stableorder,noorient,threshold,edgeThreshold]), where the arguments in [] are optional and imageList is a list of [(width,height),1,2,3,4,...] for each pixel in row order. That is, it is accessed image[x+width*y] (it is grayscale).\n The keypoints must be a list of 4-tuples (x,y,sigma,theta).\n The optional arguments can be accessed with keywords, eg\n sift.sift(image,verbose=True,keypoints=keys)."},
	{NULL, NULL}
};

//setup the sift module
PyMODINIT_FUNC
initsift(void)
{
	Py_InitModule("sift", sift_methods);
}
