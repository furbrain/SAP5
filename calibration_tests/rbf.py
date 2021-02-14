import numpy as np

class RBF:
    def __init__(self, params):
        self.params = np.array(params).reshape((-1,1))
        self.offsets = np.linspace(-1,1,len(params)).reshape((-1,1))
        self.epsilon = 2/len(params)

    def __call__(self, x):
        gaussians = self.get_gaussians(x)
        result = gaussians.T @ self.params
        return result[:,0]

    def get_gaussians(self, x):
        x = x.reshape((1,-1))
        distances = x - self.offsets
        gaussians = np.exp(-(distances / self.epsilon) ** 2)
        return gaussians

    def linspace(self, n, size=(-1.0,1.0)):
        space = np.linspace(size[0],size[1],n)
        return space, self(space)