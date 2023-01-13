const fs = require('fs');
const https = require('https');
const express = require('express');
const md5 = require('md5');
const sqlite3 = require('sqlite3').verbose();
const bodyParser = require("body-parser");
const Net = require('net');

const hostname_server = process.argv.slice(2)[0];
const port_server = 10000;
const hostname_logger = process.argv.slice(2)[1];
const port_logger = 10001;

const app = express();
app.use(bodyParser.urlencoded({extended : false}));
app.use(bodyParser.json());

const options = {
    key : fs.readFileSync('../certificates/key.pem'),
    cert : fs.readFileSync('../certificates/cert.pem')
};

let db =
    new sqlite3.Database('./db/login.db', sqlite3.OPEN_READWRITE, (err) => {
        if (err) {
            console.error(err.message);
        }
        console.log('Connected to the login database.');
    });

let sql = `SELECT Username, Password
           FROM users
           WHERE Username = ?`;

app.use(express.static('public'));

app.post('/', (req, res) => {
    var username = req.body.login;
    var password = req.body.pass;
    db.get(sql, [ username ], (err, row) => {
        if (row) {
            var hash = md5(password + "9999");
            if (username == row.Username && hash == row.Password) {
                res.sendFile(__dirname + '/public/stream.html');
            } else {
                res.sendFile(__dirname + '/public/index.html');
            }
        } else {
            res.sendFile(__dirname + '/public/index.html');
        }
    });
});

const httpsServer = https.createServer(options, app);
httpsServer.listen(port_server, hostname_server, () => {
    console.log('HTTPS Server running on port ' + hostname_server + ':' +
                port_server);
});

