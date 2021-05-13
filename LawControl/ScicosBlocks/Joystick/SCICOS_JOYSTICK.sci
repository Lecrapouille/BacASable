function [x,y,typ]=SCICOS_JOYSTICK(job,arg1,arg2)
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
    [ok,DeviceName,Verbose,exprs]=getvalue(..
	['Le Baton de la Joy';
	 ''],..
	['Device Name';
	'Verbose'],..
	 list('str',1,'vec',1),exprs)
    if ~ok then break,end //user cancel modification
    graphics.exprs=exprs;
    if ok then
	model.ipar=[str2code(DeviceName);Verbose]
	graphics.exprs=exprs;
	x.graphics=graphics;
	model.out = [3;3]; // max 4 axis, max 16 buttons, 2 infos
	x.model=model;
	break
    end
  end
case 'define' then
  DeviceName='/dev/input/joy0'
  Verbose=1

  model=scicos_model()
  model.sim=list('scicos_joystick',4)
  model.evtin = 1;
  model.out = [3;3];
  model.ipar=[str2code(DeviceName);Verbose]
  model.blocktype='d'
  model.dep_ut=[%t %f]

  exprs=[DeviceName; string(Verbose)]
  gr_i=['txt=[''Scicos input'';''JOYSTICK''];';
    'xstringb(orig(1),orig(2),txt,sz(1),sz(2),''fill'')']
  x=standard_define([4 2],model,exprs,gr_i)
end
endfunction
