<!DOCTYPE html><html lang="en"><head><meta charset="utf-8"><meta http-equiv="X-UA-Compatible" content="IE=edge"><meta name="viewport" content="width=device-width, initial-scale=1"><title>OK - Wyniki pracy generatora</title></head><body><script type="text/javascript" src="https://www.gstatic.com/charts/loader.js"></script><script type="text/javascript">google.charts.load("current", {packages:["timeline"]});google.charts.setOnLoadCallback(drawChart);function drawChart() {var container = document.getElementById('example4.2');var chart = new google.visualization.Timeline(container);var dataTable = new google.visualization.DataTable();dataTable.addColumn({ type: 'string', id: 'Role' });dataTable.addColumn({ type: 'string', id: 'Name' });dataTable.addColumn({ type: 'date', id: 'Start' });dataTable.addColumn({ type: 'date', id: 'End' });dataTable.addRows([[ 'M1', 'Zadanie 1', new Date(0,0,0,0,0,56), new Date(0,0,0,0,1,15) ],
[ 'M2', 'Zadanie 1', new Date(0,0,0,0,3,45), new Date(0,0,0,0,4,2) ],
[ 'M2', 'Zadanie 2', new Date(0,0,0,0,2,34), new Date(0,0,0,0,2,50) ],
[ 'M1', 'Zadanie 2', new Date(0,0,0,0,3,34), new Date(0,0,0,0,3,41) ],
[ 'M1', 'Zadanie 3', new Date(0,0,0,0,0,25), new Date(0,0,0,0,0,39) ],
[ 'M2', 'Zadanie 3', new Date(0,0,0,0,2,57), new Date(0,0,0,0,3,15) ],
[ 'M2', 'Zadanie 4', new Date(0,0,0,0,2,50), new Date(0,0,0,0,2,57) ],
[ 'M1', 'Zadanie 4', new Date(0,0,0,0,2,24), new Date(0,0,0,0,2,42) ],
[ 'M2', 'Zadanie 5', new Date(0,0,0,0,4,20), new Date(0,0,0,0,4,33) ],
[ 'M1', 'Zadanie 5', new Date(0,0,0,0,4,33), new Date(0,0,0,0,4,49) ],
[ 'M1', 'Zadanie 6', new Date(0,0,0,0,0,39), new Date(0,0,0,0,0,46) ],
[ 'M2', 'Zadanie 6', new Date(0,0,0,0,4,8), new Date(0,0,0,0,4,20) ],
[ 'M2', 'Zadanie 7', new Date(0,0,0,0,3,40), new Date(0,0,0,0,3,45) ],
[ 'M1', 'Zadanie 7', new Date(0,0,0,0,4,1), new Date(0,0,0,0,4,16) ],
[ 'M2', 'Zadanie 8', new Date(0,0,0,0,3,15), new Date(0,0,0,0,3,31) ],
[ 'M1', 'Zadanie 8', new Date(0,0,0,0,3,41), new Date(0,0,0,0,3,51) ],
[ 'M2', 'Zadanie 9', new Date(0,0,0,0,2,6), new Date(0,0,0,0,2,25) ],
[ 'M1', 'Zadanie 9', new Date(0,0,0,0,3,25), new Date(0,0,0,0,3,34) ],
[ 'M2', 'Zadanie 10', new Date(0,0,0,0,4,2), new Date(0,0,0,0,4,8) ],
[ 'M1', 'Zadanie 10', new Date(0,0,0,0,4,16), new Date(0,0,0,0,4,27) ],
[ 'M2', 'Zadanie 11', new Date(0,0,0,0,2,25), new Date(0,0,0,0,2,34) ],
[ 'M1', 'Zadanie 11', new Date(0,0,0,0,3,7), new Date(0,0,0,0,3,25) ],
[ 'M2', 'Zadanie 12', new Date(0,0,0,0,0,53), new Date(0,0,0,0,1,7) ],
[ 'M1', 'Zadanie 12', new Date(0,0,0,0,1,34), new Date(0,0,0,0,1,39) ],
[ 'M2', 'Zadanie 13', new Date(0,0,0,0,3,31), new Date(0,0,0,0,3,40) ],
[ 'M1', 'Zadanie 13', new Date(0,0,0,0,3,51), new Date(0,0,0,0,4,1) ],
[ 'M2', 'Zadanie 14', new Date(0,0,0,0,0,8), new Date(0,0,0,0,0,21) ],
[ 'M1', 'Zadanie 14', new Date(0,0,0,0,1,15), new Date(0,0,0,0,1,34) ],
[ 'M2', 'Zadanie 15', new Date(0,0,0,0,0,0), new Date(0,0,0,0,0,8) ],
[ 'M1', 'Zadanie 15', new Date(0,0,0,0,0,46), new Date(0,0,0,0,0,56) ],
[ 'M1', 'PRZERWANIE 1', new Date(0,0,0,0,0,5), new Date(0,0,0,0,0,25) ],
[ 'M2', 'PRZERWANIE 2', new Date(0,0,0,0,0,34), new Date(0,0,0,0,0,53) ],
[ 'M2', 'PRZERWANIE 3', new Date(0,0,0,0,1,22), new Date(0,0,0,0,1,44) ],
[ 'M2', 'PRZERWANIE 4', new Date(0,0,0,0,1,48), new Date(0,0,0,0,2,6) ],
[ 'M1', 'PRZERWANIE 5', new Date(0,0,0,0,2,7), new Date(0,0,0,0,2,24) ],
[ 'M1', 'PRZERWANIE 6', new Date(0,0,0,0,2,43), new Date(0,0,0,0,3,7) ]]);
var options = {timeline: { groupByRowLabel: true }};chart.draw(dataTable, options);}</script><div id="example4.2" style="height: 200px;"></div></body></html>
