function [x,y,typ]=SERIALPORT_READ(job,arg1,arg2)
x=[];y=[];typ=[]
select job
case 'plot' then
  standard_draw(arg1)
case 'getinputs' then
  [x,y,typ]=standard_inputs(o)
case 'getoutputs' then
  [x,y,typ]=standard_outputs(o)
case 'getorigin' then
  [x,y]=standard_origin(arg1)
case 'set' then
  x=arg1;
  graphics=arg1.graphics;
  exprs=graphics.exprs;
  model=arg1.model;
  while %t do
    [ok,buffer,fd,exprs]=getvalue(..
        ['Read in Serial Port';
         '';
         'Taille buffer: Lire plusieurs caracteres par appel (buffer < 64 char)';
         'File descriptor'],..
        ['Taille buffer';
         'FD'],..
         list('vec',1,'vec',1),exprs)
    if ~ok then break,end //user cancel modification
    graphics.exprs=exprs;
    if ok then
        model.ipar=[buffer;fd]
        graphics.exprs=exprs;
        x.graphics=graphics;
        model.out=[buffer;1];
        x.model=model;
        break
    end
  end
case 'define' then
  buffer=2
  fd=0;

  model=scicos_model()
  model.sim=list('serialport_scicos_read',4)
  model.evtin = 1;
  model.out = [buffer;1];
  model.ipar=[buffer;fd]
  model.blocktype='d'
  model.dep_ut=[%f %f]

  exprs=[string(buffer);
         string(fd)]
  gr_i=['txt=[''Read in'';''SERIAL PORT''];';
    'xstringb(orig(1),orig(2),txt,sz(1),sz(2),''fill'')']
  x=standard_define([4 2],model,exprs,gr_i)
end
endfunction
