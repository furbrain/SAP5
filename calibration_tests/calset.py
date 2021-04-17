import copy

import numpy as np
import quaternion as q
import scipy
import scipy.optimize
from matplotlib import pyplot as plt
from numpy.polynomial import Polynomial
from scipy.spatial.transform import Rotation, Rotation as R

from rbf import RBF
from readings import ReadingSet, Readings

ORDER = 3

ORTHOGONAL = True

RBF_GAUSSIAN = True

MIN_DIP = False


def normalise(vectors):
    return (vectors.T / np.linalg.norm(vectors.T, axis=0)).T


class Calibration:
    def __init__(self):
        if ORTHOGONAL:
            if RBF_GAUSSIAN:
                self.second_order = [RBF([0])] * 3
            else:
                self.second_order = [Polynomial([0])] * 3
        else:
            self.second_order = [np.zeros((ORDER, ORDER, ORDER))] * 3
        self.centre = np.zeros(3)
        self.transform = np.identity(3)
        self.gaussians = None

    def fit(self, readings, rotated=True, non_linear=False):
        if rotated:
            if non_linear:
                self.non_linear_fit(readings)
            else:
                self.fit_rotated_ellipsoid(readings)
        else:
            self.fit_orthogonal_ellipsoid(readings)

    def non_linear_fit(self, readings):
        readings = self.apply_non_linear(readings)
        def min_sphere(x):
            return np.linalg.norm((readings - x[:3]) * x[3], axis=1) - 1

        def get_mat_and_center(x):
            Q = Rotation.from_euler("xyz", x[:3])
            S = np.diag(x[3:6])
            T = x[6:9]
            mat = Q.as_dcm() @ S @ Q.inv().as_dcm()
            return mat, T

        def minnow(x):
            mat, T = get_mat_and_center(x)
            results = np.linalg.norm((readings - T) @ mat, axis=1) - 1
            return results

        x0 = np.mean(readings, axis=0)
        results = scipy.optimize.least_squares(min_sphere, np.hstack((x0, [1])), method="lm", max_nfev=10000)
        print(results)
        x = results.x
        results = scipy.optimize.least_squares(minnow, np.array([0, 0, 0, x[3], x[3], x[3], x[0], x[1], x[2]]),
                                               method="lm", max_nfev=10000)
        print(results.success, results.message)
        print(results.x)
        self.transform, self.centre = get_mat_and_center(results.x)
        print(self.transform, self.centre)
        print(minnow(results.x) ** 2)

    def fit_orthogonal_ellipsoid(self, readings):
        readings = self.apply_non_linear(readings)
        x = readings[:, 0]
        y = readings[:, 1]
        z = readings[:, 2]
        input_array = np.stack((x * x, y * y, z * z, 2 * x, 2 * y, 2 * z))
        output_array = np.ones_like(x)
        (coeff, accuracy, rank, values) = np.linalg.lstsq(input_array.T, output_array, rcond=None)
        a, b, c, g, h, i = coeff
        self.centre = -coeff[3:] / coeff[:3]
        G = 1 + sum(coeff[3:] * coeff[3:] / coeff[:3])
        self.transform = np.diag(np.sqrt(coeff[:3] / G))

    def fit_rotated_ellipsoid(self, readings):
        readings = self.apply_non_linear(readings)
        x = readings[:, 0]
        y = readings[:, 1]
        z = readings[:, 2]
        output_array = np.ones_like(x)

        input_array = np.stack((x * x, y * y, z * z, 2 * x * y, 2 * x * z, 2 * y * z, 2 * x, 2 * y, 2 * z))
        if len(output_array)>24:
            print(input_array.shape)
            output_array[:24] *= np.sqrt(50)
            input_array[:, :24] *= np.sqrt(50)
        (coeff, accuracy, rank, values) = np.linalg.lstsq(input_array.T, output_array, rcond=None)
        a, b, c, d, e, f, g, h, i = coeff
        A4 = np.array([[a, d, e, g],
                       [d, b, f, h],
                       [e, f, c, i],
                       [g, h, i, -1]])
        A3 = A4[0:3, 0:3]
        vghi = np.array([-g, -h, -i])
        self.centre = np.linalg.lstsq(A3, vghi, rcond=None)[0]
        T = np.identity(4)
        T[3, 0:3] = self.centre
        B4 = T.dot(A4).dot(T.T)
        B3 = B4[0:3, 0:3] / -B4[3, 3]
        e, v = np.linalg.eig(B3)
        self.transform = v.dot(np.sqrt(np.diag(e))).dot(v.T)

    def uniformity(self, vector):
        return (np.linalg.norm(self.apply(vector), axis=1) - 1) ** 2

    def set_higher_order_params(self, params, order=ORDER):
        if ORTHOGONAL:
            if RBF_GAUSSIAN:
                self.second_order[0] = RBF(params[:order])
                self.second_order[1] = RBF(params[order:order * 2])
                self.second_order[2] = RBF(params[order * 2:])
            else:
                self.second_order[0] = Polynomial(params[:order])
                self.second_order[1] = Polynomial(params[order:order * 2])
                self.second_order[2] = Polynomial(params[order * 2:])
        else:
            count = 0
            for i in range(order):
                for j in range(order - i):
                    for k in range(order - i - j):
                        for dim in range(3):
                            self.second_order[dim][i, j, k] = params[-count]
                            count += 1

    def fit_higher_order(self, readings, order=ORDER):
        def fit_higher(params):
            self.set_higher_order_params(params, order)
            return self.uniformity(readings)

        x0 = self.make_params_list(order)
        print("minimizing nonlinear", np.mean(fit_higher(x0)))
        ret = scipy.optimize.least_squares(fit_higher, x0)
        print(ret.success, ret.message, ret.x)
        print("nonlinear final:", np.mean(fit_higher(ret.x)))

    def make_params_list(self, order=ORDER):
        if ORTHOGONAL:
            x0 = np.zeros(order * 3)
        else:
            x0 = np.zeros(3 * (order * (order + 1) * (order + 2)) // 6)
        return x0

    def apply(self, v):
        v = v - self.centre
        v = self.apply_non_linear(v)
        v = v.dot(self.transform)
        return v

    def apply_non_linear(self, vectors):
        v = copy.copy(vectors)
        scale = self.transform[0, 0]
        normalised_v = v * scale
        if ORTHOGONAL:
            if RBF_GAUSSIAN:
                v[:, 0] += self.second_order[0](normalised_v[:, 0])/scale
                v[:, 1] += self.second_order[1](normalised_v[:, 1])/scale
                v[:, 2] += self.second_order[2](normalised_v[:, 2])/scale
        else:
            v[:, 0] += np.polynomial.polynomial.polyval3d(vectors[:, 0],vectors[:, 1], vectors[:, 2], self.second_order[0])
            v[:, 1] += np.polynomial.polynomial.polyval3d(vectors[:, 0], vectors[:, 1], vectors[:, 2], self.second_order[0])
            v[:, 2] += np.polynomial.polynomial.polyval3d(vectors[:, 0], vectors[:, 1], vectors[:, 2], self.second_order[0])
        return v

    def apply_matrix(self, rot):
        self.transform = np.dot(rot, self.transform)

    def make_ellipsoid(self, x, y, z):
        U, s, rotation = np.linalg.svd(self.transform ** 2)
        radii = 1.0 / np.sqrt(s)
        x *= radii[0]
        y *= radii[1]
        z *= radii[2]

        for i in range(len(x)):
            for j in range(len(x)):
                [x[i, j], y[i, j], z[i, j]] = np.dot([x[i, j], y[i, j], z[i, j]], rotation) + self.centre
        return x, y, z

    def find_circle(self, data):
        data = self.apply(data)
        print("CIRCLING...")
        output = np.ones_like(data[:, 0])
        x = data[:, 0]
        z = data[:, 2]
        input_array = np.stack(((x * x + z * z), x, z))
        results = np.linalg.lstsq(input_array.T, output, rcond=None)
        (coeff, accuracy, rank, values) = results
        x_offset = coeff[1] / -2 * coeff[0]
        z_offset = coeff[2] / -2 * coeff[0]
        radius = np.sqrt(x_offset ** 2 + z_offset ** 2 + 1 / coeff[0])
        print(x_offset, z_offset, radius)
        print(np.mean(data[:, 1]))
        return np.array((-x_offset, np.mean(data[:, 1]), -z_offset))

    def find_plane(self, data):
        data = self.apply(data)
        output = np.ones_like(data[:, 0])
        result = np.linalg.lstsq(data, output, rcond=None)[0]
        result = normalise(result)
        print("find_plane: ")
        print(result)
        return result

    def align_to_vector(self, v):
        if v[1] < 0:
            v = v * -1
        z_magnitude = np.sqrt((v[0] * v[0] + v[1] * v[1]))
        z_cos = v[1] / z_magnitude
        z_sin = v[0] / z_magnitude
        rot_z = np.array([[z_cos, z_sin, 0], [-z_sin, z_cos, 0], [0, 0, 1]])
        x_magnitude = np.sqrt((v[2] * v[2] + v[1] * v[1]))
        x_cos = np.sqrt(1 - v[2] * v[2])
        # x_cos = v[1]/x_magnitude
        x_sin = v[2]
        rot_x = np.array([[1, 0, 0], [0, x_cos, -x_sin], [0, x_sin, x_cos]])
        rot = np.dot(rot_x, rot_z)
        self.apply_matrix(rot)

    def align_laser(self, data):
        v = self.find_plane(data)
        self.align_to_vector(v)

    def align_circle(self, data):
        v = self.find_circle(data)
        print(v)
        self.align_to_vector(v)

    def to_vector(self):
        return np.hstack((self.transform.flatten(), self.centre))

    @classmethod
    def from_vector(cls, vector):
        self = cls()
        self.transform = vector[0:9].reshape((3, 3))
        self.centre = vector[9:]
        return self


class CalSet:
    def __init__(self):
        self.mag_cal = Calibration()
        self.grav_cal = Calibration()

    def fit_ellipsoid(self, r: ReadingSet):
        all_readings = r.all()
        self.mag_cal.fit(all_readings.mag)
        self.grav_cal.fit(all_readings.grav)

    def align_laser(self, mag, grav):
        self.mag_cal.align_laser(mag)
        self.grav_cal.align_laser(grav)

    def align_circles(self, mag, grav):
        self.mag_cal.align_circle(mag)

    def check_alignment(self, mag, grav, display=False):
        mag = normalise(self.mag_cal.apply(mag))
        down = normalise(self.grav_cal.apply(grav))
        angles = np.sum(mag * down, axis=1).T
        if display:
            print("Angles", angles)
        return np.std(angles, ddof=1)

    def check_alignment2(self, r: ReadingSet):
        result = sum(np.linalg.norm(self.check_accuracy(x)) for x in r.aligned)/len(r.aligned)
        return result

    def check_accuracy(self, r: Readings, display=False):
        east, north, orientation = self.get_orientation(r.mag, r.grav)
        if display:
            compass = np.arctan2(east[:, 1], north[:, 1])
            inclination = np.arctan2(orientation[:, 2], np.linalg.norm(orientation[:, 0:2], axis=1))
            print("Compass: ", np.rad2deg(compass) % 360)
            print("  Clino: ", np.rad2deg(inclination))
            # show_points(orientation)
        return np.rad2deg(np.std(orientation[-8:,:], axis=0, ddof=1))

    def get_orientation(self, mag, grav):
        mag = normalise(self.mag_cal.apply(mag))
        up = normalise(self.grav_cal.apply(grav)) * -1
        east = normalise(np.cross(mag, up))
        north = normalise(np.cross(up, east))
        orientation = normalise(np.stack((east[:, 1], north[:, 1], up[:, 1])).T)
        return east, north, orientation

    def check_uniformity_multiple(self, r: ReadingSet):
        all_readings = r.all()
        return self.check_uniformity(all_readings.mag, all_readings.grav)

    def check_uniformity(self, mag, grav):
        m = np.sqrt(np.mean(self.mag_cal.uniformity(mag)))
        g = np.sqrt(np.mean(self.grav_cal.uniformity(grav)))
        return np.rad2deg(m + g)

    def check_dip(self, r: ReadingSet):
        all_readings = r.all()
        return self.check_alignment(all_readings.mag, all_readings.grav)

    def check_alignment_single(self, mag, grav, r: Readings):
        main_orientation = np.mean(self.get_orientation(r.mag, r.grav)[2], axis=0)
        test_orientation = self.get_orientation(mag, grav)[2]
        return np.rad2deg(np.linalg.norm(test_orientation-main_orientation))

    def sync_sensors(self, mag, grav, x=False):
        angles = np.linspace(-np.pi / 100, np.pi / 100, 800)
        if x:
            quats = np.exp(q.x * angles)
        else:
            quats = np.exp(q.y * angles)

        old_transform = self.mag_cal.transform
        best_q = None
        best_std = self.check_alignment(m, g)
        print("before: ", best_std)
        for quat in quats:

            self.mag_cal.transform = np.dot(old_transform, q.as_rotation_matrix(quat))
            std = self.check_alignment(mag, grav)
            if std < best_std:
                best_q = quat
                best_std = std
        print("after: ", best_std)
        print("angles: ", np.rad2deg(q.as_euler_angles(best_q)))
        self.mag_cal.transform = np.dot(old_transform, q.as_rotation_matrix(best_q))

    def minimize_heading_error(self, r: ReadingSet, order=ORDER):
        def min_func(x):
            c = copy.deepcopy(self)
            c.mag_cal.set_higher_order_params(x, order)
            return c.check_alignment2(r) + c.check_uniformity_multiple(r)

        x0 = self.mag_cal.make_params_list(order)
        ret = scipy.optimize.minimize(min_func, x0)
        c = copy.deepcopy(self)
        c.mag_cal.set_higher_order_params(ret.x, order)
        if not ret.success:
            print("Minimisation error: ", ret.message)
        return c


    def minimize_dip_error(self, r: ReadingSet, order=ORDER):
        gaussians =
        def min_func(x):
            c = copy.deepcopy(self)
            c.mag_cal.set_higher_order_params(x, order)
            return c.check_dip(r) + c.check_uniformity_multiple(r)

        x0 = self.mag_cal.make_params_list(order)
        ret = scipy.optimize.minimize(min_func, x0)
        c = copy.deepcopy(self)
        c.mag_cal.set_higher_order_params(ret.x, order)
        if not ret.success:
            print("Minimisation error: ", ret.message)
        return c

    def to_vector(self):
        return np.hstack((self.mag_cal.to_vector(), self.grav_cal.to_vector()))

    @classmethod
    def from_vector(cls, vector):
        self = cls()
        self.mag_cal = Calibration.from_vector(vector[:12])
        self.grav_cal = Calibration.from_vector(vector[12:])
        return self

    def rotate(self, rots):
        rot = [rots[0], 0.0, rots[1]]
        M = R.from_euler('xyz', rot, degrees=True)
        rot1 = [rots[2], 0.0, rots[3]]
        G = R.from_euler('xyz', rot1, degrees=True)
        self.mag_cal.apply_matrix(M.as_dcm())
        self.grav_cal.apply_matrix(G.as_dcm())

    def minimize(self, r: ReadingSet) -> "CalSet":
        def minimisation_function(rots):
            # rots is a set of 6 rotations, 3 for mag, 3 for grav
            c = copy.deepcopy(self)
            c.rotate(rots)
            return c.check_alignment2(r)

        x0 = np.zeros(4)
        ret = scipy.optimize.minimize(minimisation_function, x0, method="Nelder-Mead",
                                      options={"maxiter": 20000})
        c = copy.deepcopy(self)
        c.rotate(ret.x)
        return c

    @classmethod
    def full_calibration(cls, r: ReadingSet, order=ORDER):
        c = cls()
        c.fit_ellipsoid(r)
        c = c.minimize(r)
        if order > 0:
            if MIN_DIP:
                c = c.minimize_dip_error(r, order)
            else:
                c = c.minimize_heading_error(r, order)
        return c



def graph(x, *ys, labels=None):
    fig = plt.figure()
    ax = fig.add_subplot(111)
    if labels is None:
        labels = list(range(len(ys)))
    for label,y in zip(labels, ys):
        ax.plot(x, y, label=label)
    ax.legend()
    plt.show()




