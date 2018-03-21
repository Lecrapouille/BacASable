import Base: +
import Base: *
import Base: -
import Base: /

type MaxPlus
    a::Float64
end

function +(x::MaxPlus, y::MaxPlus)
  return max(x.a, y.a)
end

function *(x::MaxPlus, y::MaxPlus)
  return x.a + y.a
end

function -(x::MaxPlus, y::MaxPlus)
  return x.a - y.a
end

function /(x::MaxPlus, y::MaxPlus)
  return x.a - y.a
end

x = MaxPlus(3)
y = MaxPlus(5)

x + y
x * y
x + x
x * x

A=[MaxPlus(4) MaxPlus(3); MaxPlus(7) MaxPlus(-Inf)]
A=[MaxPlus(4) MaxPlus(3); MaxPlus(7) MaxPlus(-4)]
A+A
A*A
