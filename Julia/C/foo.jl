struct Mystruct
    data::Ptr{Cdouble}
    size::Csize_t
    Mystruct() = new(Ptr{Cdouble}(), 0)
end

function print_struct(v::Ref{Mystruct})
    ccall((:recvstruct, "./libfoo.so"), Cvoid, (Ptr{Mystruct},), v)
end

function update_struct()
    st = Ref(Mystruct())

    # Use Ptr but not Ref since the C is managing its memory and
    # we do not want Julia manages the memory (garbage collecting
    # and producing a segfault)
    ccall((:updatestruct, "./libfoo.so"), Cvoid, (Ptr{Mystruct},), st)

    # Display the C array
    print_struct(st)

    # Convert the C array into a Julia vector
    V = Vector{Cdouble}(undef, st[].size)
    for i in 1:st[].size
        V[i] = unsafe_load(st[].data, i)
    end

    return V
end

V = update_struct()
show(V)
