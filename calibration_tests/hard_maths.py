#!/usr/bin/python
import re
import numpy as np
import scipy.linalg
import scipy.stats
import quaternion
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

#co-ord system
## Device Axes:
#X = towards top of display with laser pointing left
#Y = along laser
#Z = coming out of surface of display upwards...

# World Axes
#X = East
#Y = North
#Z = Up

RESOLUTION = 100
TEST_COUNT = 50
np.set_printoptions(suppress=True)
#np.random.seed(0)

MAGNETISM = np.array([-3.300, 11.846, 8.975]) #magnitude 1!

def normalise(vectors):
    return vectors/np.linalg.norm(vectors, axis=0)

class Sensor():
    def __init__(self, offset_error=0, scale_error=1.0, shear1=1.0, shear2=1.0, noise = 0):
        self.offset = np.random.normal(0, offset_error, 3)
        self.scale = np.array([scale_error,1,1])
        #self.scale = np.power(10,np.random.normal(0, scale_error, 3))
        #create a "pure shear"
        q = quaternion.quaternion(*np.random.random(4)*2-1)
        r = quaternion.as_rotation_matrix(q)
        self.shear =r.dot(np.diag((shear1, shear2, 1))).dot(np.linalg.inv(r))
        self.noise = noise
        self.transform = self.shear.dot(np.diag(self.scale))
        
    def get_readings(self, vectors):
        results = vectors.dot(self.transform)
        #results *= self.scale
        results += self.offset
        results = np.random.normal(results, scale=self.noise)
        return results
        
    def get_multiple_readings(self, vectors, count):
        results = np.array([self.get_readings(vectors) for i in range(count)])
        return np.mean(results, axis=0)
    
        
class Calibration():
    def __init__(self, readings):
        x = readings[:,0]
        y = readings[:,1]
        z = readings[:,2]
        alternate = False
        if alternate:
            D = np.stack((x*x+y*y-2*z*z, 
                          x*x-2*y*y+z*z,
                          4*x*y,
                          2*x*z,
                          2*y*z,
                          2*x,
                          2*y,
                          2*z,
                          np.ones_like(x)))
            E = x*x+y*y+z*z
            u = np.zeros(10)
            u[1:10] = np.linalg.lstsq(D.T,E)[0]
            S1 = np.array([[3, 1, 1],
                           [3, 1,-2],
                           [3,-2, 1]])
            S = np.identity(10)
            S[3,3] = 2
            S[0:3, 0:3] = S1
            u[0] = -1.0/3.0
            v1 = S.dot(u)
            v = v1[0:9]/-v1[9]
            a,b,c,d,e,f,g,h,i = v
        else:
            input_array = np.stack((x*x, y*y, z*z, 2*x*y, 2*x*z, 2*y*z, 2*x, 2*y, 2*z))
            output_array = np.ones_like(x)
            (coeff, accuracy, rank, values) =  np.linalg.lstsq(input_array.T, output_array)
            a, b, c, d, e, f, g, h, i = coeff
        A4 = np.array([[a, d, e, g],
                       [d, b, f, h],
                       [e, f, c, i],
                       [g, h, i, -1]])
        A3 = A4[0:3, 0:3]
        vghi = np.array([-g, -h, -i])
    
        self.centre = np.linalg.lstsq(A3, vghi)[0]
        T = np.identity(4)
        T[3, 0:3] = self.centre
        B4 = T.dot(A4).dot(T.T)
        B3 = B4[0:3, 0:3] / -B4[3, 3]
        print B3
        e,v = np.linalg.eig(B3)
        print e,v
        self.transform = v.dot(np.sqrt(np.diag(e))).dot(v.T)
        #self.transform = scipy.linalg.sqrtm(B3)
        transformed_readings = self.apply(readings)
        norms = np.linalg.norm(transformed_readings,axis=1)
        self.mean = np.mean(norms)
        self.stdev = np.std(norms)

    def is_good(self):
        if np.isnan(self.mean): return False
        #if abs(1-self.mean) >0.05: return False
        if self.stdev > 0.05: return False
        return True

    def apply(self, vectors):
        return (vectors-self.centre).dot(self.transform)
        
    def make_ellipsoid(self, x, y, z):
        U, s, rotation = np.linalg.svd(self.transform ** 2)
        radii = 1.0/np.sqrt(s)
        x *= radii[0] 
        y *= radii[1]
        z *= radii[2]

        for i in range(len(x)):
            for j in range(len(x)):
                [x[i,j],y[i,j],z[i,j]] = np.dot([x[i,j],y[i,j],z[i,j]], rotation) + self.centre
        return x, y, z          

def generate_calibration_data(sensor, resolution=100, repeat=1):
    #orientation  = np.exp(quaternion.x * 0.1/2)
    orientation  = np.quaternion(1,0,0,0)
    orientation  = orientation * np.exp(quaternion.x * np.pi/4)

    chip_orientation = np.exp(quaternion.x * 0.1/2) * np.exp(quaternion.y*0.05/2) * np.exp(quaternion.z*0.08/2)
    z_movements = np.exp(quaternion.z * np.pi * np.random.random(resolution))
    y_movements = np.exp(quaternion.y * np.pi * np.random.random(resolution))
    x_movements = np.exp(quaternion.x * np.pi * np.random.random(resolution))
    movements = np.concatenate((z_movements, y_movements, x_movements))
    transforms = quaternion.as_rotation_matrix(orientation* movements * chip_orientation)
    vectors = transforms.dot(MAGNETISM)
    return sensor.get_multiple_readings(vectors,30)

def display_results(readings, calibration):
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    ax.autoscale(enable=False)
    ax.set_xlim(-10,10)
    ax.set_ylim(-10,10)
    ax.set_zlim(-10,10)

    u = np.linspace(0.0, 2.0 * np.pi, 100)
    v = np.linspace(0.0, np.pi, 100)
    x = np.outer(np.cos(u), np.sin(v))
    y = np.outer(np.sin(u), np.sin(v))
    z = np.outer(np.ones_like(u), np.cos(v))
    transformed_readings = calibration.apply(readings)*5

    ax.scatter(transformed_readings[:,0], 
               transformed_readings[:,1], 
               transformed_readings[:,2],
               s=2, color='orange')
    ax.plot_surface(x*5, y*5, z*5,  rstride=4, cstride=4, color='orange', alpha=0.2)

    x, y, z = calibration.make_ellipsoid(x, y, z)
    ax.scatter(readings[:,0], readings[:,1], readings[:,2],s=2, color='b')
    ax.plot_surface(x, y, z,  rstride=4, cstride=4, color='b', alpha=0.2)
    plt.show()

def test_calibration(sensor, calibration, reading_count):
    vectors = np.random.random((TEST_COUNT,3))*10-5
    readings = sensor.get_multiple_readings(vectors, reading_count)
    results = calibration.apply(readings)
    vectors = (vectors.T / np.linalg.norm(vectors, axis=1)).T
    results = (results.T / np.linalg.norm(results, axis=1)).T
    return np.mean(np.rad2deg(np.linalg.norm(vectors-results, axis=1)))
    
def run_tests(num_tests=30, resolution=80, offset_error=1, scale_error=1.0, shear1=1.0, shear2=1.0, noise = 0.0003, reading_count=1):
    r = []
    for i in range(num_tests):
        s = Sensor(offset_error, scale_error, shear1, shear2, noise)
        readings = generate_calibration_data(s, int(resolution))
        cal = Calibration(readings)
        if cal.is_good():
            r.append(test_calibration(s, cal, reading_count))
    return r

def do_test_runs():
    means = []
    stdevs = []
    x = []
    for i in np.geomspace(1,300,20):
        results = run_tests(num_tests=30, 
                            shear1=0.9, 
                            shear2 = 1.1, 
                            scale_error=1.01, 
                            noise=0.3, 
                            reading_count=40, 
                            resolution=int(i))
        means.append(np.mean(results))
        print i, len(results), np.mean(results)
        stdevs.append(scipy.stats.sem(results))
        x.append(i)

    fig = plt.figure()
    ax = fig.add_subplot(111)
    ax.set_xscale('log')
    ax.set_yscale('log')
    ax.errorbar(x, means, yerr=stdevs, ls='dotted', capsize=3)
    plt.show()
    
def get_coords(x, y, z):
    x, y, z = np.meshgrid(x, y, z)
    coords = np.stack((x.flatten(), y.flatten(), z.flatten())).T
    return coords
    
def generate_globe():
    x = np.linspace(-3.0,3.0,7)
    a = get_coords(x,x,3)    
    b = get_coords(x,x,-3)    
    c = get_coords(x,3,x)    
    d = get_coords(x,-3,x)    
    e = get_coords(3,x,x)    
    f = get_coords(-3,x,x)
    coords = np.unique(np.concatenate((a,b,c,d,e,f)).T, axis=0)
    coords = normalise(coords)
    
    return coords
    
def display_quiver(sensor, cal, reading_count):
    fig = plt.figure()
    ax = fig.add_subplot(111,projection="3d")
    coords = generate_globe()
    readings = sensor.get_multiple_readings(coords.T, reading_count)
    results = cal.apply(readings)
    results = normalise(results.T).T
    #readings = normalise(readings.T)
    x, y, z = coords
    u, v, w = (results.T-coords)
    ax.quiver(x,y,z,u,v,w)
    plt.show()
    

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


def do_single_run(): 
    s = Sensor(offset_error=1.0, scale_error=1.1, shear1=1.05, shear2=0.95, noise=0.29)
    readings = generate_calibration_data(s, 80)
    print bracketiser(readings)
    cal = Calibration(readings)
    print bracketiser(cal.transform)
    print bracketiser(cal.centre)
    print test_calibration(s, cal, reading_count=100)
    display_quiver(s, cal, 100)    

def show_points(data):
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    ax.autoscale(enable=False)
    ax.set_xlim(-1,1)
    ax.set_ylim(-1,1)
    ax.set_zlim(-1,1)

    ax.scatter(data[:,0], 
               data[:,1], 
               data[:,2])
    plt.show()

def find_plane(cal, data):
	data = cal.apply(data)
	centre = np.mean(data)
	new_data = data - centre
	output = np.ones_like(data[:,0])
	return normalise(np.linalg.lstsq(new_data,output)[0])

#do_test_runs()
data = np.load("data.npz")
m = data['m']
g = data['g']

cal = Calibration(m.T)
print cal.mean
print cal.stdev
print cal.transform
print cal.centre
spins = m.T[8:]
#show_points(cal.apply(spins))
print "lstsq"
v = find_plane(cal, spins)
print v
if v[1]< 0:
	v = v * -1
print v
exit()
z_magnitude = np.sqrt((v[0]*v[0]+v[1]*v[1]))
z_cos = v[1]/z_magnitude
z_sin = v[0]/z_magnitude
rot_z = np.array([[z_cos,z_sin,0],[-z_sin,z_cos,0],[0,0,1]])
print rot_z
x_magnitude = np.sqrt((v[2]*v[2]+v[1]*v[1]))
x_cos = v[1]/z_magnitude
x_sin = v[2]/z_magnitude
rot_x = np.array([[1,0,0],[0,x_cos,x_sin],[0,-x_sin,x_cos]])
print rot_x

rot = np.dot(rot_x,rot_z)
print rot
idx = np.arange(1, 8) - np.tri(8, 8-1, k=-1, dtype=bool)
print "laser_vectors"
for row in spins[idx]:
	print(find_plane(cal, row))
#display_results(m.T,cal)

