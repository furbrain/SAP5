#!/usr/bin/python

import numpy as np
import re


def bracketiser(arr):
    arr = str(arr).replace('  ',' ')
    arr = arr.replace('  ',' ')
    arr = arr.replace('  ',' ')
    arr = arr.replace('  ',' ')
    arr = arr.replace('[ ','[')    
    arr = arr.replace(' ]',']')    
    arr = arr.replace('[','{')
    arr = arr.replace(']]','}]')
    arr = arr.replace(']','},')
    arr = re.sub('([0123456789\.]) ','\\1,', arr)
    arr = arr.replace('\n','')
    arr = arr.replace(',}','}')
    arr = re.sub(',$','',arr)
    return arr
    
def get_random_matrix(scale=2):
    m = (np.random.random((4,4))*scale*2)-scale
    m[3][0:3] = 0
    m[3][3] = 1
    return m

def get_random_vector():
    v = (np.random.random(4)*4)-2
    v[3] = 1
    return v
    
def to_points(x):
    return [math.sin(x)*SCALE, math.cos(x)*SCALE]
    
def get_2d_rotation_matrix(theta):
    c, s = np.cos(theta), np.sin(theta)
    return np.matrix([[c, s], [-s, c]])

def create_2d_raw_data(sample_count, theta, offset, noise=0, scale=10, skew=1.2):
    angles = np.random.random(sample_count)*np.pi*2
    points = np.array([np.sin(angles)*scale, np.cos(angles)*scale]).T
    points += np.random.random((sample_count,2))*noise-(noise/2)
    mat = create_2d_rotation_and_scale_matrix(theta, skew)
    points = np.matmul(points, mat)
    points += offset
    return points

def create_2d_rotation_and_scale_matrix(theta, skew):
    rot_mat = get_2d_rotation_matrix(theta)
    scale_mat = np.matrix([[skew,0], [0,1]])
    return np.matmul(np.matmul(np.linalg.inv(rot_mat), scale_mat), rot_mat)


def make_rotation_matrix(theta, axes):
    m = np.identity(4)
    c,s = (np.cos(theta), np.sin(theta))
    a,b = axes[0],axes[1]
    m[a][a] = c
    m[a][b] = s
    m[b][a] = -s
    m[b][b] = c
    return m
    
def make_scale_matrix(scale, axis):
    m = np.identity(4)
    m[axis][axis] = scale
    return m

def make_2d_rotation_fixture():
    axes = np.arange(3)
    for i in range(10):
        if (i==0):
            calib = np.identity(4)
            calib[0][3]=2
            calib[1][3]=3
            theta = np.pi/2
        else:
            calib = get_random_matrix()
            theta = np.random.random() *2 *np.pi
        vec = (np.cos(theta), np.sin(theta))
        np.random.shuffle(axes)
        original = calib.copy();
        calib = np.matmul(make_rotation_matrix(theta, axes[0:2]), calib)
        print "{%s, %s, %s,  %s}," % (bracketiser(axes[0:2]), bracketiser(vec), bracketiser(original[0:3]), bracketiser(calib[0:3]))
        
def make_scale_fixture():
    axes = np.arange(3)
    for i in range(10):
        if (i==0):
            calib = np.identity(4)
            calib[0][3]=2
            calib[1][3]=3
            scale = 3
        else:
            calib = get_random_matrix()
            scale = np.random.random()*4-2
        axis = np.random.randint(3)
        original = calib.copy();
        calib = np.matmul(make_scale_matrix(scale, axis), calib)
        print "{%d, %f, %s,  %s}," % (axis, scale, bracketiser(original[0:3]), bracketiser(calib[0:3]))

def make_multiply_matrices_fixture():
    for k in range(2):
        calib = np.identity(4)
        for i in range(5):
            delta = get_random_matrix(1.0)
            calib = np.matmul(delta, calib)
            print "{%s, %s}," % (bracketiser(delta[0:3]), bracketiser(calib[0:3]))
            
def make_normalise_fixture():
    for i in range(10):
        vector = get_random_vector()[:3]
        original = vector.copy()
        vector /= np.linalg.norm(vector)
        print "{%s, %s}," % (bracketiser(original), bracketiser(vector))
            
def make_find_rotation_and_scale_fixture(samples, skew=1.2, noise=0):
    theta = np.random.random()*np.pi
    points = create_2d_raw_data(samples, theta, [0,0])
    vectors = np.zeros((samples,3))
    axes = np.arange(3)
    np.random.shuffle(axes)
    a = int(axes[0])
    b = int(axes[1])
    vectors[:, a:a+1] = points[:, 0]
    vectors[:, b:b+1] = points[:, 1]
    print theta
    print skew
    print axes
    print bracketiser(vectors)

np.set_printoptions(suppress=True, precision=4)
np.random.seed(10)
make_find_rotation_and_scale_fixture(20)
make_find_rotation_and_scale_fixture(20)
make_find_rotation_and_scale_fixture(20)

