const fs = require('fs');
const https = require('https');
const express = require('express');
const path = require('path');
const host = process.argv.slice(2)[0];
var bodyParser = require("body-parser");
//var mysql = require('mysql');
const app = express();
app.use(bodyParser.urlencoded({ extended: false }));
app.use(bodyParser.json());

// Certificate

const privateKey = fs.readFileSync('./certificate/danil_petrov.key');
const certificate = fs.readFileSync('./certificate/danil_petrov.crt');

const credentials = {
	key: privateKey,
	cert: certificate,
};

// let connection = mysql.createConnection({
//     host: 'localhost',
//     user: 'root',
//     password: '',
//     database: 'LiveStream'
// });

app.use(express.static('public'));

app.post('/', (req, res) => {
    var user_name = req.body.login;
    var password = req.body.pass;
    console.log("User name = " + user_name + ", password is " + password);
    if(user_name == 'root' && password == 'pass')
        res.sendFile( __dirname + '/public/stream.html');
    else
	res.sendFile( __dirname + '/public/index.html');
});

// app.get('/about', (req, res) =>{
//     res.sendFile( __dirname + '/public/about.html');
// });



// app.post('/new', (req, res) => {
//     var ip = req.body.ip;
//     var f = true;
//     const sql_select = "SELECT * FROM camers WHERE ip='" + ip + "'";
//     connection.query(sql_select, function(err, results) {
//         if(err) console.log(err);
//         console.log(results);
//         if (results.length)
//         {
//             res.send("Already");
//             console.log("Already");
//             f = false;
//         }
//         else
//         {
//             var sql_insert = "INSERT INTO camers (ip) VALUES ('"+ ip +"')";
//             connection.query(sql_insert, function (err, result) {
//                 if (err) throw err;
//                 console.log("1 record inserted");
//                 res.send("OK");
//             });
//         }
//     });
// });


// app.get('/stream', (req,res) =>{
//     console.log("Get Stream");
//     res.sendFile( __dirname + '/public/stream.html');
// });

// app.post('/stream', (req,res) =>{
//     var last_request = req.body.ip;
//     console.log("Post Stream: " + last_request);
// });

// app.post('/request', (req,res) =>{
//     console.log("Request: " + last_request);
//     res.send(last_request);
// });


// app.post('/read', (req, res) => {
//     const sql = `SELECT * FROM camers`;
//     connection.query(sql, function(err, results) {
//         if(err) console.log(err);
//         console.log(results);
//         res.send(results);
//     });
// });

// app.post('/delete', (req, res) => {
//     var ip = req.body.ip;
//     var sql = "DELETE FROM camers WHERE ip='" + ip + "'";
//     connection.query(sql, function (err, result) {
//         if (err) throw err;
//         console.log("Number of records deleted: " + result.affectedRows);
//     });
// });

const httpsServer = https.createServer(credentials, app);

httpsServer.listen(3000, host, () => {
    console.log('HTTPS Server running on port '+ host +':3000');
});
