import numpy as np

class RBF:
    def __init__(self, params):
        self.params = np.array(params).reshape((-1,1))
        if len(params)==1:
            self.offsets = [0.0]
            self.epsilon = 0.5
        else:
            self.offsets = np.linspace(-1,1,len(params)).reshape((-1,1))
            self.epsilon = 1.5/len(params)

    def __call__(self, x, gaussians=None):
        if gaussians is None:
            gaussians = self.get_gaussians(x)
        result = gaussians.T @ self.params
        return result[:,0]

    def __str__(self):
        return str(self.params)

    def __repr__(self):
        return str(self)

    def get_gaussians(self, x):
        x = x.reshape((1,-1))
        distances = x - self.offsets
        gaussians = np.exp(-(distances / self.epsilon) ** 2)
        return gaussians

    def linspace(self, n, size=(-1.0, 1.0)):
        space = np.linspace(size[0], size[1], n)
        return space, self(space)