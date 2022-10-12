# Examples

## Colormaps
The image can be colorized by OpenCV's colormaps instead of using the original image as color template. This can be done by specifying the name of colormap by `-c` option (see [OpenCV documentation](https://docs.opencv.org/4.4.0/d3/d50/group__imgproc__colormap.html#ga9a805d8262bcbe273f16be9ea2055a65) for the detailed view of all colormaps with images). The color depends on the x a y coordinates of each area (but also on implementation of selected mode). This can be prevented by randomizing the colors by using `-r` flag, as seen in the second row of the displayd examples:\

| bone | cividis | turbo| twilight |
| --- | --- | --- | --- |
| <img src="examples/cmap_bone.png" width="400"/>        | <img src="examples/cmap_cividis.png" width="400"/>        | <img src="examples/cmap_turbo.png" width="400"/>        | <img src="examples/cmap_twilight.png" width="400"/> |
| <img src="examples/cmap_bone_random.png" width="400"/> | <img src="examples/cmap_cividis_random.png" width="400"/> | <img src="examples/cmap_turbo_random.png" width="400"/> | <img src="examples/cmap_twilight_random.png" width="400"/> |

## Smoothing
Without any smoothing the edges of areas are quite hard and unpleasant. To avoid this, level of edge smoothing can be defined by `-s` option:

| 0 | 3 | 10 | 20 |
| --- | --- | --- | --- |
| <img src="examples/smooth_0.png" width="400"/> | <img src="examples/smooth_3.png" width="400"/> | <img src="examples/smooth_10.png" width="400"/> | <img src="examples/smooth_20.png" width="400"/> |

## Modes
### sobel
| default | -o ,10 | -o ,100 | -o 0,30,0 |
| --- | --- | --- | --- |
| <img src="examples/lena-sobel-default.png" width="400"/> | <img src="examples/lena-sobel-_10.png" width="400"/> | <img src="examples/lena-sobel-_100.png" width="400"/> | <img src="examples/lena-sobel-0_30_0.png" width="400"/> |
| <img src="examples/carqueiranne-sobel-default.png" width="400"/> | <img src="examples/carqueiranne-sobel-_10.png" width="400"/> | <img src="examples/carqueiranne-sobel-_100.png" width="400"/> | <img src="examples/carqueiranne-sobel-0_30_0.png" width="400"/> |
| <img src="examples/collobrieres-sobel-default.png" width="400"/> | <img src="examples/collobrieres-sobel-_10.png" width="400"/> | <img src="examples/collobrieres-sobel-_100.png" width="400"/> | <img src="examples/collobrieres-sobel-0_30_0.png" width="400"/> |

### kmeans-circles
| default | -o ,30,5 | -o 1,,3,14,2 | -o ,,20,30,2 |
| --- | --- | --- | --- |
| <img src="examples/lena-kmeans-circles-default.png" width="400"/> | <img src="examples/lena-kmeans-circles-_30_5.png" width="400"/> | <img src="examples/lena-kmeans-circles-1__3_14_2.png" width="400"/> | <img src="examples/lena-kmeans-circles-__20_30_2.png" width="400"/> |
| <img src="examples/carqueiranne-kmeans-circles-default.png" width="400"/> | <img src="examples/carqueiranne-kmeans-circles-_30_5.png" width="400"/> | <img src="examples/carqueiranne-kmeans-circles-1__3_14_2.png" width="400"/> | <img src="examples/carqueiranne-kmeans-circles-__20_30_2.png" width="400"/> |
| <img src="examples/collobrieres-kmeans-circles-default.png" width="400"/> | <img src="examples/collobrieres-kmeans-circles-_30_5.png" width="400"/> | <img src="examples/collobrieres-kmeans-circles-1__3_14_2.png" width="400"/> | <img src="examples/collobrieres-kmeans-circles-__20_30_2.png" width="400"/> |

### kmeans-lines
| default | -o ,,1  | -o ,,5,10 | -o ,,5,100 |
| --- | --- | --- | --- |
| <img src="examples/lena-kmeans-lines-default.png" width="400"/> | <img src="examples/lena-kmeans-lines-__1.png" width="400"/> | <img src="examples/lena-kmeans-lines-__5_10.png" width="400"/> | <img src="examples/lena-kmeans-lines-__5_100.png" width="400"/> |
| <img src="examples/carqueiranne-kmeans-lines-default.png" width="400"/> | <img src="examples/carqueiranne-kmeans-lines-__1.png" width="400"/> | <img src="examples/carqueiranne-kmeans-lines-__5_10.png" width="400"/> | <img src="examples/carqueiranne-kmeans-lines-__5_100.png" width="400"/> |
| <img src="examples/collobrieres-kmeans-lines-default.png" width="400"/> | <img src="examples/collobrieres-kmeans-lines-__1.png" width="400"/> | <img src="examples/collobrieres-kmeans-lines-__5_10.png" width="400"/> | <img src="examples/collobrieres-kmeans-lines-__5_100.png" width="400"/> |

### sift-circles
| default | -o 0,,5,0.6 | -o 3,,,0.5 | -o ,,1,2 |
| --- | --- | --- | --- |
| <img src="examples/lena-sift-circles-default.png" width="400"/> | <img src="examples/lena-sift-circles-0__5_0.6.png" width="400"/> | <img src="examples/lena-sift-circles-3___0.5.png" width="400"/> | <img src="examples/lena-sift-circles-__1_2.png" width="400"/> |
| <img src="examples/carqueiranne-sift-circles-default.png" width="400"/> | <img src="examples/carqueiranne-sift-circles-0__5_0.6.png" width="400"/> | <img src="examples/carqueiranne-sift-circles-3___0.5.png" width="400"/> | <img src="examples/carqueiranne-sift-circles-__1_2.png" width="400"/> |
| <img src="examples/collobrieres-sift-circles-default.png" width="400"/> | <img src="examples/collobrieres-sift-circles-0__5_0.6.png" width="400"/> | <img src="examples/collobrieres-sift-circles-3___0.5.png" width="400"/> | <img src="examples/collobrieres-sift-circles-__1_2.png" width="400"/> |

### sift-lines
| default | -o 0 | -o 3,20, | -o 1,100 |
| --- | --- | --- | --- |
| <img src="examples/lena-sift-lines-default.png" width="400"/> | <img src="examples/lena-sift-lines-0.png" width="400"/> | <img src="examples/lena-sift-lines-3_20_.png" width="400"/> | <img src="examples/lena-sift-lines-1_100.png" width="400"/> |
| <img src="examples/carqueiranne-sift-lines-default.png" width="400"/> | <img src="examples/carqueiranne-sift-lines-0.png" width="400"/> | <img src="examples/carqueiranne-sift-lines-3_20_.png" width="400"/> | <img src="examples/carqueiranne-sift-lines-1_100.png" width="400"/> |
| <img src="examples/collobrieres-sift-lines-default.png" width="400"/> | <img src="examples/collobrieres-sift-lines-0.png" width="400"/> | <img src="examples/collobrieres-sift-lines-3_20_.png" width="400"/> | <img src="examples/collobrieres-sift-lines-1_100.png" width="400"/> |

