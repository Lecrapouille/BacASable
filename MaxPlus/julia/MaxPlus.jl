#-------------Max-Plus------------------------#

import Base.show,Base.zero,Base.one,Base.promote_rule,Base.+,Base.*,Base.convert
immutable MP  v::Float64 end
# promote_rule(MP,Real)=MP
promote_rule(::Type{MP},::Type{Int64})=MP
promote_rule(::Type{MP},::Type{Float64})=MP
*(a::MP,b::MP)= MP(a.v+b.v)
*(a::MP,b::Real)=MP(a.v+b)
*(a::Real,b::MP)=MP(a+b.v)
+(a::MP,b::MP)= MP(max(a.v,b.v))
+(a::MP,b::Real)=MP(max(a.v,b))
+(a::Real,b::MP)=MP(max(a,b.v))
show(io::IO,k::MP)=print(io,k.v)
zero(::Type{MP})=MP(typemin(Float64))
zero(::MP)=MP(typemin(Float64))
one(::Type{MP})= MP(zero(Float64))
convert(::Type{MP},x::Float64)=MP(x)
convert(::Type{MP},x::Int64)=MP(Float64(x))
convert(::Type{Float64},x::MP)=x.v
mparray(A::Array)=map(MP,A)
array(A::Array{MP})=map(x->x.v, A)
mpsparse(S::SparseMatrixCSC)=SparseMatrixCSC(S.m,S.n,S.colptr,S.rowval,mparray(S.nzval))
mpsparse(M::Array{Float64})=mpsparse(sparse(M))
mpsparse(M::Array{MP})=(k=0; R::Array{Int64,1}=[]; C::Array{Int64,1}=[]; V::Array{MP,1}=[];
    for i=1:size(M,1) for j=1:size(M,2)
     M[i,j]!=zero(MP) && (R=[R;i]; C=[C;j]; V=[V;M[i,j]])
    end end ;
    sparse(R,C,V) )

#------------Test-Max-Pus----- -----------------#

promote_type(MP,Float64)
promote_type(MP,Int64)
one(MP)
zero(MP)
ones(MP,2,2)
zeros(MP,3,4)
spzeros(MP,2,2)
speye(MP,2,3)
eye(MP,3,4)
MP(2.0)+MP(3)
MP(2.0)*MP(3.0)
MP(2.0)+3.0
MP(4.0)+2
[MP(2) 2.0]
[MP(2) 2]
MP[2 3 5]
MP[2.0 3.0]
MP[2 3;4 5]
MP[MP(4) 2]
a=[MP(2.0) 3.0;  1 2.0 ]
a+a
a*a
b=rand(2,2)
c=mparray(b)
a*b
d=array(c)
n=400
a = rand(n,n);  gc(); @time  a*a
a = mparray(rand(n,n));  gc(); @time a*a
c=rand(n,n); gc(); @time a*c
a=sparse(rand(n,n)); gc(); @time a*a
a=mpsparse(rand(n,n)); gc(); @time a*a

#----------------Ring-Operator------------------#

function ringmatmul(+, *, A::Matrix, B::Matrix)
    m, n, p = size(A,1), size(B,2), size(A,2)
    C = [A[i,1]*B[1,j] for i=1:m, j=1:n]
    for i=1:m, j=1:n, k=2:p
        C[i,j] += A[i,k]*B[k,j]
    end
    return C
end
*((+)::Function, (*)::Function) = (A,B)->ringmatmul(+,*,A,B)

#----------------Test-Ring-Operator--------------#

a = rand(n,n); b = rand(n,n); gc(); @time  *(max,+)(a,b)
