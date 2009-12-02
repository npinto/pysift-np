##Copywrite 2009 Alexandre Gramfort
##All rights reserved.
##
##Redistribution and use in source and binary forms, with or without
##modification, are permitted provided that the following conditions
##are met:
##1. Redistributions of source code must retain the above copyright
##   notice, this list of conditions and the following disclaimer.
##2. Redistributions in binary form must reproduce the above copyright
##   notice, this list of conditions and the following disclaimer in the
##   documentation and/or other materials provided with the distribution.
##3. The name of the author may not be used to endorse or promote products
##   derived from this software without specific prior written permission.
##
##THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
##IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
##OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
##IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
##INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
##NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
##DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
##THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
##(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
##THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import Image, ImageDraw
import numpy as np
import pylab as pl
import sift
import math
from os import path

mypath = path.dirname(path.abspath(__file__))

input_image = path.join(mypath, 'test.pgm')
output_image = path.join(mypath, 'demo_sift.png')
output_descriptors = path.join(mypath, 'results_toy.txt')

def printDescriptor(desc,file):
    map(lambda a: file.write(str(a)+' '),desc)
    file.write('\n')

if 0:
    a = np.zeros((128,128),dtype=np.uint8)
    a[40:70,20:50] = 255
    im = Image.fromarray(a,mode='L')
else:
    im = Image.open(input_image)
    a = np.asarray(im)
    a = np.flipud(a)

l = list(im.getdata())
l.insert(0,im.size)
desc = sift.sift(l,verbose=True)
print len(desc), "descriptors found"

output = open(output_descriptors,'w')

pl.close()
pl.imshow(np.asarray(im),cmap="gray", origin='upper')
pl.axis('off')

r = np.linspace(0,2*math.pi,100)

for a in desc:
    x,y,scale,ang = a[:4]
    im.putpixel((x,y),int(255))
    pl.plot(x+scale/2*np.cos(r),y+scale/2*np.sin(r),'r')
    pl.plot((x,x+scale/2*math.cos(ang)),(y,y+scale/2*math.sin(ang)),'g')
    printDescriptor(a,output)

output.close()

pl.show()
pl.savefig(output_image)
