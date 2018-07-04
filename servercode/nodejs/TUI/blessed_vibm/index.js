var blessed = require('blessed');

// Create a screen object
var screen = blessed.screen({
    smartCSR: true
});

screen.title = 'my window title';

// Create a box perfectly centered horizontally and vertically
var box = blessed.box({
    top: 'center',
    left: 'center',
    width: '50%',
    height: '50%',
    content: 'Hello {bold}world{/bold}!',
    tags: true,
    border: {
        type: 'line'
    },
    style: {
        fg: 'white',
        bg: 'magenta',
        border: {
            fg: '#f0f0f0'
        },
        hover: {
            bg: 'green'
        }
    }
});

// Add a PNG icon to the box
var icon = blessed.image({
    parent: box,
    top: 0,
    left: 0,
    type: 'overlay',
    width: 'shrink',
    height: 'shrink',
    file: __dirname + '/index2.png',
    search: false
});

// If our box is clicked, change the content
box.on('click', function(data) {
    box.setContent('{center}Some different {red-fg}content{/red-fg}.{/center}');
    screen.render();
});

// Append our box to the screen
screen.append(box);

// Quit on Escape, q, or Control-C
screen.key(['escape', 'q', 'C-c'], function(ch, key) {
    return process.exit(0);
});

// Focus our element
box.focus();

screen.render();