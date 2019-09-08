using MaxPlus

function f(n::Int64)
  A = rand(n,n);
  @time y = A * A;
end

function g(n::Int64)
  A = mparray(rand(n,n));
  @time y = A * A;
end

# Tested on Scilab 4.4.1 and Julia 0.4 and 1.0.3 downloaded "as it"
# (meaning I did not compile sources). Therefore I guess there is
# no Cuda and no OpenBlas activated.

# Debian 10 amd64. AMD Ryzen 1800x.
         # Julia 1.0.3         # Scicoslab    # Julia 0.4
f(400);  # 0.0018 seconds      0.033596       0.005923 seconds
f(800);  # 0.012382 seconds    0.214737       0.016760 seconds
f(1600); # 0.052756 seconds    2.218538       0.079267 seconds

g(400);  # 0.088829 seconds    0.177505       0.250052 seconds
g(800);  # 0.674794 seconds    1.4063         1.930686 seconds
g(1600); # 5.288377 seconds    14.655486      15.462906 seconds

using Plots
x=[4; 8; 16];
ju04=[0.005923; 0.016760; 0.079267];
ju103=[0.0018; 0.012; 0.05];
sci=[0.033596; 0.214737; 2.218538];
plot(x, [ju04, ju103, sci], title="standard + dense", label=["julia 0.4", "Julia 1.0.3", "ScicosLab"])

mpju04=[0.250052; 1.930686; 15.462906];
mpju103=[0.088829; 0.674794; 5.288377];
mpsci=[0.177505; 1.4063; 14.655486];
plot(x, [mpju04, mpju103, mpsci], title="maxplus + dense", label=["julia 0.4", "Julia 1.0.3", "ScicosLab"])

plot(x, [ju103, mpju103], title="julia 1.0.3 maxplus + dense", label=["normal", "maxplus"])
