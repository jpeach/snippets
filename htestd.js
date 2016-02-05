#! /usr/bin/env node

// htestd - HTTP Test Daemon
//
// A janky little HTTP server for testing peculiar things.

'use strict';

var http = require ('http');
var url = require ('url');
var console = require ('console');
var util = require ('util');

// Various utility shorthands ...
var info = console.info;
var fmt = util.format;

var port = 60000;

function send_response(response, status, body) {
    if (!body) {
        body = http.STATUS_CODES[status]
    }

    response.writeHead(status, {
        'Content-Length': body.length,
        'Content-Type': 'text/plain'
    });
    response.write(body);
    response.end();
}

// Close the socket without sending a response (with an optional timeout).
//
// Syntax:
//
// [UNITS/TIME]
//
// UNITS:
//      's', 'sec' -> seconds
//      'ms', 'msec' -> milliseconds
function send_no_response(request, response, path) {
    var ms = 0;
    var now = new Date().getTime();

    if (path.length == 2) {
        switch (path[0]) {
            case 's':
            case 'sec':
                ms = path[1] * 1000;
                break;
            case 'ms':
            case 'msec':
                ms = path[1];
                break;
            default:
                return send_response(response, 400, fmt('unsupported time unit \'%s\'\n', path[0]));
        }
    }

    return setTimeout(function() {
        var elapsed = new Date().getTime() - now;
        request.socket.destroy();
    }, ms);
}

// Send a cacheable response
//
// Examples:
//  /cacheable
//  /cacheable/anything/you/like
function send_cacheable_response(request, response, path) {
    var status = 200;
    var body = http.STATUS_CODES[status] + "\n";

    response.writeHead(status, {
        //'Content-Length' : body.length,
        'Content-Type': 'text/plain',
        'Cache-Control': 'public,max-age=6000'
    });
    response.write(body);
    response.end();
}

// Respond 200 OK after a timeout.
//
// Syntax:
//
// UNITS/TIME[/(head|body)]
//
// UNITS:
//      's', 'sec' -> seconds
//      'ms', 'msec' -> milliseconds
// (head|body):
//      Optional flag to specify whether the timeout occurs
//      before sending the headers or while we are sending
//      the body. 'head' is the default.
//
// Examples:
//  /timeout/s/10
//  /timeout/msec/500/body
function send_timeout_response(request, response, path) {
    var ms;
    var how;
    var now = new Date().getTime();

    if (!path || path.length < 2 || path.length > 3) {
        return send_response(response, 400);
    }

    how = (path.length == 3) ? path[2] : 'head';

    switch (path[0]) {
        case 's':
        case 'sec':
            ms = path[1] * 1000;
            break;
        case 'ms':
        case 'msec':
            ms = path[1];
            break;
        default:
            return send_response(response, 400, fmt('unsupported time unit \'%s\'\n', path[0]));
    }

    switch (how) {
        case 'head':
            // Wait for the timeout, then send the head and body.
            return setTimeout(function() {
                var elapsed = new Date().getTime() - now;
                var body = fmt('%sms elapsed\n', elapsed);
                send_response(response, 200, body);
            }, ms);

        case 'body':
            // Send the head and partial body, wait the timeout, then finish.
            response.writeHead(200, {
                'Content-Type': 'text/plain'
            });

            response.write('body pending ...\n');
            return setTimeout(function() {
                var elapsed = new Date().getTime() - now;
                var body = fmt('%sms elapsed\n', elapsed);
                response.write(body);
                response.end();
            }, ms);

        default:
            return send_response(response, 400, fmt('invalid timeout type \'%s\'\n', how));
    }
}

// Respond with the a truncated HTTP response body.
function send_truncated_response(request, response, path) {
    response.chunkedEncoding = false;
    response.shouldKeepAlive = false;

    response.writeHead(200, {
        'Content-Type': 'text/plain',
        'Content-Length': '100'
    });

    response.write('body pending ...\n');
    return setTimeout(function() {
        response.write('or not\n');
        response.end();
    }, 1000);
}

// Respond with the requested HTTP status.
function send_status_response(response, path) {
    var status;

    if (!path || path.length != 1) {
        return send_response(response, 400);
    }

    status = parseInt(path[0]);
    if (isNaN(status)) {
        return send_response(response, 400);
    }

    return send_response(response, status, fmt('you asked for a %d response\n', status));
}

// Respond with a JSON body containing the request headers.
//
// Examples:
//  /headers
//  /headers/anything you like
function send_headers_response(request, response, path) {
    response.writeHead(200, {
        'Content-Type': 'application/json'
    });
    response.write(JSON.stringify(request.headers));
    response.write('\n');
    response.end();
}

var routes = {
    'timeout': send_timeout_response,
    'truncate': send_truncated_response,
    'status': send_status_response,
    'cacheable': send_cacheable_response,
    'headers': send_headers_response,
    'abort': send_no_response,
};

var server = http.createServer(function (request, response) {
    var u = url.parse(request.url);

    if (request.method != 'GET') {
        return send_response(response, 400, 'Only GET is supported');
    }

    if (!u.pathname) {
        // No path /.. bad request ...
        return send_response(response, 400, 'You had better send a path');
    }

    console.info('attempting to route path %s', u.pathname);

    var current = routes;
    var components = u.pathname.split("/");
    for (var i = 0; i < components.length; ++i) {
        if (components[i] === '') {
            continue;
        }

        if (components[i] in current) {
            current = current[components[i]];
        } else if (typeof(current) === typeof(Function)) {
            // We have a handler for the partial path, but there's more of the
            // path to consume. For example, the timeout handler take care or
            // URLs like /timeout/ms/100
            return current(request, response, components.slice(i, components.length));

        } else {
            return send_response(response, 404);
        }
    }

    if (typeof(current) === typeof(Function)) {
        return current(request, response, u.pathname.split("/"));
    }

    return send_response(response, 404);
});

server.listen(port, null, null,  function() { console.info('listening on port %d', port); });
// vim: ts=4 sw=4 et :
