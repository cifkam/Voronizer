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
    def __init__(self, row, col, cls, state=State.unseen):
        self.row = row
        self.col = col
        self.cls = cls
        self.state = state

    def __str__(self):
        return f"<{self.cls}>"
    def __repr__(self):
        return f"<{self.cls}>"
    def __hash__(self):
        return hash((self.row,self.col))
    def __eq__(self, other):
        return hash(self) == hash(other)

def cells_to_numpy(m):
        return np.array(list(map(lambda x: list(map(lambda y: y.cls, x)), m)))

def get_neighbour_indices(x, bool_4_8, width, height):
    if x.row == 0:
        row_delta = [0,1]
    elif x.row == height-1:
        row_delta = [-1,0]
    else:
        row_delta = [-1,0,1]

    if x.col == 0:
        col_delta = [0,1]
    elif x.col == width-1:
        col_delta = [-1,0]
    else:
        col_delta = [-1,0,1]

    if bool_4_8:
        for r in row_delta:
            if r == 0: continue
            yield (x.row+r, x.col)
        for c in col_delta:
            if c == 0: continue
            yield (x.row, x.col+c)
    else:
        for r in row_delta:
            for c in col_delta:
                if r == 0 and c == 0: continue
                yield (x.row+r, x.col+c)

def get_neighbours(x, bool_4_8, width, height, mat):
    return [ mat[r][c] for r,c in get_neighbour_indices(x, bool_4_8, width, height) ]


def growing(data, init_funct, post_funct=nop, neighborhood=Neighborhood.n4):
    bool_4_8 = (neighborhood == Neighborhood.n4)
    height = len(data)
    width = len(data[0])
    opened,mat = init_funct()
    steps = 0

    processed = []
    while len(opened) > 0:
        new = set()
        for x in opened:
            for neighbour in get_neighbours(x, bool_4_8, width, height, mat):
            #for neighbour in get_neighbours(x, np.random.random()>0.5, width, height, mat):

                if neighbour.state == State.unseen:
                    new.add(neighbour)
                    neighbour.cls = x.cls
                    neighbour.state = State.opened
            
            x.state = State.closed
            processed.append(x)
        
        if neighborhood == Neighborhood.swap: bool_4_8 = not bool_4_8
        opened = new
        steps += 1
    
    post_funct(processed)
    return mat, steps

def voronoi(data):
    def init_funct():
        height = len(data)
        width = len(data[0])
        opened = set()

        mat = [[None for col in range(width)] for row in range(height)]
        for row in range(len(mat)):
            for col in range(len(mat[0])):
                x = data[row][col]

                if x != 0:
                    x = Cell(row, col, x, State.opened)
                    opened.add(x)
                    mat[row][col] = x
                else:
                    x = Cell(row, col, x, State.unseen)
                    mat[row][col] = x

        return opened,mat

    return cells_to_numpy(growing(data, init_funct, neighborhood=Neighborhood.swap)[0])



def cluster(data, treshold=50):
    
    def init_funct(cls):
        opened = set()
        i = np.argpartition(  -(cells_to_numpy(mat).reshape(-1) == 0).astype(int) , 1  )[0]
        x = mat[i//width][i%width]

        if x.cls == 0:
            x.state = State.opened
            x.cls = cls
            opened.add(x)
        return opened,mat

    def post_funct(processed, treshold):
        if len(processed) < treshold:
            for x in processed:
                x.cls = -1
            
    height = len(data)
    width = len(data[0])
    data = (data > 0).astype(int) - 1
    mat = [[None for col in range(width)] for row in range(height)]

    for row in range(height):
        for col in range(width):
            mat[row][col] = Cell(row, col, data[row][col], State.unseen if data[row][col] == 0 else State.closed)

    post = functools.partial(post_funct, treshold=treshold)
    n = 1
    while True:
        init = functools.partial(init_funct, cls=n)
        mat,steps = growing(mat, init, post_funct=post, neighborhood=Neighborhood.n4)

        if steps == 0:
            return cells_to_numpy(mat)+1
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

plt.imshow(cmap(v)*(mat==0).reshape(v.shape+(1,)), interpolation='nearest')
plt.show()
