'use strict';

const vorpal = require('vorpal')();
const chalk = vorpal.chalk;

vorpal
    //.title(chalk.magenta('Vorpal'))
    .version('1.4.0')
    //.description(chalk.cyan('Conquer the command-line.'))
    //.banner(chalk.gray('====================------>'));

vorpal.command('build', 'Build the application')
    .option('-d')
    .option('-a')
    .action(function(args, cb) {
        this.log(args);
        cb();
    });

vorpal.command('compress', 'Compress assets.')
    .option('-gzip')
    .action(function(args, cb) {
        this.log(args);
        cb();
    });

vorpal
    .catch('', 'Displays the index view.')
    .action(function(args, cb) {
        this.log(this.parent._commandHelp(args,command));
        cb();
    });

vorpal
    .delimiter('vorpal:')
    .show()
    .parse(process.argv);