# Voronizer
Voronizer is tool which transforms a raster image into raster voronoi diagram. It offers several modes which differs in the way of creating the generators. These modes have their specific arguments which can further modify the behavior of the program. By default the colors of the final voronoi cells are chosen to be the mean colors of underlying pixels (of the input image) in that region, but user is also allowed to choose an OpenCV colormap to use instead. The program also offers an option of smoothing the edges of voronoi cells and resizing the image before and after the computation.

## Build
Requires OpenCV >= 4.4.0 and at least C++17:
```
cmake -DCMAKE_BUILD_TYPE=Release .
make
```
or use corresponding Windows/Linux x86-64 pre-built binaries in `Build` directory.

## Usage
Voronizer can be runned from terminal. e.g:
```
./Voronizer -m sift-circles -a 10,,1,0.5 img/lena.jpg 
```
will run the Voronizer in `sift-circles` mode (with mode-specific arguments defined by `-a` arguments) and display the result. The image can be also saved to file instead by specifying the path by `-f` option. All arguments are explained in following description, which can also be displayed by running with `-h` flag.


```
Usage: Voronoizer [options] image 

Positional arguments:
image           path to image to voronize [required]

Optional arguments:
-v --version    prints version information and exits [default: false]
-h --help       shows help message and exits [default: false]
-a --args       Comma separated list of mode-specific positional arguments
                (see --mode argument description for more information).
                Supports truncated list as well as using default value by omitting the value
                (e.g. "-a ,,,0.5" will result in setting the third argument to 0.5,
                all other arguments will keep their default values). [default: ""]
-m --mode       voronizer mode: {sobel, kmeans-circles, kmeans-lines, sift-circles, sift-lines}

                   sobel:  arguments [MEDIAN_PRE=5,EDGE_TRESHOLD=30,MEDIAN_POST=5,CLUSTER_SIZE_TRESHOLD=15]
                      Generators created with Sobel edge detector:
                      1. preprocess image by median filter of size MEDIAN_PRE
                      2. find edges by Sobel detetor
                      3. apply edge tresholding with treshold value EDGE_TRESHOLD [0-255]
                      4. apply median filter of size MEDIAN_POST
                      5. remove any regions with less than CLUSTER_SIZE_TRESHOLD pixels.

                   kmeans-circles:  arguments [MEDIAN_PRE=5,N_COLORS=8,CLUSTER_SIZE_TRESHOLD=15,RADIUS=0,THICKNESS=-1]
                      Generators created by KMeans color clustering - generators are circles
                      centered at centers of mass of regions found by KMeans:
                      1. preprocess image by median filter of size MEDIAN_PRE
                      2. quantize the image to N_COLORS by KMeans
                      3. split the clusters into regions of spatially close pixels with the
                         same color and remove any with less than CLUSTER_SIZE_TRESHOLD pixels
                      4. use centers of mass of the regions as centers of generator circles
                         with given RADIUS (0 for points instead of circles) and THICKNESS
                         (-1 to fill the circles)

                   kmeans-lines:  arguments [MEDIAN_PRE=5,N_COLORS=8,CLUSTER_SIZE_TRESHOLD=15,RANDOM_ITER=25]
                      Generators created by KMeans color clustering - generators are lines
                      where endpoints are centers of  mass of regions found by KMeans:
                      1. - 3. same as "kmeans-circles"
                      4. use centers of mass of the regions as endpoints of line segment generators
                         - for each point try RANDOM_ITER other (unused) points and select
                         the closest one to create new line segment

                   sift-circles:  arguments [KEYPOINT_SIZE_TRESHOLD=5,RADIUS=-1,THICKNESS=-1,RADIUS_MULTIPLIER=1.00]
                      Generators are points/circles created from SIFT keypoints:
                      1. Detect SIFT keypoints of the image
                      2. Filter out keypoints of size less than KEYPOINT_SIZE_TRESHOLD
                      3. Create generator circles at selected keypoints with given RADIUS
                         (0 for points instead of circles) and THICKNESS (-1 to fill the circles).
                         You can also use RADIUS = -1 for radius given by size of SIFT keypoints,
                         which can be adjusted by RADIUS_MULTIPLIER.
                         (RADIUS_MULTIPLIER is ignored in case of RADIUS >= 0)

                   sift-lines:  arguments [KEYPOINT_SIZE_TRESHOLD=5,RANDOM_ITER=25]
                      Generators created with SIFT keypoints - generators are lines where
                      endpoints are SIFT keypoints:
                      1. - 2. same as "sift-circles"
                      3. use selected SIFT keypoints as endpoints of line segment generators
                         - for each point try RANDOM_ITER other (unused) points and select
                         the closest one to create new line segment
                 [default: "sobel"]
-c --colormap   OpenCV colormap name to use instead of original image as color template
                or "bw" for black & white image: {autumn, bone, jet, winter, rainbow, ocean,
                summer, spring, cool, hsv, pink, hot, parula, magma, inferno, plasma, viridis,
                cividis, twilight, twilight_shifted, turbo, deepgreen, bw} [default: ""]
-f --file       Write output to file instead of displaying it in a window [default: ""]
-r --random     Has effect only if using colormap: shuffle colors of areas randomly,
                otherwise color will depend on x and y coordinate of area [default: false]
-s --smooth     Strength of edges smoothing [default: 3]
-i --isize      Size of the longer image side for image resizing *before* the computation.
                This can speed-up the computation but also can change the output as it can
                process of creating the generators. [default: 0]
-o --osize      Size of the longer image side for image resizing *after* the computation.
                 [default: 0]
```

## Examples
Here are just few output examples, more can be viewed in [examples.md](examples.md).

### Smoothing
Without any smoothing the edges of areas are quite hard and unpleasant. To avoid this, level of edge smoothing can be defined by `-s` option:

| 0 | 3 | 20 |
| --- | --- | --- |
| <img src="examples/smooth_0.png" width="400"/> | <img src="examples/smooth_3.png" width="400"/> | <img src="examples/smooth_20.png" width="400"/> |

### Modes
By chosing different mode and different mode arguments you can achive significant changes in output. Here are some examples to illustrate the difference:

#### sobel
| default | -a ,10 | -a 0,30,0 |
| --- | --- | --- |
| <img src="examples/lena-sobel-default.png" width="400"/> | <img src="examples/carqueiranne-sobel-_10.png" width="400"/> | <img src="examples/collobrieres-sobel-0_30_0.png" width="400"/> |

#### kmeans-circles
| -a ,30,5 | -a ,,20,30,2 | -a 1,,3,14,2 |
| --- | --- | --- |
| <img src="examples/lena-kmeans-circles-_30_5.png" width="400"/> | <img src="examples/carqueiranne-kmeans-circles-__20_30_2.png" width="400"/> | <img src="examples/collobrieres-kmeans-circles-1__3_14_2.png" width="400"/> |

#### kmeans-lines
| -a ,,5,100 | default | -a ,,1   |
| --- | --- | --- |
| <img src="examples/lena-kmeans-lines-__5_100.png" width="400"/> | <img src="examples/carqueiranne-kmeans-lines-default.png" width="400"/> | <img src="examples/collobrieres-kmeans-lines-__1.png" width="400"/> |

#### sift-circles
| -a 0,,5,0.6 | -a ,,1,2 | default |
| --- | --- | --- |
| <img src="examples/lena-sift-circles-0__5_0.6.png" width="400"/> | <img src="examples/carqueiranne-sift-circles-__1_2.png" width="400"/> | <img src="examples/collobrieres-sift-circles-default.png" width="400"/> |

#### sift-lines
| -a 1,100 | default | -a 3,20 |
| --- | --- | --- |
| <img src="examples/lena-sift-lines-1_100.png" width="400"/> | <img src="examples/carqueiranne-sift-lines-default.png" width="400"/> | <img src="examples/collobrieres-sift-lines-3_20_.png" width="400"/> |

### Colormaps
The image can be colorized by OpenCV's colormaps instead of using the original image as color template. This can be done by specifying the name of OpenCV colormap (or additionally "bw" for black & white gradient colormap) as the value for `-c` option (see [OpenCV documentation](https://docs.opencv.org/4.4.0/d3/d50/group__imgproc__colormap.html#ga9a805d8262bcbe273f16be9ea2055a65) for the detailed view of all colormaps with images). The color depends on the x a y coordinates of each area (but also on implementation of selected mode). This can be prevented by randomizing the colors by using `-r` flag, as seen in the second row of the following examples:

| bone | cividis | turbo| twilight |
| --- | --- | --- | --- |
| <img src="examples/cmap_bone.png" width="400"/>        | <img src="examples/cmap_cividis.png" width="400"/>        | <img src="examples/cmap_turbo.png" width="400"/>        | <img src="examples/cmap_twilight.png" width="400"/> |
| <img src="examples/cmap_bone_random.png" width="400"/> | <img src="examples/cmap_cividis_random.png" width="400"/> | <img src="examples/cmap_turbo_random.png" width="400"/> | <img src="examples/cmap_twilight_random.png" width="400"/> |

## Developer Documentation
See [dev_docs.md](dev_docs.md).
