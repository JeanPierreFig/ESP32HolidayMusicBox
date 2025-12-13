const char script_js[] PROGMEM = R"rawliteral(
function updateVolume(value) {
    document.getElementById('volume-display').textContent = value + '%';
    
    fetch('/api/volume', {
        method: 'POST',
        headers: {'Content-Type': 'application/x-www-form-urlencoded'},
        body: 'volume=' + value
    })
    .then(response => response.json())
    .then(data => console.log('Volume set:', data))
    .catch(error => console.error('Error:', error));
}

function sendControl(action) {
    fetch('/api/control', {
        method: 'POST',
        headers: {'Content-Type': 'application/x-www-form-urlencoded'},
        body: 'action=' + action
    })
    .then(response => response.json())
    .then(data => console.log('Action:', data))
    .catch(error => console.error('Error:', error));
}

function play() {
    sendControl('play');
}

function pause() {
    sendControl('pause');
}

function next() {
    sendControl('next');
}

function previous() {
    sendControl('previous');
}
)rawliteral";

#endif