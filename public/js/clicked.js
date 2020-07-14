var size_camera = 0;
var row = 0;
var flags = false;
function clickedAdd() {
    if (flags)
    {
        var input_ip = document.getElementById('ip_addres');
        var re = /((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(\.|$)){4}/;
        var OK = re.exec(input_ip.value);
        if (input_ip.value == "petrov.in.ua")
        {
            OK = true;
        }
        if (input_ip.value.length >= 16 || !OK)
        {
            document.getElementById('url').innerHTML = "Bad IP";
        }
        else
        {
            var xhr = new XMLHttpRequest();
            var body = 'ip=' + encodeURIComponent(input_ip.value);
            console.log(body);
            xhr.open("POST", '/new', );
            xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
            xhr.send(body);
            xhr.onload = function() {
                var mes = xhr.response;
                if (mes == "Already")
                {
                    console.log("Already new");
                    return;
                }
                else
                {
                    console.log("Create");
                    var td = document.getElementById('td_add_camera');
                    let div_test = document.createElement('div');
                    div_test.className = "div_form_camera";
                    
                    size_camera += 1;
                    div_test.id = input_ip.value; //.substr(7);
                    if(size_camera == 7)
                    {
                        size_camera = 1;
                        row += 1;
                    }
                    div_test.style.top = (row*280+95).toString() + "%";
                    div_test.style.left = (size_camera*10 + 30) + "%";
                    div_test.innerHTML = '<p align="center"><img src=images/delete.png onclick="clickedNone(\'' + input_ip.value + '\')"></img></p><p align="center"><img src=images/video-camera.png onclick="clickStream(\'' + input_ip.value + '\')"></img></p><p align="center">' + input_ip.value + '</p>';       
                    td.append(div_test);
                }
            };
        }
    }
}

function clickedAdd_mobile() {
}

function clickStream(text) {    
    document.location="/stream.html?ip="+text;
}

function clickedNone(text) {
    var id = text;
    console.log(id);
    size_camera -= 1;
    if (size_camera == 0)
    {
        if (row != 0)
        {
            size_camera = 6;
            row -= 1;
        }
    }
    //document.getElementById(id).style.display = "none";
    var el = document.getElementById(id);
    el.parentNode.removeChild(el);
    var xhr = new XMLHttpRequest();
    var body = 'ip=' + encodeURIComponent(id);
    console.log(body);
    xhr.open("POST", '/delete', );
    xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
    xhr.send(body);
}


function createUrl() {
    var input1 = document.getElementById('ip_addres');
    var input2 = document.getElementById('ip_addres_mobile');
    if (input1.value.length >= 16 || input2.value.length >= 16)
    {
        document.getElementById('url').innerHTML = "Bad IP";
        document.getElementById('url_mobile').innerHTML = "Bad IP";
    }
    else
    {        
        document.getElementById('url').innerHTML = "rtsp://" + input1.value + "/axis-media/media.amp";
        document.getElementById('url_mobile').innerHTML = "rtsp://" + input2.value + "/axis-media/media.amp";
    }
}

window.onload = function()
{
    const host = window.location.href;
    var n = host.search("html");
    if (n == -1)
    {
        flags = true;
        var xhr = new XMLHttpRequest();
        xhr.open("POST", '/read',);
        xhr.send();
        xhr.onload = function() {
            console.log(xhr.response); // response -- это ответ сервера
            var json = JSON.parse(xhr.response);
            console.log(json);
            for (var i = 0; i < json.length; i++)
            {
                console.log(json[i].ip);
                var ip =json[i].ip;
                console.log("Create");
                var td = document.getElementById('td_add_camera');
                let div_test = document.createElement('div');
                div_test.className = "div_form_camera";
                
                size_camera += 1;
                div_test.id = ip; //.substr(7);
                if(size_camera == 7)
                {
                    size_camera = 1;
                    row += 1;
                }
                div_test.style.top = (row*280+95).toString() + "%";
                div_test.style.left = (size_camera*10 + 30) + "%";
                div_test.innerHTML = '<p align="center"><img src=images/delete.png onclick="clickedNone(\'' + ip + '\')"></img></p><p align="center"><img src=images/video-camera.png onclick="clickStream(\'' + ip + '\')"></img></p><p align="center">' + ip + '</p>';       
                td.append(div_test);
            }
        };
    }
    else
    {
        console.log("Error");
    }
};
