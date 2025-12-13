#ifndef INDEX_H
#define INDEX_H

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <title>🎵 Music Box 🎵</title>
  <meta name="keywords" content="music, christmas, mp3, 90s, geocities" />
  <meta name="description" content="A totally radical 90s-style Christmas music player" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <style>
    body {
      background-image: url("https://media.giphy.com/media/Nhfd9oF0pYvn2/giphy.gif");
      background-repeat: repeat;
      background-size: 200px 200px;
      font-family: "Comic Sans MS", "Arial", sans-serif;
      color: #00ff00;
      text-align: center;
      cursor: pointer;
    }
    h1 {
      font-size: clamp(28px, 6vw, 42px);
      color: red;
      text-shadow: 2px 2px yellow;
      animation: blink 1s infinite;
    }
    @keyframes blink {
      0% { opacity: 1; }
      50% { opacity: 0.4; }
      100% { opacity: 1; }
    }
    .container {
      max-width: 600px;
      width: 90%;
      margin: 0 auto;
      border: 6px ridge red;
      background: black;
      padding: 16px;
      box-sizing: border-box;
    }
    .marquee {
      background: green;
      color: white;
      padding: 6px;
      margin-bottom: 10px;
      font-weight: bold;
    }
    button {
      font-family: "Comic Sans MS";
      background: red;
      color: yellow;
      border: 3px outset green;
      padding: 6px 10px;
      margin: 4px;
      font-size: 14px;
      cursor: pointer;
      width: auto;
      min-width: 90px;
    }
    button:active {
      border: 3px inset green;
    }
    .playlist {
      text-align: left;
      margin-top: 15px;
      background: #111;
      padding: 10px;
      border: 3px dotted lime;
    }
    .playlist div {
      padding: 4px;
      cursor: pointer;
    }
    .playlist div:hover {
      background: red;
      color: white;
    }
    .footer {
      margin-top: 20px;
      font-size: 12px;
      color: cyan;
    }
  </style>
</head>
<body>
  <h1><img src="https://media3.giphy.com/media/v1.Y2lkPTc5MGI3NjExMm41ODg0aHp0cmZnMjVhNnJreHd2ZGUyNnkwbnpidGR1dTd3N3F2NyZlcD12MV9pbnRlcm5hbF9naWZfYnlfaWQmY3Q9cw/IBAFn2cP42zkCYLL5F/giphy.gif" alt="Music Note" style="width: 40px; height: 40px; vertical-align: middle; image-rendering: pixelated;" /> MUSIC BOX <img src="https://media3.giphy.com/media/v1.Y2lkPTc5MGI3NjExMm41ODg0aHp0cmZnMjVhNnJreHd2ZGUyNnkwbnpidGR1dTd3N3F2NyZlcD12MV9pbnRlcm5hbF9naWZfYnlfaWQmY3Q9cw/IBAFn2cP42zkCYLL5F/giphy.gif" alt="Music Note" style="width: 40px; height: 40px; vertical-align: middle; image-rendering: pixelated;" /></h1>
  <img src="https://media1.giphy.com/media/v1.Y2lkPTc5MGI3NjExOWE5N2tnYTdpejNhZTBuaTY1OWVsZWpkcDYzbDQ0NnY4aTA1a2x0cyZlcD12MV9pbnRlcm5hbF9naWZfYnlfaWQmY3Q9cw/cE4hYhquh5YnkAWysd/giphy.gif" alt="Dancing Santa" style="margin-bottom:10px; image-rendering: pixelated;" />
  <div class="container">
    <marquee class="marquee" behavior="alternate">
      ❄️ Welcome to my totally awesome 90s Christmas Music Player! ❄️
    </marquee>
    <audio id="player" controls style="width:100%">
      <source src="song1.mp3" type="audio/mpeg">
      Your browser does not support the audio tag.
    </audio>
    <div style="display:flex; flex-wrap:wrap; justify-content:center;">
      <button onclick="previous()">⏮ PREVIOUS</button>
      <button onclick="play()">▶ PLAY</button>
      <button onclick="pause()">⏸ PAUSE</button>
      <button onclick="next()">⏭ NEXT</button>
    </div>
    <div style="margin-top:10px; color:cyan;">
      🔊 Volume: <span id="volume-display">100</span>%<br>
      <input type="range" min="0" max="100" value="20" oninput="updateVolume(this.value)" style="width:60%;">
    </div>
    <div class="playlist">
      <strong>🎶 PLAYLIST 🎶</strong>
      <div onclick="loadSong('song1.mp3')">Jingle Bells</div>
      <div onclick="loadSong('song2.mp3')">Silent Night</div>
      <div onclick="loadSong('song3.mp3')">Deck the Halls</div>
    </div>
  </div>
  <div class="footer">
    <p>Best viewed in Netscape Navigator 4.0 😎</p>
    <p>© 1997 SantaNet Productions</p>
    <img src="https://upload.wikimedia.org/wikipedia/commons/6/6a/Under_construction_icon-yellow.gif" alt="Under Construction" />
  </div>

   <script>
        {{SCRIPT_CONTENT}}
    </script>
</body>
</html>
)rawliteral";

#endif