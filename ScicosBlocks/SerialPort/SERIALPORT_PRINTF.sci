function [x,y,typ]=SERIALPORT_PRINTF(job,arg1,arg2)
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
    [ok,thestring,exprs]=getvalue(..
        ['printf'],..
        ['String:'],..
         list('str',1),exprs)
    if ~ok then break,end //user cancel modification
    graphics.exprs=exprs;
    if ok then
        model.ipar=[length(thestring);str2code(thestring)]
        graphics.exprs=exprs;
        x.graphics=graphics;
        x.model=model;
        break
    end
  end
case 'define' then
  thestring='hello world'

  model=scicos_model()
  model.sim=list('serialport_scicos_printf',4)
  model.evtin=1;
  model.ipar=[length(thestring);str2code(thestring)]
  model.blocktype='d'
  model.dep_ut=[%f %f]
  exprs=[string(thestring)]
  gr_i=['txt=[''printf''];';
    'xstringb(orig(1),orig(2),txt,sz(1),sz(2),''fill'')']
  x=standard_define([4 2],model,exprs,gr_i)
end
endfunction
