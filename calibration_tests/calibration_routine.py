#!/usr/bin/python3
import copy
import sys

import matplotlib.pyplot as plt
import numpy as np
import scipy
import scipy.linalg
import scipy.optimize
import scipy.stats
from matplotlib.widgets import Slider
from numpy.polynomial import Polynomial
from scipy.spatial.transform import Rotation as R

# co-ord system
# Device Axes:
# X = towards top of display with laser pointing left
# Y = along laser
# Z = coming out of surface of display upwards...

# World Axes
# X = East
# Y = North
# Z = Up
import readings
from calset import CalSet, graph

RESOLUTION = 100
TEST_COUNT = 50
np.set_printoptions(suppress=True)


def show_points(data, autoscale=False, labels=None):
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d', proj_type="ortho")
    if not autoscale:
        ax.autoscale(enable=False)
        ax.set_xlim(-1, 1)
        ax.set_ylim(-1, 1)
        ax.set_zlim(-1, 1)

    ax.plot(data[:, 0],
            data[:, 1],
            data[:, 2], )
    if labels is not None:
        for x, y, z, label in zip(data[:, 0], data[:, 1], data[:, 2], labels):
            ax.text(x, y, z, str(label))
    plt.show()


def get_offset_cals(xz, cals):
    c = copy.deepcopy(cals)
    c.mag_cal.apply_matrix(get_shears(xz))
    # c.mag_cal.second_order = xz
    return c


def get_shears(x):
    return np.array([[1, x[0], x[1]], [0, 1, 0], [x[2], x[3], 1]]).T


def minimisation_xz_function(rots, cals, mag, grav):
    # rots is a set of 6 rotations, 3 for mag, 3 for grav
    c = get_offset_cals(rots, cals)
    return c.check_alignment2(mag[8:], grav[8:]) + c.check_uniformity_multiple(mag, grav) / 4


def minimisation_mat_function(vector, cals, mag, grav):
    c = CalSet.from_vector(vector)
    return c.check_alignment2(mag[8:], grav[8:]) + c.check_uniformity_multiple(mag, grav)


def minimize_matrix(cals, mag, grav):
    x0 = cals.to_vector()
    print("Starting minimization: ", minimisation_mat_function(x0, cals, mag, grav))
    print(x0.reshape((-1, 3)))
    ret = scipy.optimize.minimize(lambda x: minimisation_mat_function(x, cals, mag, grav), x0, method="Nelder-Mead",
                                  options={"disp": True, "maxiter": 20000})
    print(ret.fun, "\n", ret.x.reshape((-1, 3)))
    print(ret.success, ret.message)
    return CalSet.from_vector(ret.x)




def minimize_offset(cals, mag, grav):
    x0 = np.zeros(4)
    print("Starting minimization: ", minimisation_xz_function(x0, cals, mag, grav))
    ret = scipy.optimize.minimize(lambda x: minimisation_xz_function(x, cals, mag, grav), x0, method="Nelder-Mead",
                                  options={"disp": True, "maxiter": 20000})
    print(ret)
    return get_offset_cals(ret.x, cals)


def mag_only(x, y, cals, mag, grav):
    rots = [0, y, x, 0]
    return minimisation_function(rots, cals, mag, grav)


def plot_energy_function(cals: CalSet, mag, grav):
    x = y = np.outer(np.linspace(-5, 5, 30), np.ones(30))
    y = x.copy().T  # transpose
    null = np.zeros_like(x)
    z = np.vectorize(lambda x1, y1: mag_only(x1, y1, cals, mag, grav))(x, y)
    fig = plt.figure()
    print(x.shape)
    print(y.shape)
    print(z.shape)
    ax = plt.axes(projection='3d')
    ax.plot_surface(x, y, z, cmap='viridis', edgecolor='none')
    ax.set_title('Surface plot')
    plt.show()


def get_graphs(cals: CalSet, mag: np.ndarray, grav: np.ndarray):
    m = cals.mag_cal.apply(mag)
    g = cals.grav_cal.apply(grav)
    rotations = np.arctan2(g[:, 0], g[:, 2])
    r = R.from_euler("y", -rotations)
    g1 = r.apply(g)
    # rotations = np.arctan2(g1[:,1], g1[:,2])
    east, north, orientation = cals.get_orientation(mag, grav)
    compass = np.arctan2(east[:, 1], north[:, 1])
    errors = compass - np.mean(compass)
    compass = np.rad2deg(compass)
    compass = [f"{i}: {x:3.2f}" for i, x in enumerate(compass)]
    m1 = r.apply(m)
    m_bar = np.mean(m1, axis=0)
    m2 = r.inv().apply(m_bar)
    m3 = m - m2
    return rotations, m3, errors, (np.linalg.norm(m, axis=1) - 1) ** 2


def show_rotated_positions(cals: CalSet, mag, grav):
    fig = plt.figure()
    ax = fig.add_subplot(211)

    def fit_pol(params):
        cals.mag_cal.second_order[0] = Polynomial(params[:len(params) // 2])
        cals.mag_cal.second_order[2] = Polynomial(params[len(params) // 2:])
        rotations, m3, error, m = get_graphs(cals, mag, grav)
        return error + m

    rotations, m3, error, _ = get_graphs(cals, mag, grav)
    rotations = np.rad2deg(rotations)
    plots = []
    plots.append(ax.plot(rotations, m3[:, 0], 'r-')[0])
    plots.append(ax.plot(rotations, m3[:, 2], 'g-')[0])
    plots.append(ax.plot(rotations, error, 'b-')[0])
    # ax.set_ylim(-0.1, 0.1)
    axx = fig.add_subplot(212)
    # axz = fig.add_subplot(313)
    sx = Slider(axx, "X offset", 1, 7, 1, valstep=1)

    # sz = Slider(axz, "Z offset", -0.2, 0.2, 0)

    def update(val):
        cals.mag_cal.second_order[0] = sx.val
        x0 = np.zeros((int(sx.val) * 2))
        ret = scipy.optimize.least_squares(fit_pol, x0)
        print(ret)
        params = ret.x
        cals.mag_cal.second_order[0] = Polynomial(params[:len(params) // 2])
        cals.mag_cal.second_order[2] = Polynomial(params[len(params) // 2:])
        rotations, m3, error, _ = get_graphs(cals, mag, grav)
        rotations = np.rad2deg(rotations)
        plots[0].set_xdata(rotations)
        plots[0].set_ydata(m3[:, 0])
        plots[1].set_xdata(rotations)
        plots[1].set_ydata(m3[:, 2])
        plots[2].set_xdata(rotations)
        plots[2].set_ydata(error)
        # ax.autoscale(True)

    sx.on_changed(update)
    # sz.on_changed(update)

    plt.show()


def print_neatness(calibs: CalSet, r: readings.ReadingSet):
    print(calibs.check_uniformity_multiple(r))
    print(calibs.check_alignment2(r))


# do_test_runs()
if len(sys.argv) > 1:
    readingset = readings.load_from_npz(sys.argv[1])
else:
    readingset = readings.load_from_npz("data.npz")

cals = CalSet()
print("uncalibrated")
print_neatness(cals, readingset)

print("\nunaligned")
cals.fit_ellipsoid(readingset)
print_neatness(cals, readingset)

#cals.grav_cal.fit_higher_order(g)
print_neatness(cals, readingset)

cals = cals.minimize(readingset)
print_neatness(cals, readingset)

print("\nhigher order mag")
#cals.mag_cal.fit_higher_order(m)
linear = [[], [], []]
uniformity = [cals.check_uniformity_multiple(readingset)]
accuracy = [cals.check_alignment2(readingset)]
combined = [accuracy[-1]+uniformity[-1]]
for k in range(1, 7):
    min_cals = cals.full_calibration(readingset, order=k)
    print(k, ": ", min_cals.mag_cal.second_order)
    accuracy.append(min_cals.check_alignment2(readingset))
    uniformity.append(min_cals.check_uniformity_multiple(readingset))
    combined.append(accuracy[-1]+uniformity[-1])
    for i in range(3):
        x, y = min_cals.mag_cal.second_order[i].linspace(100, (-1.3, 1.3))
        linear[i].append(y)
graph(list(range(0,7)), accuracy, uniformity, combined, labels=["accuracy", "uniformity", "combined"])
for i in range(3):
    graph(x, *linear[i], labels=range(1,7))
print(cals.mag_cal.second_order)
print_neatness(cals, readingset)
print(cals.check_accuracy(readingset.aligned[0], display=True))
print(cals.check_accuracy(readingset.aligned[1], display=True))

