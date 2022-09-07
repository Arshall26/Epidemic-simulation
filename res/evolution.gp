set title "Resultat simulation"
set xlabel "nombre de tours"
set ylabel "nombre de personnes"
set xrange [0:99]
set yrange [0:37]
set border
set grid
plot for [i=2:5] "evolution.txt" using 1:i smooth csplines