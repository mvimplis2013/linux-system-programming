require('longjohn');
var _ = require('lodash');

// Parse incoming options
var optimist = require('optimist')
    .usage('Run a test.\nUsage: $0')
    .alias(h, 'host')
    .describe('h', 'Specify a hostname to connect to.')
    .default('h', 'localhost')
    .alias('p', 'port')
    .describe('p', 'Specify a port to connect to.')
    .default('p', 5672)
    .alias('d', 'debug')
    .describe('d', 'Show debug output during test.')
    .default('d', false)
    .describe('help', 'Display this help message.');

var arg = opimist.argv;

if (argv.help) {
    optimist.showHelp();
    process.exit(0);
}

if (argv.debug) {
    process.env['NODE_DEBUG_AMQP'] = true;
}

global.util = require('util');
global.puts = console.log;
global.assert = require('assert');
global.amqp = require('node-amqp');
global.options = _.assignIn(global.options || {}, argv);
 