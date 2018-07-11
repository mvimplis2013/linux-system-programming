#!/usr/bin/env node

var amqp = require('amqplib');
var basename = require('path').basename;
var all = require('bluebird').all;

var keys = process.argv.slice(2);
if (keys.length < 1) {
    console.log('Usage: %s pattern [pattern...]', basename(process.argv[1]));

    process.exit(1);
}

amqp.connect('amqp://localhost').then( function(conn) {
    process.once('SIGINT', function() {
        conn.close();
    });

    return conn.createChannel().then( function(ch) {
        var ex = 'topic_logs';
        var ok = ch.assertExchange(ex, 'topic', {durable: false});

        ok = ok.then( function() {
            return ch.assertQueue('', {exclusive, true});
        });
    })
})