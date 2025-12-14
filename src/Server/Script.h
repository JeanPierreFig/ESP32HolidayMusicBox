#ifndef SCRIPT_H
#define SCRIPT_H

const char script_js[] PROGMEM = R"rawliteral(

const evtSource = new EventSource("http://santaBox.local/events");

evtSource.onopen = () => {
    console.log("Connected to SSE server at santaBox.local");
};

evtSource.addEventListener("audio_state", e => {
    const state = JSON.parse(e.data);
    console.log("Current audio state:", state);

    if (state.isPlaying !== undefined) {
        isPlaying = state.isPlaying;
        updatePlayPauseButton();
    }
    
    if (state.volume !== undefined) {
        updateDisplayVolume(state.volume);
    }

    if (state.trackIndex !== undefined) {
        highlightTrack(state.trackIndex);
    }
});

function updateDisplayVolume(volumeValue) {
    document.getElementById('volume-slider').value = volumeValue
    document.getElementById('volume-display').textContent = volumeValue
}

function highlightTrack(index) {
    const tracks = document.querySelectorAll('.playlist .track-item');
    
    // 1. Remove highlight from all tracks
    tracks.forEach(track => {
        track.style.backgroundColor = '';
        track.style.color = '';
    });

    if (index >= 0 && index < tracks.length) {
        const currentTrack = tracks[index];
        // Apply the same highlight style as the hover effect for consistency
        currentTrack.style.backgroundColor = 'red'; 
        currentTrack.style.color = 'white';
        console.log('Highlighted track:', index + 1);
    }
}
    
function updateVolume(value) {
    document.getElementById('volume-display').textContent = value;
    
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
    .then(response => {
        if (!response.ok) {
            throw new Error('HTTP ' + response.status);
        }
        return response.json();
    })
    .then(data => {
        console.log('Action confirmed:', action);

        // Only change state AFTER success
        if (action === 'play') isPlaying = true;
        if (action === 'pause') isPlaying = false;

        updatePlayPauseButton();
    })
    .catch(error => {
        console.error('Control failed:', error);
        // UI stays unchanged
    });
}

function togglePlayPause() {
    if (isPlaying) {
        sendControl('pause');
    } else {
        sendControl('play');
    }
}

function next() {
    sendControl('next');
}

function previous() {
    sendControl('previous');
}

function updatePlayPauseButton() {
    const button = document.getElementById('playPauseBtn');
    button.textContent = isPlaying ? '⏸ PAUSE' : '▶ PLAY';
}

function fetchPlaylist() {
    console.log('Requesting playlist from /api/playlist...');
    const playlistContainer = document.querySelector('.playlist');
    
    // Clear the current list content while fetching
    playlistContainer.innerHTML = '<strong>🎶 PLAYLIST 🎶</strong><div style="color:yellow;">Loading...</div>';

    // Send HTTP GET request to the new endpoint
    fetch('/api/playlist', { method: 'GET' })
        .then(response => {
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            return response.json();
        })
        .then(data => {
            // Check if the expected array is in the response
            const playlist = data.playlist;
            if (!Array.isArray(playlist)) {
                throw new Error('Invalid playlist format received.');
            }

            // Clear the container again before rendering
            playlistContainer.innerHTML = '<strong>🎶 PLAYLIST 🎶</strong>';

            playlist.forEach((title, index) => {
                const div = document.createElement('div');
                div.classList.add('track-item');
                // Use index + 1 for display number
                div.textContent = `${index + 1}. ${title}`;
                
                div.onclick = () => selectTrack(index); 
                
                playlistContainer.appendChild(div);
            });
            console.log(`Playlist loaded with ${playlist.length} tracks.`);
        })
        .catch(error => {
            console.error('Failed to fetch playlist:', error);
            playlistContainer.innerHTML = `<strong>🎶 PLAYLIST 🎶</strong><div style="color:red;">Error loading list.</div>`;
        });
}

function selectTrack(index) {
    console.log("Selecting track index:", index);
  
    // Send a POST request to the new API endpoint
    fetch('/api/selectTrack', {
        method: 'POST',
        headers: {'Content-Type': 'application/x-www-form-urlencoded'},
        body: 'index=' + index
    })
    .then(response => {
        if (!response.ok) {
            throw new Error('Network response was not ok');
        }
        return response.json();
    })
    .then(data => {
        // Optional: Do something with the successful response
        console.log('Track selected successfully:', data);
    })
    .catch(error => {
        console.error('Error selecting track:', error);
    });
}
)rawliteral";

#endif