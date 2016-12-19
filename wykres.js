google.charts.load("current", {packages:["timeline"]});

var chart;
function drawChart(container, table) {
	chart = new google.visualization.Timeline(container);
	
	var dataTable = google.visualization.arrayToDataTable([
		['Role', 'name',{role: 'style'}, 'start', 'end']
	].concat(table));
	
	var options = {
		'height':175,
		timeline: { groupByRowLabel: true,colorByRowLabel: false }
	};
	chart.draw(dataTable, options);
}

function createDiv() {
	return document.querySelector(".template .result").cloneNode(true);
}

var resultsDiv = document.getElementById('results');

function process(e){
	var text = e.target.result;
	
	var div = createDiv();
	
	//console.log(text);
	var lines = text.split("\n")
	var graphTitle = lines[0].slice(5, -5);
	var goalValue = parseInt(lines[1].split(",")[0]);
	var maintM1count = parseInt(lines[4].split(",")[0]);
	var maintM1duration = parseInt(lines[4].split(",")[1]);
	var maintM2count = parseInt(lines[5].split(",")[0]);
	var maintM2duration = parseInt(lines[5].split(",")[1]);
	var idleM1count = parseInt(lines[6].split(",")[0]);
	var idleM1duration = parseInt(lines[6].split(",")[1]);
	var idleM2count = parseInt(lines[7].split(",")[0]);
	var idleM2duration = parseInt(lines[7].split(",")[1]);
	
	div.querySelector(".wartoscFCelu").innerHTML = goalValue;
	div.querySelector(".tytulGrafu").innerHTML = graphTitle;
	div.querySelector(".lPrzerwM1").innerHTML = maintM1count;
	div.querySelector(".cPrzerwM1").innerHTML = maintM1duration;
	div.querySelector(".lPrzerwM2").innerHTML = maintM2count;
	div.querySelector(".cPrzerwM2").innerHTML = maintM2duration;
	div.querySelector(".lIdleM1").innerHTML = idleM1count;
	div.querySelector(".cIdleM1").innerHTML = idleM1duration;
	div.querySelector(".lIdleM2").innerHTML = idleM2count;
	div.querySelector(".cIdleM2").innerHTML = idleM2duration;
	
	var taskColor = '#F15025';
	var maintColor = '#595959';
	var idleColor = '#C1CAD6';
	
	var tableToGraph = [];
	
	for(var k=2;k<4;k++){
		var M = lines[k].slice(3).replace(/ /g,"").split(";");
		var Mname = "";
		if(k==2) Mname = "M1";
		else Mname = "M2";
		for(var i=0;i<M.length;i++){
			var task = M[i];
			if(task=="") continue;
			var tempTaskTab = task.split(",");
			var opName = tempTaskTab[0];
			var opColor
			if( opName[0]=="o") opColor = taskColor;
			else if( opName[0]=="m") opColor = maintColor;
			else if( opName[0]=="i") opColor = idleColor;
			else opColor = '#ff0000';
			var opStart = parseInt(tempTaskTab[1]);
			var opEnd = parseInt(tempTaskTab[2])+ opStart;
			tableToGraph.push([Mname,opName,opColor,opStart,opEnd]);
		}
	}
	resultsDiv.appendChild(div);
	drawChart(div.querySelector('.poleWykresu'), tableToGraph);
	
}

document.getElementById('upload').addEventListener('change', function(e) {
	for(var i=0;i<this.files.length;i++) {
		var file=this.files[i];
		var reader = new FileReader();
		reader.onloadend = process;
		reader.readAsText(file);
	}
}); 