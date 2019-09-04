var ws = new ReconnectingWebSocket('ws://' + location.hostname + ':8080', null, { debug: true });

var form = document.getElementById('form');
var ssid = document.getElementById('ssid');
var wpa2pass = document.getElementById('wpa2pass');
var btnRefresh = document.getElementById('btn-refresh');
var btnConnect = document.getElementById('btn-connect');

btnRefresh.onclick = function() {
    btnRefresh.disabled = true; // Debounce the button to prevent multiple invocations
    ws.send('scan:');
};

form.onsubmit = function(evt) {
    evt.preventDefault();
    btnConnect.disabled = true;
    ws.send('auth:' + ssid.value + ':' + wpa2pass.value + ':');
}

function handleRefresh(networks) {
    btnRefresh.disabled = false; // End debounce, since we received a result
    ssid.innerHTML = '';

    var opt = document.createElement('option');
    opt.value = '';
    opt.innerHTML = 'Network';
    opt.disabled = true;
    opt.selected = true;
    opt.hidden = true;
    ssid.appendChild(opt);

    for (var network of networks) {
        opt = document.createElement('option');
        opt.value = network;
        opt.innerHTML = network;
        ssid.appendChild(opt);
    }
}

ws.onmessage = function(msg) {
    console.log('received:', msg);
    var json;
    try {
        json = JSON.parse(msg.data);
    } catch(e) {
        console.log('Failed to parse', msg.data);
        return;
    }

    if (json.networks !== undefined) {
        handleRefresh(json.networks);
    }
    if (json.authorized !== undefined) {
        btnConnect.disabled = false;
    }
}
