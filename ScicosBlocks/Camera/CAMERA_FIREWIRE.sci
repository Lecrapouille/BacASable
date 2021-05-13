function [x,y,typ]=CAMERA_FIREWIRE(job,arg1,arg2)
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
    [ok,height,width,exprs]=getvalue(..
	['Cam With Her';
	 '';
	 'dddddd'],..
	['Height';
	'Width'],..
	 list('vec',1,'vec',1),exprs)
    if ~ok then break,end //user cancel modification
    graphics.exprs=exprs;
    if ok then
	model.ipar=[height,width]
	graphics.exprs=exprs;
	x.graphics=graphics;
	model.out = 3 * width * height;
	x.model=model;
	break
    end
  end
case 'define' then
  height=240
  width=320

  model=scicos_model()
  model.sim=list('scicos_camerafirewire',4)
  model.evtin = 1;
  model.out = 3 * width * height;
  model.ipar=[height,width]
  model.blocktype='d'
  model.dep_ut=[%t %f]

  exprs=[string(height); string(width)]
  gr_i=['txt=[''Scicos input'';''FIREWIRE''];';
    'xstringb(orig(1),orig(2),txt,sz(1),sz(2),''fill'')']
  x=standard_define([4 2],model,exprs,gr_i)
end
endfunction
