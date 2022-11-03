# Developer Documentation

## Introduction
Voronizer is tool which allows user to transform a raster image into raster voronoi diagram. The process consists of two stages: in fist stage the raster generators are created, in the second we create the voronoi diagram itself. Voronizer offers several modes which differs in the first stage, i.e. the way of creating the generators. These modes have their specific arguments which can further modify the behavior of the program. By default the colors of the final voronoi cells are chosen to be the mean colors of underlying pixels (of the input image) in that region, but user is also allowed to choose an OpenCV colormap to use instead. The program also offers an option of smoothing the edges of voronoi cells.

In this documentation, we will describe the architecture of Voronizer and also explain the individual Voronizer modes. It can be also used as an guide for an implementation of custom modes.

## Architecture

### Voronizers classes
All the classes that do the whole process of 'Voronization' (e.g. `SobelVoronizer` or `SIFTVoronizerCircles`) are derived from abstract class `AbstractVoronizer`. It provides several members:

1. `run` is an abstract function, which has to be implemented by derived classes. It is the main public method that's used to do the computation. The implementation is completely up to the developer, but the general convention is to use the 3 following steps:

    a. Create the generators of voronoi cells

    b. Use the `Voronoi` class (see the next section about Growing classes) to create the diagram from the generators

    c. Use colorization function stored in member `colorize_funct` and return the result. Depending on the colorization type, the final image is either created directly from the image output of Voronoi::compute function or from the `Growing::groups`, i.e. using the `std::map` containing vector of all the pixels for each voronoi cell ID.

2. `set/unset_colormap` function allows developer to select between types colorization of final voroni cells by image template. By default the color of each voronoi cell is computed as a mean color of all the pixels (of input image) in that cell. Alternatively the colorization can be done by applying one of [OpenCV's colormaps](https://docs.opencv.org/4.4.0/d3/d50/group__imgproc__colormap.html#ga9a805d8262bcbe273f16be9ea2055a65) by using our `set_colormap` function (additionally the raw black & white output can be kept by
 setting the colormap to (cv::ColormapTypes)-1). During the computation, each voroni cell is assigned an ID, which will be used as an input to the colormap function. The ordering of IDs completely depends on implementation of the class – in our classes it depends on `x` and `y` coordinates of the regions. Another option is to set the `random` argument to true, which will randomly shuffle the IDs. Be aware that currently the OpenCV supports only colormapping of 8-bit color depth images so before applying the colormap we use an modulo operation to make sure that all the values are in range [0-255]. This will result in repeating colors if there are more that 256 cells.

For more details see the source code and the description of the modes below.

### Growing classes
The algorithm for raster voronoi digram is basically modified region growing algorithm (one can also see it as BFS on graph, where pixels are nodes and edges correspond to the pixel neghborhood relationship). As we utilize the region growing also in other ways, we define an abstract `Growing` class and derive the `Voronoi` class (and the `Separator` class) from it.

#### Growing class
This class implements the core of the region growing algorithm. The main logic is implemented in the `compute_inner` function – we start by initializing the necessary variables, e.g. we create an 2D array of `Pixels` keeping an information about the state of each pixel (unseen/opened/closed) and then mark several pixels as 'opened'. This initialization needs to be implemented by overriding the `init_funct` member function in derived class.

After initialization we process the 'opened' pixels until there are no opened pixels left: we select every opened pixel, look at all its neighbors and check the growing condition. If the condition is satisfied, we mark the neighbor as 'opened' and assign it the value of the selected pixel. After checking all the neghbors we mark selected pixel as 'closed'. The derived class can choose to use 4/8-neighborhood or it can alternate these two each step by using corresponding value of `Neighborhood` enum as the `Growing` constructor parameter. Also the growing condition can be modified by overriding the `grow_condition` member function, by default it only checks if the pixel's state is 'unseen'. Another option how to modifiy the behaviour of the algorithm is by overriding the `post_funct`, which can do some postprocessing based on information about all the pixels modified during the computation.

The computation itself can be runned by calling the `compute` member function, which works as a wrapper – it takes care of copying the data etc. After the computation is done, the developer can (apart from the output image data assigned to the `output_data` variable reference) take advantage of the `Growing::groups` variable – map that keeps information about the value assigned to each pixel (`std::map<int,std::vector<Pixel*>>` keeping lists of pixels that share the same value after the growing, thus belonging to the same 'group'). Be careful – the variable keeps only pointers to the `Pixel` instances that are actually stored in member `pixel_mat`. You can use the information provided by `Grwoing::groups` only as long as the data of `pixel_mat` exist in the memory, i.e. until the `Growing` instance hasn't been destroyed and until next call of `compute` function. If you need the data later, you can move it outside of the class, but make sure you also save the `pixel_mat` data. You can either use the C++ move semantics or more preferably get the `unique_ptr` instances by calling `clear_groups` and `clear_pixelmat` which returns them and automatically resets the members to null-pointers.

The `compute_inner` function expect the input to be 16-bit single channel image (`CV_16S`) – depending on the image, mode and its arguments, it can easily happend that there will be more than 256 voronoi cells, therefore using the 16-bit depth is necessary. However the input can still be 8-bit image – for this purpose we just convert the input to `CV_16S` without any value scaling, i.e. keeping the pixel values in range [0-255].

#### Voronoi class
`Voronoi` is just a simple extension of the `Growing` class. Almost all the logic has been already implemented by the base class, so we only set the alternating neighborhood type and implement the `init_funct` function: we use simple convention and set the pixels with non-zero value to 'opened' state, all other pixels are set to 'unseen'. This way, input can be an image where the background is black and the areas with different gray colors represent different generators.

#### Separator class
In some cases we may have groups of pixels that have the same value but don't form a single continuous area. For example in _Sobel_ mode we create generators by binary tresholding and use the white pixels as generators. But we want to treat every continuous group of pixels with the same value as separate generator, therefore we need to separate or partition these groups of pixels. For that purpose we use the `Separator` class. It is again derived from the `Growing` class, but we change the behaviour a little. We call the `compute_inner` multiple times and every time we initialize only single pixel as `opened`. In this single call we find all the neighboring pixels with the same (pixels forming one generator) starting with our selected pixel, and assign all of them a new unique value – an ID. We repeat the same process (find new pixel and grow to neighbors) until we use all the pixels with non-zero value in the input image.

The separator also offers an option to remove the groups number of pixels less then a treshold. This is done by overriding the `post_funct` – we remove these groups and reset the pixels value to the background value (zero). However this creates blank areas in the output, therefore we need to fill these areas with values of neighboring pixels. This is done by a helper class `AfterTresholdGrowing`, which identifies pixels on the border of the areas, runs the growing again and also takes care of removed IDs by remapping the group IDs to continuous range of integers.

## Modes
Now we will describe the difference between the modes, i.e. how the generators are created. Each mode has arguments that modify its behaviour, in this text they are highlighted by *CAPITAL ITALICS*.

### sobel
Mode `sobel` creates generators by finding edges using sobel detector applying binary tresholding:

1. Preprocess image by median filter of size *MEDIAN_PRE* to remove unnecessary details.
2. Find edges by Sobel detetor.
3. Apply binary tresholding with treshold value *EDGE_TRESHOLD* [0-255] to filter out edges that are not too significant.
4. Apply median filter of size MEDIAN_POST to make the generators smoother.
5. Use the `Separator` to partition the white pixels into generators and remove those with less than *CLUSTER_SIZE_TRESHOLD* pixels.


### kmeans-circles
This mode computes the K-means color clustering and uses the centers of mass of found regions as centers of circles, which are used as generators:

1. Preprocess image by median filter of size *MEDIAN_PRE* to remove unnecessary details.
2. Quantize the image to *N_COLORS* by K-means.
3. Use the `Separator` to partition the quantized image into regions of neighboring pixels with the same color and remove any with less than *CLUSTER_SIZE_TRESHOLD* pixels.
4. Use centers of mass of the regions as centers of generator circles with given *RADIUS* (0 for single pixel points instead of circles) and *THICKNESS* (-1 to fill the circles).

### kmeans-lines
Mode similar to the `kmeans-circles` except for the last step. Generators are not circles but lines. Endpoints of the lines are the centers of mass of regions after K-means color clustering, the endpoints are selected randomly by trying several combinations and selecting the closest ones:

1. Preprocess image by median filter of size *MEDIAN_PRE* to remove unnecessary details.
2. Quantize the image to *N_COLORS* by K-means.
3. Use the `Separator` to partition the quantized image into regions of neighboring pixel with the same color and remove any with less than *CLUSTER_SIZE_TRESHOLD* pixels.
4. Use centers of mass of the regions as endpoints of line segment generators – for each point try *RANDOM_ITER* other unused points and select the closest one to create new line segment.


### sift-circles
This mode detects SIFT keypoints of the image and then creates generators by drawing circles at these keypoints. The radius can be either defined by user or each circle can have its radius defined by the size of corresponding SIFT keypoint (modified by multiplicative factor given by user):

1. Detect SIFT keypoints of the image.
2. Filter out keypoints with size less than *KEYPOINT_SIZE_TRESHOLD*.
3. Create generator circles at selected keypoints with given *RADIUS* (0 for single pixel points instead of circles, -1 to use the size of SIFT keypoint as radius) and *THICKNESS* (-1 to fill the circles). If the SIFT keypoint size are used as radii, each circle radius can be modified by multiplicative factor *RADIUS_MULTIPLIER*. If radius defined by user (i.e. the vaue is greater or equal to 0), the *RADIUS_MULTIPLIER* is ignored.


### sift-lines
Mode similar to the `sift-circles` except for the last step. Generators are not circles but lines where its endpoints are the SIFT keypoints, the endpoints are selected randomly by trying several combinations and selecting the closest ones:

1. Detect SIFT keypoints of the image.
2. Filter out keypoints of size less than *KEYPOINT_SIZE_TRESHOLD*.
3. Use selected SIFT keypoints as endpoints of line segment generators – for each point try *RANDOM_ITER* other unused points and select the closest one to create new line segment.
