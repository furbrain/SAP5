import JSON
using LinearAlgebra: I, Diagonal, eigen


function fit_ellipsoid(readings)
    x = readings[:, 1]
    y = readings[:, 2]
    z = readings[:, 3]
    x² = x .* x
    y² = y .* y
    z² = z .* z
    xy = x .* y
    yz = y .* z
    xz = x .* z
    D = hcat(x², y², z², 2xy, 2xz, 2yz, 2x, 2y, 2z)
    coeffs = D \ ones(size(x))
    a, b, c, d, e, f, g, h, i = coeffs
    A₄ = [a d e g
          d b f h
          e f c i
          g h i -1]
    A₃ = A₄[1:3,1:3]
    vghi = [-g; -h; -i]
    centre = A₃ \ vghi
    T = Matrix(1.0I, 4, 4)
    T[4,1:3] = centre
    display(T)
    B₄ = T * A₄ * T'
    B₃ = B₄[1:3,1:3] / -B₄[4,4]
    eg = eigen(B₃)
    display(Diagonal(eg.values))
    transform = eg.vectors * (√Diagonal(eg.values)) * transpose(eg.vectors)
    display(transform)
end

function read_data(fname::String)
    open(fname, "r") do f
        dct = JSON.parse(f)
    filter_sensor(x) = Array{Float64}(transpose(reshape((filter(!isnan, x)), 3, :)))
    mag =  filter_sensor(dct["shots"]["mag"])
    grav = filter_sensor(dct["shots"]["grav"])
    return (mag, grav)
    end
end

data = read_data("../sample data/ab.cal")
fit_ellipsoid(data[1])
fit_ellipsoid(data[2])
