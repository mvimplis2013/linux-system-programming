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