'use strict';

const vorpal = require('vorpal')();

vorpal
    .command('login', 'Login (u: root , p: vorpal)')
    .action(function(args, cb) {
        var self = this;

        var promise = this.prompt([
            {
                type: 'input',
                name: 'username',
                message: 'Username: '
            },
            {
                type: 'password',
                name: 'password',
                message: 'Password: '
            }
        ], function(answers) {
            // You can use callbacks
        });

        promise.then(function(answers) {
            // On promises
            if (answers.username === 'root' && answers.password === 'vorpal') {
                self.log('Successful login!');
            } else {
                self.lof('Login failed! Try username "roor" and password "vorpal"!');
            }

            cb();
        });
    });

    vorpal
        .show()
        .parse(process.argv);
