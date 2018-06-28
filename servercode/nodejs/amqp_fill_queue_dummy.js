var amqp = require('amqplib/callback_api');

// if the connection is closed or fails to be established at all, we will reconnect
var amqpConn = null;
function start() {
    amqp.connect(
        'amqp://localhost:5672', function(err, conn) 
        {
            if (err) {
                console.error("[AMQP]", err.message);
                return setTimeout(start, 1000);
            }

            conn.on("error", function(err) {
                if (err.message !== "Connection closing") {
                    console.error("[AMQP] conn error", err.message);
                }
            });

            conn.on("close", function() {
                console.error("[AMQP] reconnecting");
                return setTimeout(start, 1000);
            });

            console.log("[AMQP] connected");
            amqpConn = conn;

            whenConnected();
        });
}

function whenConnected() {
    startPublisher();
    startWorker();
}

var pubChannel = null;
var offlinePubQueue = [];

function startPublisher() {
    amqpConn.createConfirmChannel(
        function(err, ch) {
            if (closeOnErr(err)) {
                return;
            }

            ch.on("error", function(err) {
                console.error("[AMQP] channel error", err.message);
            });

            ch.on("close", function() {
                console.log("[AMQP] channel closed");
            });

            pubChannel = ch;

            while (true) {
                var m = offlineQueue.shift();

                if (!m) break;

                publish(m[0], m[1], m[2]);
            }

        });
}

function publish(exchange, routingkey, content) {
    try {
        pubChannel.publish(exchange, routingkey, content, {persistent: true},
            function(err, ok) {
                if (err) {
                    console.error("[AMQP] publish", err);
                    offlinePubQueue.push([exchange, routingkey, content]);
                    pubChannel.connection.close();
                }
            });
    } catch (e) {
        console.error("[AMQP] publish", e.message);
        offlinePubQueue.push([exchange, routingkey, content]);
    }
}

// A woerker that acks messages only if processed successfully
function startWorker() {
    amqpConn.createChannel( function(err, ch) {
        if (closeOnErr(err)) {
            return;
        }

        ch.on("error");
        ch.on("close");
        
    })
            var q = 'celery';

        ch.assertQueue(q, {durable: true});

        ch.sendToQueue(q, new Buffer('Snoopy and Friends 66!'));

        console.log("[x] Sent `Snoopy and Friends1");
    });

    setTimeout( function() {
        conn.close();
        process.exit(0);
    }, 500);
});

/*var connection = amqp.createConnection(
    {
        host: 'localhost', port: 5672
    });

var started = false;

connection.on('ready', function() {
    if (started == false) {
        started = true;

        connection.exchange('celery', {confirm: true}, function(exchange) {
            publish(exchange, 1);
        });
    }
});

function publish(exc, i) {
    if (i === 100) {
        return connection.disconnect();
    }

    exc.publish('celery', i, {}, function(err) {
        console.lod('Added ' + i);
        setTimeout(function() {
            publish(exc, ++i);
        }, 1);
    });
}
*/