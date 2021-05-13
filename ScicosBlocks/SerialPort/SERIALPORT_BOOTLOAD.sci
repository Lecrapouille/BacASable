function [x,y,typ]=SERIALPORT_BOOTLOAD(job,arg1,arg2)
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
    [ok,portname,exprs]=getvalue(..
        ['Bootloader (Master TFTP)'],..
        ['Serial Port ID'],..
         list('vec',1),exprs)
    if ~ok then break,end //user cancel modification
    graphics.exprs=exprs;
    if ok then
        model.ipar=[portname]
        graphics.exprs=exprs;
        x.graphics=graphics;
        model.out=1;
        x.model=model;
        break
    end
  end
case 'define' then
  portname=0

  model=scicos_model()
  model.sim=list('serialport_scicos_bootloader',4)
  model.evtin=1;
  model.out=1;
  model.ipar=[portname]
  model.blocktype='d'
  model.dep_ut=[%f %f]
  exprs=[string(portname)]
  gr_i=['txt=[''Bootloader'';''SERIAL PORT''];';
    'xstringb(orig(1),orig(2),txt,sz(1),sz(2),''fill'')']
  x=standard_define([4 2],model,exprs,gr_i)
end
endfunction
