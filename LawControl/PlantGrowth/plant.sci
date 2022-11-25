// A graphic display of a 3d tree for ScicosLab
// made by Quentin Quadrat

function []=forest(nbTrees,age)

  // Create a Maple leaf.
  // Create vertices of the leaf. Apply to them a scaling factor, a rotation and translation.
  // X: Rotation axis
  // M: Rotation matrix
  // e: Scaling coef
  function [L]=Leaf(X,M,e)
    L=[e*[leaf1;leaf2;leaf3;leaf4;leaf5;leaf6;leaf7;leaf8;leaf9;leaf10;leaf11;leaf12;leaf13],..
         [green;green;green;green;green;green;green;green;green;green;green;yellow;yellow]];
    L=Translation(Rotation(L,M),X)
  endfunction

  // Create a Maple trunk.
  // Create vertices of the trunk. Apply to them a scaling factor, a rotation and translation.
  // X: Rotation axis
  // M: Rotation matrix
  // e: Scaling coef
  function [T]=Trunk(X,M,e)
    T=[e/0.6*[trunk1;trunk2;trunk3;trunk4;trunk5;trunk6;trunk7;trunk8],..
             [brown;brown;brown;brown;brown;brown;brown;brown]]
    T=Translation(Rotation(T,M),X)
  endfunction

  // Create a Maple bud.
  // Create vertices of the bud. Apply to them a scaling factor, a rotation and translation.
  // X: Rotation axis
  // M: Rotation matrix
  // e: Scaling coef
  function [B]=Bud(X,M,e)
    B=[e*[bud1;bud2;bud3;bud4;bud5;bud6;bud7;bud8;bud9],..
         [pink;pink;pink;pink;pink;pink;pink;pink;pink]];
    B=Translation(Rotation(B,M),X)
  endfunction

  // Create a Maple tree.
  // age: the age of the tree
  // X: Rotation axis
  // M: Rotation matrix
  // e: Scaling coef
  function [T]=Tree(age,X,M,e)
    T=Bud(Ze,Id,3*e);
    for i=1:age,

      // Random the number of branch.
      nbBranchs = ceil(MaxBranchs*rand());
      if nbBranchs == 1 then
        if (i < 3*age/4) then
          nbBranchs = nbBranchs + 1;
        end;
      end;

      Leafs=[];
      if (i < age/4) then
        Leafs=[Leaf([0;0;e;0],Id,2*e);..
               Leaf([0;0;2*e;0],Ma,2*e);..
               Leaf([0;0;3*e;0],Maa,2*e);..
               Leaf([0;0;4*e;0],Mb*Mb,2*e);..
               Leaf([0;0;5*e;0],Maa*Mb,2*e)];
      end;

      B=[Leafs;Trunk(Ze,Id,5*e)];
      if nbBranchs == 3 then
         T=[Leafs;..
            Trunk(Ze,Id,5*e);..
            Simil(trans,Ma,.9,T);..
            Simil(trans,Mb,.9,T);..
            Simil(trans,Maa*Mb,.7,T)];
      elseif nbBranchs == 2 then
         T=[Leafs;..
            Trunk(Ze,Id,5*e);..
            Simil(trans,Ma,.9,T);..
            Simil(trans,Maa*Mb,.7,T)];
      else
         T=[Trunk(Ze,Id,5*e);Simil(trans,Ma,.7,T)];
         break; // Halt the recursion
      end;
    end;
    T=Translation(Rotation(T,M),X);
  endfunction

  // Apply the combined translation/rotation/scaling transformations on the object Imn.
  function [Imo]=Simil(T,M,s,Imn)
    Mp=M'
    [np,nd]=size(Imn)
    Tp=ones(np,1)*T'
    S=s*eye(4,4)
    S(4,4)=1
    Imo=Imn*S*Mp+Tp
  endfunction

  // Apply the Rotation matrix M on the object Imn.
  function [Imo]=Rotation(Imn,M)
    Imo=Imn*M'
  endfunction

  // Apply the translation T on the object Imn.
  function [Imo]=Translation(Imn,T)
    [np,nd]=size(Imn)
    Tp=ones(np,1)*T'
    Imo=Imn+Tp
  endfunction

  // Draw the 3D model
  function []=draw(Model)
    [np,nd]=size(Model);
    nf=np/4;
    X=matrix(Model(:,1),4,nf),
    Y=matrix(Model(:,2),4,nf),
    Z=matrix(Model(:,3),4,nf),
    plot3d1(X,Y,Z);
  endfunction

  // Convert degrees to radians
  function [r]=radians(d)
    r=0.01745*d;
  endfunction

  //
  MaxBranchs=3;
  a=radians(360/MaxBranchs);   // Rotation around the "trunk" axis
  b=-radians(30);              // Rotation around the branches

  // Identity and zero matrix
  Id=eye(4,4);
  Ze=zeros(4,1);

  // Rotation matrix (first axis).
  Ma=[cos(a),-sin(a),0,0; sin(a), cos(a),0,0; 0,0,1,0;0,0,0,1];
  Maa=Ma*Ma;
  // Rotation matrix (second axis).
  Mb=[1,0,0,0;0,cos(b),-sin(b),0;0,sin(b),cos(b),0;0,0,0,1];
  Mba=Mb*Ma;

  // Define some colors
  pink=6*ones(4,1);
  yellow=7*ones(4,1);
  green=13*ones(4,1);
  brown=19*ones(4,1);

  // Meshes for drawing in 3D a Maple bud.
  bud1=0.05*[1,1,0;-1,1,0;-1,-1,0;1,-1,0];
  bud2=0.05*[-1,1,0;1,1,0;2,2,2;-2,2,2];
  bud3=0.05*[1,-1,0;2,-2,2;2,2,2;1,1,0];
  bud4=0.05*[-2,-2,2;2,-2,2;1,-1,0;-1,-1,0];
  bud5=0.05*[-2,2,2;-2,-2,2;-1,-1,0;-1,1,0];
  bud6=0.05*[2,2,2;0,0,15;0,0,15;2,-2,2];
  bud7=0.05*[2,-2,2;-2,-2,2;0,0,15;0,0,15];
  bud8=0.05*[-2,2,2;0,0,15;0,0,15;-2,-2,2];
  bud9=0.05*[-2,2,2;0,0,15;0,0,15;2,2,2];

  // Meshes for drawing in 3D a Maple bud.
  nb=0.2;
  trunk1=0.03*[1,0,0;1-nb,0-nb,20;sqrt(2)/2-nb,sqrt(2)/2-nb,20;sqrt(2)/2,sqrt(2)/2,0];
  trunk2=0.03*[sqrt(2)/2,sqrt(2)/2,0;sqrt(2)/2-nb,sqrt(2)/2-nb,20;0-nb,1-nb,20;0,1,0];
  trunk3=0.03*[-sqrt(2)/2,sqrt(2)/2,0;0,1,0;0-nb,1-nb,20;-sqrt(2)/2+nb,sqrt(2)/2-nb,20];
  trunk4=0.03*[-1,0,0;-sqrt(2)/2,sqrt(2)/2,0;-sqrt(2)/2+nb,sqrt(2)/2-nb,20;-1+nb,0-nb,20];
  trunk5=0.03*[0,-1,0;-sqrt(2)/2,-sqrt(2)/2,0;-sqrt(2)/2+nb,-sqrt(2)/2+nb,20;0-nb,-1+nb,20];
  trunk6=0.03*[-sqrt(2)/2,-sqrt(2)/2,0;-1,0,0;-1+nb,0-nb,20;-sqrt(2)/2+nb,-sqrt(2)/2+nb,20];
  trunk7=0.03*[1,0,0;sqrt(2)/2,-sqrt(2)/2,0;sqrt(2)/2-nb,-sqrt(2)/2+nb,20;1-nb,0-nb,20];
  trunk8=0.03*[sqrt(2)/2,-sqrt(2)/2,0;0,-1,0;0-nb,-1+nb,20;sqrt(2)/2-nb,-sqrt(2)/2+nb,20];

  // Meshes for drawing in 3D a Maple leaf.
  leaf9=[0.5,0.5,0;1.43,0.93,0;1.1,0.34,0;0.7,0.05,0];
  leaf11=[0.7,0.05,0;1.1,0.34,0;1.2,0.1,0;0.95,0,0];
  leaf10=[1.1,0.34,0;1.43,0.93,0;1.7,0.75,0;1.5,0.35,0];
  leaf8=[1.29,1.05,0;1.84,1.6,0;1.71,1,0;1.45,0.95,0];
  leaf7=[1.3,1.05,0;0.85,1.15,0;1.06,1.4,0;1.85,1.6,0];
  leaf6=[1.06,1.4,0;1.1,1.57,0;1.35,1.75,0;1.85,1.6,0];
  leaf5=[0.5,0.5,0;0.85,1.15,0;1.3,1.05,0;1.43,0.93,0];
  leaf4=[0.15,0.65,0;0.37,1.2,0;0.85,1.15,0;0.5,0.5,0];
  leaf3=[0.37,1.2,0;1,1.85,0;1.05,1.4,0;0.85,1.15,0];
  leaf2=[0.37,1.2,0;0.4,1.6,0;0.65,1.8,0;1,1.85,0];
  leaf1=[0.15,0.65,0;0,1,0;0.1,1.35,0;0.37,1.2,0];
  leaf12=[-0.02,0.02,0.01;1,1,0.01;1.02,0.98,0.01;0.02,-0.02,0.01];
  leaf13=[1,1,0.01;1.81,1.62,0.01;1.83,1.57,0.01;1.02,0.98,0.01];

  // translation
  e=0.3;
  trans=[0;0;5*e;0];

  // Random positions of trees.
  posxy=15*rand(nbTrees,2);
  zer2=zeros(nbTrees,2);
  pos=[posxy,zer2];

  // Create a forest.
  T=zeros(0,4);
  for i=1:nbTrees
    T=[T;Tree(age,pos(i,:)',Id,e)];
  end;

  // Draw the forest.
  draw(T);
endfunction

stacksize(200000000);
forest(2,10*rand());
