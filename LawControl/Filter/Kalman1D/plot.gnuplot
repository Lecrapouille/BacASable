filename='res.csv'

set datafile separator ";"
set key autotitle columnhead # use the first line as title

set style line 101 lw 3 lt rgb "#f62aa0" # style for targetValue (1) (pink)
set style line 102 lw 3 lt rgb "#26dfd0" # style for measuredValue (2) (light blue)
set style line 103 lw 4 lt rgb "#b8ee30" # style for secondYAxisValue (3) (limegreen)

plot filename using 1:2 with lines ls 102, '' using 1:3 with lines ls 101

pause mouse close
