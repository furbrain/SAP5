#!/usr/bin/python
import matplotlib.pyplot as plt
import numpy as np
import math


SCALE = 10
SKEW = 1.2
NOISE = 0
def to_points(x):
    return [math.sin(x)*SCALE, math.cos(x)*SCALE]
    
def get_rotation_matrix(theta):
    c, s = np.cos(theta), np.sin(theta)
    return np.matrix([[c, s], [-s, c]])

def create_raw_data(sample_count, theta, offset):
    angles = np.random.random(sample_count)*np.pi*2
    points = np.array([np.sin(angles)*SCALE, np.cos(angles)*SCALE]).T
    points += np.random.random((sample_count,2))*NOISE-(NOISE/2)
    mat = create_rotation_and_scale_matrix(theta, SKEW)
    points = np.matmul(points, mat)
    points += offset
    return points

def create_rotation_and_scale_matrix(theta, skew):
    rot_mat = get_rotation_matrix(theta)
    scale_mat = np.matrix([[skew,0], [0,1]])
    return np.matmul(np.matmul(np.linalg.inv(rot_mat), scale_mat), rot_mat)
    
def run_test(sample_count, algorithm):
    offset = np.random.random(2)*20-10
    theta = np.random.random()*np.pi*2
    #theta = 30*np.pi/180
    points = create_raw_data(sample_count, theta, offset)
    new_points, new_offset, new_theta = algorithm(points)
    print offset-new_offset, (theta-new_theta) * 180.0/np.pi
    return new_points
    
    
def pca(points):
    offset = np.mean(points,0)
    points -= offset
    vals, vectors = np.linalg.eig(np.cov(points.T))
    if vals[0]>vals[1]:
        theta = np.arctan2(vectors[0][1], vectors[0][0])
        skew = vals[0]/vals[1]
    else:        
        theta = np.arctan2(vectors[1][1], vectors[1][0])
        skew = vals[1]/vals[0]
    mat = create_rotation_and_scale_matrix(-theta,skew)
    return np.matmul(points, mat), offset, -theta

def idiot(points):
    max_p = np.max(points,0)
    min_p = np.min(points,0)
    offset = (max_p+min_p)/2
    points -=offset
    sc = 0
    theta = 0
    for i in range(180):
        temp_theta = i*np.pi/180
        temp_mat = get_rotation_matrix(temp_theta)
        temp_points = np.matmul(points, temp_mat)
        shape = np.max(np.abs(temp_points),0)
        temp_sc = shape.item(0)/shape.item(1)
        if temp_sc > sc:
            sc = temp_sc
            theta = temp_theta
    mat = create_rotation_and_scale_matrix(-theta, 1/sc)
    return np.matmul(points, mat), offset, -theta

for i in range(30):
    run_test(50, idiot)

