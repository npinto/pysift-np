##Copywrite 2008 Dylan Sale
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



import sift
import Image

#This is an example file showing how to read in a descriptor file
#for sift to use instead of detecting them itself


def printDescriptor(desc,file):
    map(lambda a: file.write(str(a)+' '),desc)
    file.write('\n')


def main():
    print "opening image"
    im = Image.open("test.pgm")

    l = list(im.getdata())    

    l.insert(0,im.size)
    #the image data must be a list of pixels in row order
    #(that is, the first row, then the second row etc)
    #with a (width,height) tuple as the first element
    #the image must be grayscale (one channel)


    print "running sift"

    #we do this to load in a file of keypoints so that sift
    #doesnt have to find them (or we can guide it)
    #takes the first 4 floats per line and puts in a tuple
    input = open('keypoints.txt','r')
    keys = []
    for a in input:
        keys.append(tuple(map(float,a.split()[0:4])))

    input.close()

    #keys must be a list of 4-tuples for x,y,sigma and theta of each keypoint
    desc = sift.sift(l,keypoints=keys,verbose=True)
    
    print "done"

    print len(desc), "descriptors found"
    #print desc

    output = open('results.txt','w')
    
    for a in desc:
        x = a[0]
        y = a[1]
        im.putpixel((x,y),int(255))
        printDescriptor(a,output)

    output.close()


    im.show()




if __name__ == "__main__":
    main()
