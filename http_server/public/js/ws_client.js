const remoteVideo = document.getElementById('remoteVideo');
const iceConnectionLog = document.getElementById('ice-connection-state');
const iceGatheringLog = document.getElementById('ice-gathering-state');
const statusLog = document.getElementById('status-log');
const dataChannelLog = document.getElementById('data-channel');

const configuration = {
    bundlePolicy: "max-bundle",
    iceServers : [ {urls : 'stun:stun.l.google.com:19302'} ]
};

var currentState = "start";
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


var pc = null;
var client_id = -1;
var localDescription = null;
var remoteDescription = null;


// #####################################################################################################################

let host = prompt("Please enter hostname websocket server with port:",
                  "127.0.0.1:10001");

// Websocket security on C/C++
var connection = new WebSocket('ws://' + host);

connection.onerror = function() {
    statusLog.textContent = "Error connection to server";
};

connection.onopen = function() {
    statusLog.textContent = "Wait answer from server";
};

connection.onclose = function(event) {
    if (pc != null) {
        pc.close();
        pc = null;
    }
    console.log("Close");
};

function CreatePeerConnection(answer) {
    client_id = answer.id;
    pc = new RTCPeerConnection(configuration);

    pc.addEventListener('iceconnectionstatechange', () =>
        iceConnectionLog.textContent += ' -> ' + pc.iceConnectionState);
    iceConnectionLog.textContent = pc.iceConnectionState;

    pc.addEventListener('icegatheringstatechange', () => {
        iceGatheringLog.textContent += ' -> ' + pc.iceGatheringState;
        if (pc.iceGatheringState === 'complete') {
            const answer = pc.localDescription;
            localDescription = answer;
            connection.send(JSON.stringify({
                id: client_id,
                type: "sdp",
                sdp: answer.sdp,
            }));
        }
    });

    iceGatheringLog.textContent = pc.iceGatheringState;

    pc.ontrack = (evt) => {
        if (!remoteVideo.srcObject) {
            remoteVideo.srcObject = evt.streams[0]; // The stream groups audio and video tracks
            console.log('pc received remote stream');
            remoteVideo.play();
        }
    };
}

function SetRemoteDesciption(answer) {
    var description = {type : "offer", sdp : answer.sdp};
    remoteDescription = description;
    pc.setRemoteDescription(new RTCSessionDescription(description))
        .catch(remoteDescriptionError);

    const brAnswer = pc.createAnswer();
    pc.setLocalDescription(brAnswer);
}

function remoteDescriptionError() {
    pc.close();
    statusLog.textContent = "Error with server description.";
}

connection.onmessage = function(event) {
    const answer = JSON.parse(event.data);
    switch (answer.type) {
        case 'id':
            console.log("Type: " + answer.type + " Id: " + answer.id);
            CreatePeerConnection(answer);
            break;
        case 'sdp':
            SetRemoteDesciption(answer);
            break;
        default:
            console.log("Type undefine: " + answer.type);
    }

};

window.onunload = function() {
    pc.close();
    console.log("Close pages");
};

