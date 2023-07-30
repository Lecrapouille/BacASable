# ScicosLab scrip plotting data
# http://www.scicoslab.org/

M=fscanfMat('res.csv');
T=M(:,1);
Raw=M(:,2);
Filt=M(:,3);
plot2d(T, Raw, style=1);
plot2d(T, Filt, style=2);
