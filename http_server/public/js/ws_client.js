
const remoteVideo = document.getElementById('remoteVideo');
const configuration = {
    iceServers : [ {urls : 'stun:stun.l.google.com:19302'} ]
};

var localConnection;
var localDescription;
var remoteDescription;
var localIce;
var remoteIce;
var remoteStream;
var sizeIce = 0;
var index = 0;
var flagSDP = true;
var candidate_result = null;
var options = {offerToReceiveAudio : false, offerToReceiveVideo : true};
var flag_ICE = true;
var flag_Connection = false;

// #####################################################################################################################

let host = prompt("Please enter hostname websocket server with port:",
                  "10.168.0.206:10000");

// Websocket security on C/C++
var connection = new WebSocket('wss://' + host, 'wss');
console.log(connection);
connection.onerror = function() {
    console.log("Connection...");
    document.getElementById("status").innerText = "Error connection to server";
};

connection.onopen = function() {
    console.log("Send");
    flag_Connection = true;
};

connection.onclose = function(event) {
    if (localConnection != null)
        localConnection.close();
    localConnection = null;
    console.log("Close");
};
connection.onmessage = function(event) {
    var mes;
    index = parseInt(event.data);
    var Type = event.data.substr(index.toString().length + 1, 3);
    if (Type === 'Con') {
        console.log(event.data);
        conncetion.send({status : "ready"});
    }
    if (Type === 'OK') {
        console.log(event.data);
        localConnection = new RTCPeerConnection(configuration);
        localConnection.onicecandidate = sendIceCandidate; // ice candidate
        localConnection.createOffer(sLocalDescription, onError, options);
        localConnection.addEventListener('track', gotRemoteStream);
    }
    if (Type === 'SDP') {
        console.log("SDP");
        mes = event.data.substr(5);
        console.log(mes);
        var description = {type : "answer", sdp : mes};
        localConnection
            .setRemoteDescription(new RTCSessionDescription(description))
            .catch(onError_Valid_Description);
        remoteDescription = description;
    }
    if (Type === 'ICE') {
        mes = event.data.substr(5);
        var candidate =
            new RTCIceCandidate({sdpMLineIndex : 0, candidate : mes});
        remoteIce = candidate;
        console.log("Receive remote ice candidate");
        localConnection.addIceCandidate(candidate, sucIce, errorIce);
    }
    if (Type === 'Err') {
        closeAll();
        console.log(event.data);
        document.getElementById("status").innerText = event.data.substr(2);
    }
};

function sendIceCandidate(event) {
    if (event.candidate) {
        console.log("Candidate: " + event.candidate.candidate);
        var array = event.candidate.candidate.split(' ');
        if (array.indexOf("srflx") == -1) {
            candidate_result = event.candidate.candidate;
            // connection.send(index + ':ICE:' + event.candidate.candidate);
            sizeIce++;
        }
    }
}

function sLocalDescription(description) {
    localConnection.setLocalDescription(description);
    console.log("Offer description:" + description.sdp.length + " \n" +
                description.sdp);
    localDescription = description;
    // connection.send(index + ':SDP:' + description.sdp);
}

function gotRemoteStream(e) {
    if (remoteVideo.srcObject !== e.streams[0]) {
        remoteVideo.srcObject = e.streams[0];
        console.log('pc2 received remote stream');
    }
}

function onError() {
    closeAll();
    console.log("Error with description");
    document.getElementById("status").innerText = "Error with description";
    // connection.send(index + ':ERROR:' +
    //                 "Error with client description.");
}

function onError_Valid_Description() {
    closeAll();
    console.log("Error with Validate Description");
    flagSDP = false;
    document.getElementById("status").innerText =
        "Error with server description.";
    // connection.send(index + ':ERROR:' +
    //                 "Error with server description.");
}

function sucIce() { console.log("Successful in candidate other user"); }

function errorIce() {
    closeAll();
    console.log("Error in candidate client user");
    document.getElementById("status").innerText =
        "Error in candidate client user.";
    // connection.send(index + ':ERROR:' +
    //                 "Error in candidate client user.");
}

window.onunload = function() {
    closeAll();
    // connection.send(index + ':CLOSE');
    console.log("Close pages");
};

function closeAll() {
    localConnection.close();
    localConnection = null;
}
