# Region Of Interest from an Segment Line

In a given image, let suppose that a region with the shape of an oriented rectangle is interesting us and we want to crop it. This Python code is helping us to do this.

Why we need an oriented rectangle as region of interest ?

From an input image, like this one:

![input1](doc/input.jpg)

in which points and segment lines are defined in the associated [file](doc/input.mat). For example:

![input2](doc/segments.jpg)

After "inflating" these segments to obtain the oriented rectangles as bounding boxes, are shown in this figure:

![debug](doc/debug.jpg)

After croping these oriented rectangles we obtained these two images:

![output1](doc/output0.jpg)

and:

![output2](doc/output1.jpg)
