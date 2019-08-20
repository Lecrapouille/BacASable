module MaxPlus

using LinearAlgebra, SparseArrays

import Base.+, Base.*, Base.show, Base.convert, Base.zero, Base.one

export MP, mpzeros, mpones

# Max-Plus immutable structure
struct MP{T} <: Number v::T end

Base.show(io::IO,k::MP)   = show(io, k.v)

Base.:+(a::MP,   b::MP)   = MP(max(a.v, b.v))
Base.:+(a::MP,   b::Real) = MP(max(a.v, b))
Base.:+(a::Real, b::MP)   = MP(max(a,   b.v))

Base.:*(a::MP,   b::MP)   = MP(a.v + b.v)
Base.:*(a::MP,   b::Real) = MP(a.v + b)
Base.:*(a::Real, b::MP)   = MP(a   + b.v)

Base.:/(a::MP,   b::MP)   = MP(a.v - b.v)
Base.:/(a::MP,   b::Real) = MP(a.v - b)
Base.:/(a::Real, b::MP)   = MP(a   - b.v)

Base.convert(::MP{T}, x) where T = MP(T(x))
Base.zero(::MP{T})       where T = MP(typemin(T))
Base.zero(::Type{MP{T}}) where T = MP(typemin(T))
Base.one(::MP{T})        where T = MP(zero(T))
Base.one(::Type{MP{T}})  where T = MP(zero(T))

mp0=zero(MP{Float64})
mp1=one(MP{Float64})
mpInf=MP(Inf)

#mpeye(::Type{T}, n::Int64)             where T = Matrix{MP{T}}(1.0I, n, n)
#mpeye(::Type{T}, n::Int64, m::Int64)   where T = map(MP,Matrix(1.0I, n, m))
mpzeros(::Type{T}, n::Int64)           where T = zeros(MP{T}, n, n)
mpzeros(::Type{T}, n::Int64, m::Int64) where T = zeros(MP{T}, n, m)
mpones(::Type{T}, n::Int64)            where T = ones(MP{T}, n, n)
mpones(::Type{T}, n::Int64, m::Int64)  where T = ones(MP{T}, n, m)
mparray(A::Array)           = map(MP,A)
mpsparse(A::Array)          = map(MP,sparse(A))

end # MaxPlus module
