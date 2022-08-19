from enum import Enum
import numpy as np
import matplotlib
from matplotlib import pyplot as plt
from PIL import Image
import functools

colors = plt.rcParams['axes.prop_cycle'].by_key()['color']
cmap = matplotlib.colors.ListedColormap(list(map(matplotlib.colors.to_rgb, colors*99999)))
#cmap = plt.get_cmap('tab20c')


State = Enum('State', 'unseen opened closed')
Neighborhood = Enum('Neughborhood', 'n4 n8 swap')

def nop(x):
    pass

"""
class MySet:
    def __init__(self):
        self.container = dict()
    def __len__(self):
        return len(self.container)
    def __getitem__(self,key):
        return self.container[key]
    def add(self, item):
        self.container[item] = item
    def remove(self, item):
        self.container.pop(item)
    def __iter__(self):
        return self.container.__iter__()
    def __contains__(self,item):
        return self.container.__contains__(item)
    def __repr__(self):
            return set(self.container.keys()).__repr__()
    def __str__(self):
            return set(self.container.keys()).__str__()
"""

class Cell:
    def __init__(self, row, col, state=State.unseen):
        self.row = row
        self.col = col
        self.state = state

    def __str__(self):
        return f"<{self.row}, {self.col}, {self.state}>"
    def __repr__(self):
        return f"<{self.row}, {self.col}, {self.state}>"
    def __hash__(self):
        return hash((self.row,self.col))
    def __eq__(self, other):
        return hash(self) == hash(other)

def get_neighbour_indices(cell, bool_4_8, width, height):
    if cell.row == 0:
        row_delta = [0,1]
    elif cell.row == height-1:
        row_delta = [-1,0]
    else:
        row_delta = [-1,0,1]

    if cell.col == 0:
        col_delta = [0,1]
    elif cell.col == width-1:
        col_delta = [-1,0]
    else:
        col_delta = [-1,0,1]

    if bool_4_8:
        for r in row_delta:
            if r == 0: continue
            yield (cell.row+r, cell.col)
        for c in col_delta:
            if c == 0: continue
            yield (cell.row, cell.col+c)
    else:
        for r in row_delta:
            for c in col_delta:
                if r == 0 and c == 0: continue
                yield (cell.row+r, cell.col+c)

def get_neighbours(cell, bool_4_8, width, height, cellMat):
    return [ cellMat[r][c] for r,c in get_neighbour_indices(cell, bool_4_8, width, height) ]


def growing(data, init_funct, post_funct=nop, neighborhood=Neighborhood.n4):
    bool_4_8 = (neighborhood == Neighborhood.n4)
    opened,cellMat = init_funct()
    height = len(cellMat)
    width = len(cellMat[0])
    steps = 0

    processed = []
    while len(opened) > 0:
        new = set()
        for cell in opened:
            for neighbour in get_neighbours(cell, bool_4_8, width, height, cellMat): #TODO: edit neighbours
            #for neighbour in get_neighbours(x, np.random.random()>0.5, width, height, mat):

                if neighbour.state == State.unseen:
                    new.add(neighbour)
                    data[neighbour.row,neighbour.col] = data[cell.row,cell.col]
                    neighbour.state = State.opened
            
            cell.state = State.closed
            processed.append(cell)
        
        if neighborhood == Neighborhood.swap: bool_4_8 = not bool_4_8
        opened = new
        steps += 1
    
    post_funct(processed)
    return steps

def voronoi(data):
    def init_funct():
        height = len(data)
        width = len(data[0])
        opened = set()

        cellMat = [[None for col in range(width)] for row in range(height)]
        for row in range(len(cellMat)):
            for col in range(len(cellMat[0])):
                x = data[row][col]

                if x != 0:
                    x = Cell(row, col, State.opened)
                    opened.add(x)
                    cellMat[row][col] = x
                else:
                    x = Cell(row, col, State.unseen)
                    cellMat[row][col] = x

        return opened,cellMat

    growing(data, init_funct, neighborhood=Neighborhood.swap)
    return data



def cluster(data, treshold=50):
    
    def init_funct(cls):
        opened = set()
        i = np.argpartition(  -(data.reshape(-1) == 0).astype(int) , 1  )[0]
        cell = cellMat[i//width][i%width]

        if data[cell.row,cell.col] == 0:
            cell.state = State.opened
            data[cell.row,cell.col] = cls
            opened.add(cell)
        return opened,cellMat

    def post_funct(processed, treshold):
        if len(processed) < treshold:
            for cell in processed:
                data[cell.row,cell.col] = -1
            
    height = len(data)
    width = len(data[0])
    data = (data > 0).astype(int) - 1
    cellMat = [[None for col in range(width)] for row in range(height)]

    for row in range(height):
        for col in range(width):
            cellMat[row][col] = Cell(row, col, State.unseen if data[row][col] == 0 else State.closed)

    post = functools.partial(post_funct, treshold=treshold)
    n = 1
    while True:
        init = functools.partial(init_funct, cls=n)
        steps = growing(data, init, post_funct=post, neighborhood=Neighborhood.n4)

        if steps == 0:
            return data+1
        else:
            n += 1
    

data1 = np.array(
[[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9],
 [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
 [0,0,1,0,0,0,0,0,0,0,0,0,0,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
 [0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
 [0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
 [0,0,0,0,0,2,0,0,0,0,0,0,0,4,0,0,1,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
 [0,0,0,0,2,0,0,0,0,0,0,0,0,4,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
 [0,0,0,2,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
 [0,0,2,0,0,0,0,0,0,0,0,0,0,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
 [0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
 [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
 [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
 [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
 [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
 [0,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
 [0,0,0,0,0,0,0,0,0,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
 [0,0,0,0,0,0,0,0,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
 [0,0,0,5,0,0,0,3,3,3,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
 [0,0,0,0,0,0,3,3,3,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
 [0,0,0,0,0,3,3,3,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,8,0,0,0,0,0,0,0,0,0],
 [0,0,0,0,3,3,3,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,8,0,0,0,0,0,0,0,0,0,0],
 [0,0,0,3,3,3,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,8,0,0,0,0,0,0,0,0,0,0,0],
 [0,0,3,3,3,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,8,0,0,0,0,0,0,0,0,0,0,0,0],
 [0,3,3,3,0,0,0,0,6,0,0,3,0,0,0,0,7,7,7,0,0,0,0,0,0,8,8,0,0,0,0,0,0,0,0,0,0,0,0,0],
 [0,3,3,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,7,7,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
 [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,7,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
 [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,7,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
 [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,7,7,0,0,0,0,0,0,0,0,0,0,0,0,0],
 [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,7,7,0,0,0,0,0,0,0,0,0,0,0],
 [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]])


img = np.array(  Image.open('kupka_1.png') )
data = np.array( Image.open('kupka_pts_1.png').convert('L') )

n=1
data = data[:data.shape[0]//n, :data.shape[1]//n]

mat = cluster(data)

plt.imshow(cmap(mat), interpolation='nearest')
plt.show()

v = voronoi(mat)


plt.imshow(cmap(v), interpolation='nearest')
plt.show()

plt.imshow(cmap(v)*(data==0).reshape(v.shape+(1,)), interpolation='nearest')
plt.show()
