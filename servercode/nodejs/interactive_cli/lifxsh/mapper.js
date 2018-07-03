'use strict';

const _ = require('lodash');
const log = require('./log');

let nameToId = {};

let mapper = {
    /**
     * Add light name to identifier mapping.
     * 
     * @param {string} id - Light identifier
     * @param {string} name - Light name mapping
     */
    add: function addName(id, name) {
        if (!id || !name) {
            log.error('Could not add light mapping. ID or Name not defined!');
            return;
        }

        let key = getKey(name);
        nameToId[key] = id;
    },

    /**
     * Get light identifier by name.
     * 
     * @param {string}name - Light name 
     * @return {string|undefined} - Light identifier or undefined
     */
    get: function getId(name) {
        if (!name) {
            log.error('Could not find light mapping. No name defined !');
            return;
        }

        // value found from nameToId should already be an identifier
        if (_.includes(nameToId, name)) {
            return name;
        }
    }
}