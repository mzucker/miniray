miniray
=======
Business card raytracer / eventual IOCCC winner. See full writeup at <https://mzucker.github.io/2016/08/03/miniray.html>

Building
========

To build:

    cd /path/to/miniray
    mkdir build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make
    make ppm_images #optional, takes a while


Using the software
==================

For all versions, you can pipe the output to a PPM (displayable with ImageMagick `display` command among other apps):

    ./miniray > image.ppm

For versions 4.2 and after, you can input your own text:

    ./miniray "my text" > image.ppm
    ./miniray "my text" -preview > image.ppm # faster, reflections & AA disabled

See also
========

  - <http://ioccc.org/2011/zucker/hint.html>
