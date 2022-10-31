# Developer Documentation

## Introduction
Voronizer is tool which allows user to transform a raster image into raster voronoi diagram. The process consists two stages: in fist stage the raster generators are created, in the second we create the voronoi diagram itself. Voronizer offers several modes which differs in the first stage, i.e. the way how the generators are created. These modes have their specific options which can further modify the behavior of the algorithm. By default the colors of the final voronoi cells are chosen to be the mean colors of underlying pixels (of the input image) in that region, but user is also allowed to choose an OpenCV colormap to use instead. Voronoizer also offers an option of smoothing the edges of voronoi cells.

In this documentation, we will describe the architecture of Voronizer and also explain the logic behind individual Voronizer modes. It can be also used as an guide for implementation of additional modes.

## Architecture

### Voronizers classes
All the classes that do the whole process of 'Voronization' (e.g. `SobelVoronizer` or `SIFTVoronizerCircles`) are derived from abstract class `AbstractVoronizer`. It provides several members:

1. `run` is an abstract function, which has to be implemented by derived classes. It is the main public method that's used to do the computation. The implementation is completely up to the developer, but the general convention is to use the 3 following steps:


    a. Create the generators of voronoi cells

    b. Use the `Voronoi` class (see the next section about Growing classes) to create the diagram from  the generators

    c. Use colorization function stored in member `colorize_funct` and return the result. Depending on the colorization type, the final image is either created directly from the image output of Voronoi::compute function, or from the `Growing::groups`, i.e. using the `std::map` containing vector of all the pixels for each voronoi cell ID.


2. `set/unset_colormap` function allows developer to select between types colorization of final voroni cells by image template. By default the color of each voronoi cell is computed as a mean color of all the pixels (of input image) in that cell. Alternatively the colorization can be done by applying one of [OpenCV's colormaps](https://docs.opencv.org/4.4.0/d3/d50/group__imgproc__colormap.html#ga9a805d8262bcbe273f16be9ea2055a65), i.e. by using our `set_colormap` function. During the computation, each voroni cell is assigned an ID, which will be used as an input to the colormap function. The ordering of IDs completely depends on implementation of the class - in our classes it depends on `x` and `y` coordinates of the regions. Another option is to set the `random` argument to true, which will randomly shuffle the IDs. Be aware that currently the OpenCV supports only colormapping of 8-bit color depth images so before applying the colormap we use an modulo operation to make sure that all the values are in range [0-255]. This will result in repeating colors if there are more that 256 cells.


###

### Growing classes
The algorithm for raster voronoi digram is basically modified region growing algorithm (one can also see it as BFS on graph, where pixels are nodes and edges correspond to the pixel neghborhood relationship). As we utilize the region growing also in one other way, we define an abstract `Growing` class and derive the `Voronoi` class (and the `Separator` class) from it.

#### Growing class
This class implements the core of the region growing algorithm. The main logic is implemented in the `compute_inner` function - we start by initializing the necessary variables, e.g. we create an 2D array of `Pixels`keeping an information about the state of each pixel (unseen/opened/closed) and then mark several pixels as 'opened'. This initialization needs to be implemented by overriding the `init_funct` member function in derived class.

After initialization we are process the 'opened' pixels until there are no opened pixels left: we select every opened pixel, look at all its neighbors and check the growing condition. If the condition is satisfied, we mark the neighbor as 'opened' and assign it a value of the selected pixel. After checking all the neghbors we mark the pixel as 'closed'. The derived class can choose to use 4/8-neighborhood or it can alternate these two each step. Also the growing condition can be modified by overriding the `grow_condition` member function, by default it only checks if the pixel's state is 'unseen'. Another option how to modifiy the behaviour of the algorithm is by overriding the `post_funct`, which can do some postprocessing based on information about all the pixels modified during the computation.

The computation itself can be runned by calling the `compute` member function, which works as a wrapper - it takes care of copying the data etc. After the computation is done, the developer can (apart from the output image data assigned to the `output_data` variable) take advantage of the Growing::groups variable - map which keeps information about the value assigned to each pixel (`std::map<int,std::vector<Pixel*>>`), i.e. it keeps lists of pixels that share the same value after the growing, thus belonging to the same 'group'. Be careful - the variable keeps only pointers to the `Pixel` instances that are actually stored in `pixel_mat` member. You can use the information provided by `Grwoing::groups` only as long as the data of `pixel_mat` exist in the memory, i.e. until the `Growing` instance exists and until next call of `compute` function. If you need the data later, you can move it outside of the class, but make sure you save also the `pixel_mat` data. You can either use the C++ move semantics or more preferably get the unique_ptr instances by calling `clear_groups` and `clear_cellmat` which automatically resets the members to nullptr.

#### Voronoi class
TODO

#### Separator class
TODO


## Modes

