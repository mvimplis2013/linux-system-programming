var blessed = require('blessed')
  , contrib = require('blessed-contrib');

var screen = blessed.screen();

// create Layout & Widgets
var grid = new contrib.grid({
    rows: 12,
    cols: 12,
    screen: screen
});

screen.key(['escape', 'q', 'C-c'], function(ch, key) {
    return process.exit(0);
});

/**
 * Donut Options
 */
var donut = grid.set(8, 8, 4, 2, contrib.donut, {
    label: 'Percent Donut',
    radius: 16,
    arcWidth: 4,
    yPadding: 2,
    data: [{
        label: 'Storage', percent: 87
    }]
});

//var latencyLine = grid.set(8, 10, 4, 2,contrib.line, {
//    style: {
//        line: 'yellow',
//        'text': 'green',
//        baseline: 'black',
//    },
//    xLabelPadding: 3,
//    xPadding: 5,
//    label: 'Network Latency (sec)'
//});

var gauge = grid.set(8, 10, 2, 2, contrib.gauge, {
    label: 'Storage', 
    percent: [80, 20]
});

var gauge2 = grid.set(2, 9, 2, 3, contrib.gauge, {
    label: 'Deployment Progress',
    percent: 80
});

var sparkline = grid.set(10, 10, 2, 2, contrib.sparkline, {
    label: 'Throughput (bits/sec)',
    tags: true,
    style: {
        fg: 'blue',
        titleFg: 'white'
    }
});

var bar = grid.set(4, 6, 4, 3, contrib.bar, {
    label: 'Server Utilization (%)',
    barWidth: 4,
    barSpacing: 6,
    xOffset: 2,
    maxHeight: 9
});

var table = grid.set(4, 9, 4, 3, contrib.table, {
    label: 'Active Processes',
    keys: true,
    fg: 'green',
    columnSpacing: 1,
    columnWidth: [10 , 10, 10]
});

var log = grid.set(8, 6, 4, 2, contrib.log, {
    fg: 'green',
    selectedFg: 'green',
    label: 'Server Log',
});

var transactionsLine = grid.set(0, 0, 6, 6, contrib.line, 
    {
        showNthLabel: 5,
        maxY: 100,
        label: 'Total Transactions',
        showLegend: true,
        legend: {
            width: 10
        }
    });

var map = grid.set(6, 0, 6, 6, contrib.map, {
    label: 'Servers Location'
});

// dummy data 
var servers = ['US1', 'US2', 'EU1', 'AU1', 'AS1', 'JP1']
var commands = ['grep', 'node', 'java', 'timer', '~/ls -l', 'netns', 
                'watchdog', 'gulp', 'tar -xvf', 'awk', 'npm install'];

// Set dummy data on gauge
var gauge_percent = 0;
setInterval( function() {
    gauge.setData([gauge_percent, 100-gauge_percent]);
    gauge_percent++;
    if (gauge_percent>=100) gauge_percent = 0;
}, 200);

var pct = 0.00;

function updateDonut() {
    if (pct > 0.99) {
        pct = 0.00;
    }

    var color = 'green';

    if (pct >= 0.25) color = 'cyan';
    if (pct >= 0.50) color = 'yellow';
    if (pct >= 0.75) color = 'red';
    
    donut.setData([
        {
            percent: parseFloat((pct+0.00) % 1).toFixed(2), 
            label: 'storage',
            color: color
        }
    ]);
    pct += 0.01;
}

setInterval( function() {
    updateDonut();
    screen.render();
}, 500);

// Set dummy data for table
function generateTable() {
    var data = [];

    for (var i=0; i<30; i++) {
        var row = [];

        row.push(commands[Math.round(Math.random()*(commands.length-1))]);
        row.push(Math.round(Math.random()*5));
        row.push(Math.round(Math.random()*100));

        data.push(row);
    }

    table.setData( {
        headers: ['Process', 'Cpu (%)', 'Mem'], 
        data: data
    });
}

generateTable();
table.focus();
setInterval(generateTable, 3000);

// set log dummy data 
setInterval(function() {
    var rnd = Math.round(Math.random()*2);

    if (rnd==0) log.log('starting process ' + commands[Math.random(Math.random()*(commands.length-1))]);
    else if (rnd==1) log.log('terminating server ' + servers[Math.round()*(servers.length-1)]);
    else if (rnd==2) log.log('avg. wait time ' + Math.random().toFixed(2));

    screen.render();
}, 500);

var spark1 = [1,2,5,2,1,5,1,2,5,2,1,5,4,4,5,4,1,5,1,2,5,2,1,5,1,2,5,2,1,5];
var spark2 = [4,4,5,4,1,5,1,2,5,2,1,5,4,4,5,4,1,5,1,2,5,2,1,5,1,2,5,2,1,5];

refreshSpark();
setInterval(refreshSpark, 1000);

function refreshSpark() {
    spark1.shift();
    spark1.push(Math.random()*5+1);
    spark2.shift();
    spark2.push(Math.random()*5+1);
    sparkline.setData(['Server1', 'Server2'], [spark1, spark2]);
};

// set line charts dummy data 
var transactionsData = {
    title: 'USA',
    style: {
        line: 'red'
    },
    x: ['00:00', '00:05'],
    y: [0, 20],
} ;

setLineData([transactionsData], transactionsLine);

setInterval(function() {
    setLineData([transactionsData], transactionsLine);
    screen.render();
}, 500);

function setLineData(mockData, line) {
    for (var i=0; i<mockData.length; i++) {
        var last = mockData[i].y[mockData[i].y.length-1]
        mockData[i].y.shift()
        var num = Math.max(last + Math.round(Math.random()*10)-5, 10)
        mockData[i].y.push(num)
    }

    line.setData(mockData);
};

screen.render();