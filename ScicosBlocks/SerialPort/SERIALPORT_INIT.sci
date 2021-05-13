function [x,y,typ]=SERIALPORT_INIT(job,arg1,arg2)
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
    [ok,portname,speed,databits,parity,stopbits,flowcontrol,fd,exprs]=getvalue(..
        ['Init serial port';
         '';
         'Bauds: 0, 50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400, 4800, 9600, 19200, 38400, 57600, 115200';
         'Nombres bits donnees: 5, 6, 7, 8';
         'Parite: paire = 2, impaire = 1, sans = 0';
         'Nombre bits stops: 1 ou 2';
         'Flot de controle: software = 2, hardware = 1, sans = 0'],..
        ['Nom du port';
        'Vitesse (baud)';
         'Nombres bits de donees';
         'Parite';
         'Nombres bits de stop';
         'Flot de controle';
         'FD'],..
         list('str',1,'vec',1,'vec',1,'vec',1,'vec',1,'vec',1,'vec',1),exprs)
    if ~ok then break,end //user cancel modification
    graphics.exprs=exprs;
    if ok then
        model.ipar=[speed;databits;parity;stopbits;flowcontrol;fd;length(portname);str2code(portname)]
        graphics.exprs=exprs;
        x.graphics=graphics;
        x.model=model;
        break
    end
  end
case 'define' then
  portname='/dev/tty.usbserial-FTCEW0V0'
  speed=19200
  databits=8
  parity=0
  stopbits=1
  flowcontrol=0
  fd=0;

  model=scicos_model()
  model.sim=list('serialport_scicos_init',4)
  model.ipar=[speed;databits;parity;stopbits;flowcontrol;fd;length(portname);str2code(portname)]
  model.blocktype='d'
  model.dep_ut=[%f %f]
  model.out=1

  exprs=[portname;
         string(speed);
         string(databits);
         string(parity);
         string(stopbits);
         string(flowcontrol);
         string(fd)]
  gr_i=['txt=[''Init'';''SERIAL PORT''];';
    'xstringb(orig(1),orig(2),txt,sz(1),sz(2),''fill'')']
  x=standard_define([4 2],model,exprs,gr_i)
end
endfunction
