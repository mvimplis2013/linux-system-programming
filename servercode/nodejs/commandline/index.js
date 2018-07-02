#!/usr/local/bin/node

var program = require('commander');


program
    .version('0.1.0')
    .option('-C, --chdir <path>', 'change the working directory')
    .option('-c, --config <path>', 'set config path ... defaults to ./deploy.conf')
    .option('-T, --no-tests', 'ignore test hook')

program
    .command('setup')
    .description('run remote setup commands')
    .action(function() {
        console.log('setup');
    });

program.parse(process.argv);

console.log('you ordered a pizza with:');

/*
var prompt = require('prompt');

var schema = {
    properties: {
        name: {
            pattern: /^[a-zA-Z\s\-]+$/,
            message: 'Name must be only letters, spaces, or dashes',
            required: true
        }, 
        password: {
            hidden: true
        }
    }
};

prompt.start();

prompt.get(schema, function(err, result) {
    console.log('Command-line input received:');
    console.log(' name: ' + result.name);
    console.log(' password: ' + result.password);
});
*/