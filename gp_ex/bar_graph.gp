# bar graph 

set boxwidth 0.5
set style fill solid
plot "bar_graph.dat" using 1:3:xtic(2) with boxes
