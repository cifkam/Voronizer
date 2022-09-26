# Voronizer

## Build
Requires OpenCV >= 4.4.0 and C++17
```
cmake -DCMAKE_BUILD_TYPE=Release . && make
```

## Usage
```
Usage: Voronoizer [options] image 

Positional arguments:
image           path to image to voronize [required]

Optional arguments:
-h --help       shows help message and exits [default: false]
-v --version    prints version information and exits [default: false]
-o --options    Comma separated list of mode-specific positional options - see --mode option description for more information.
                Supports truncated list as well as using default value by omitting the value
                (e.g. "-o ,,,0.5" will result in setting the third option to 0.5, all other options will keep their default values).
-m --mode       voronizer mode: {sobel, kmeans-circles, kmeans-lines, sift-circles, sift-lines}

                   sobel:  options [MEDIAN_PRE=5,EDGE_TRESHOLD=30,MEDIAN_POST=5,CLUSTER_SIZE_TRESHOLD=15]
                      Generators created with Sobel edge detector:
                      1. preprocess image by median filter of size MEDIAN_PRE
                      2. find edges by Sobel detetor
                      3. apply edge tresholding with treshold value EDGE_TRESHOLD [0-255]
                      4. apply median filter of size MEDIAN_POST
                      5. remove any regions with less than CLUSTER_SIZE_TRESHOLD pixels.

                   kmeans-circles:  options [MEDIAN_PRE=5,N_COLORS=8,CLUSTER_SIZE_TRESHOLD=15,RADIUS=0,THICKNESS=-1]
                      Generators created by KMeans color clustering - generators are circles centered at centers of mass of regions found by KMeans:
                      1. preprocess image by median filter of size MEDIAN_PRE
                      2. quantize the image to N_COLORS by KMeans
                      3. split the clusters into compact regions of pixels with the same color and remove any with less than CLUSTER_SIZE_TRESHOLD pixels
                      4. use centers of mass of the regions as centers of generator circles with given RADIUS (0 for points instead of circles)
                         and THICKNESS (-1 to fill the circles)

                   kmeans-lines:  options [MEDIAN_PRE=5,N_COLORS=8,CLUSTER_SIZE_TRESHOLD=15,RANDOM_ITER=25]
                      Generators created by KMeans color clustering - generators are lines where endpoints are centers of  mass of regions found by KMeans:
                      1. - 3. same as "kmeans-circles"
                      4. use centers of mass of the regions as endpoints of line segment generators
                          - for each point try RANDOM_ITER other (unused) points and select the closest one to create new line segment

                   sift-circles:  options [KEYPOINT_SIZE_TRESHOLD=5,RADIUS-1,THICKNESS-1,RADIUS_MULTIPLIER1.000000]
                      Generators are points/circles created from SIFT keypoints:
                      1. Detect SIFT keypoints of the image
                      2. Filter out keypoints of size less than KEYPOINT_SIZE_TRESHOLD
                      3. Create generator circles at selected keypoints with given RADIUS (0 for points instead of circles) and THICKNESS (-1 to fill the circles).
                         You can also use RADIUS = -1 for radius given by size of SIFT keypoints, which can be adjusted by RADIUS_MULTIPLIER
                         (RADIUS_MULTIPLIER is ignored in case of RADIUS >= 0)

                   sift-lines:  options [KEYPOINT_SIZE_TRESHOLD=5,RANDOM_ITER25]
                      Generators created with SIFT keypoints - generators are lines where endpoints are SIFT keypoints:
                      1. - 2. same as "sift-circles"
                      3. use selected SIFT keypoints as endpoints of line segment generators - for each point try RANDOM_ITER other (unused) points
                         and select the closest one to create new line segment
                 [default: "sobel"]
-c --colormap   OpenCV colormap name to use instead of original image as template: {autumn, bone, jet, winter, rainbow, ocean, summer, spring, cool, hsv, pink, hot, parula, magma, inferno, plasma, viridis, cividis, twilight, twilight_shifted, turbo}
-f --file       Write output to file instead of displaying it in a window
-r --random     Has effect only if using colormap: shuffle colors of areas randomly, otherwise color will depend on x and y coordinate of area
-s --smooth     Strength of edges smoothing [default: 3]
```