# Region Of Interest from a Segment Line

In a given image, let's suppose that a region with the shape of an oriented rectangle is interesting to us and we want to crop it. This Python code helps us to do this.

Why do we need an oriented rectangle as a region of interest?

From an input image, like this one:

![input1](doc/input.jpg)

in which points and segment lines are defined in the associated [file](doc/input.mat). For example:

![input2](doc/segments.jpg)

After "inflating" these segments to obtain the oriented rectangles as bounding boxes, they are shown in this figure:

![debug](doc/debug.jpg)

After cropping these oriented rectangles, we obtained these two images:

![output1](doc/output0.jpg)

and:

![output2](doc/output1.jpg)
