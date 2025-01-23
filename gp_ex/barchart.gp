# bar chart colors 

set style fill solid
set yrange[0:]
set xrange[1990:1997]
GOOD_ROW(x,y) = (x eq "G") ? y:1/0
BAD_ROW(x,y) = (x eq "B") ? y:1/0

set boxwidth 1

plot 'data.dat' u ($0):(GOOD_ROW(stringcolumn(3),$2)):xtic(1) w boxes lc rgb "green", \
             '' u ($0):(BAD_ROW(stringcolumn(3),$2)):xtic(1) w boxes lc rgb "red"
