#include "web_config.h"
#include "variables.h"
#include "storage_manager.h"
#include "wifi_manager.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>

AsyncWebServer server(80);

// ================================================
// MAIN DASHBOARD PAGE
// ================================================
const char MAIN_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Smart Cooler Dashboard</title>
<style>
* {margin:0; padding:0; box-sizing:border-box;}
body {
  font-family: 'Segoe UI', Arial, sans-serif;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  min-height: 100vh;
  padding: 20px;
}
.container {
  max-width: 1200px;
  margin: 0 auto;
}
.header {
  background: rgba(255,255,255,0.95);
  padding: 20px;
  border-radius: 15px;
  box-shadow: 0 8px 32px rgba(0,0,0,0.1);
  margin-bottom: 20px;
  display: flex;
  justify-content: space-between;
  align-items: center;
  flex-wrap: wrap;
}
.header h1 {
  color: #667eea;
  font-size: 28px;
  margin-bottom: 5px;
}
.lang-switch {
  display: flex;
  gap: 10px;
}
.lang-btn {
  padding: 8px 16px;
  border: 2px solid #667eea;
  background: white;
  color: #667eea;
  border-radius: 8px;
  cursor: pointer;
  font-weight: 600;
  transition: all 0.3s;
}
.lang-btn.active {
  background: #667eea;
  color: white;
}
.lang-btn:hover {
  transform: translateY(-2px);
  box-shadow: 0 4px 12px rgba(102,126,234,0.3);
}
.grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
  gap: 20px;
  margin-bottom: 20px;
}
.card {
  background: rgba(255,255,255,0.95);
  padding: 25px;
  border-radius: 15px;
  box-shadow: 0 8px 32px rgba(0,0,0,0.1);
}
.card h2 {
  color: #667eea;
  margin-bottom: 20px;
  font-size: 22px;
  border-bottom: 2px solid #667eea;
  padding-bottom: 10px;
}
.sensor-grid {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 15px;
  margin-bottom: 15px;
}
.sensor-box {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  padding: 15px;
  border-radius: 10px;
  text-align: center;
}
.sensor-box label {
  font-size: 12px;
  opacity: 0.9;
  display: block;
  margin-bottom: 5px;
}
.sensor-box .value {
  font-size: 24px;
  font-weight: bold;
}
.control-row {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin: 15px 0;
  padding: 10px;
  background: #f5f7fa;
  border-radius: 8px;
}
.control-row label {
  font-weight: 600;
  color: #333;
}
select {
  padding: 8px 15px;
  border: 2px solid #667eea;
  border-radius: 8px;
  background: white;
  color: #333;
  font-size: 14px;
  cursor: pointer;
  min-width: 150px;
}
.btn {
  padding: 12px 30px;
  border: none;
  border-radius: 10px;
  font-size: 16px;
  font-weight: 600;
  cursor: pointer;
  transition: all 0.3s;
  width: 100%;
  margin-top: 10px;
}
.btn-relay {
  background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);
  color: white;
  font-size: 28px;
  padding: 40px;
  height: 120px;
  display: flex;
  align-items: center;
  justify-content: center;
}
.btn-relay.on {
  background: linear-gradient(135deg, #4facfe 0%, #00f2fe 100%);
}
.btn:hover {
  transform: translateY(-2px);
  box-shadow: 0 6px 20px rgba(0,0,0,0.15);
}
.btn-config {
  background: linear-gradient(135deg, #43e97b 0%, #38f9d7 100%);
  color: white;
  text-decoration: none;
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 10px;
  font-size: 20px;
  padding: 20px;
  height: 70px;
}
.btn-wifi {
  background: linear-gradient(135deg, #43e97b 0%, #38f9d7 100%);
  color: white;
  text-decoration: none;
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 10px;
  font-size: 20px;
  padding: 20px;
  height: 70px;
}
.button-row {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 10px;
  margin-top: 10px;
}
.status-badge {
  display: inline-block;
  padding: 6px 12px;
  border-radius: 20px;
  font-size: 12px;
  font-weight: 600;
  margin-left: 10px;
}
.status-online {
  background: #43e97b;
  color: white;
}
.timer-display {
  background: #f5f7fa;
  padding: 15px;
  border-radius: 10px;
  text-align: center;
  margin-top: 15px;
}
.timer-display .time {
  font-size: 32px;
  font-weight: bold;
  color: #667eea;
  font-family: monospace;
}
@media (max-width: 768px) {
  .header {flex-direction: column; text-align: center;}
  .sensor-grid {grid-template-columns: 1fr;}
  .grid {grid-template-columns: 1fr;}
  .button-row {grid-template-columns: 1fr;}
}
</style>
</head>
<body>

<div class="container">
  <div class="header">
    <div>
      <h1 id="title">Smart Cooler Dashboard</h1>
      <span class="status-badge status-online">● <span id="status">Online</span></span>
    </div>
    <div class="lang-switch">
      <button class="lang-btn active" onclick="setLang('en')">English</button>
      <button class="lang-btn" onclick="setLang('bn')">বাংলা</button>
    </div>
  </div>

  <div class="grid">
    <!-- Sensor Data Card -->
    <div class="card">
      <h2 id="sensorTitle">Sensor Readings</h2>
      <div class="sensor-grid">
        <div class="sensor-box">
          <label id="lblAmbTemp">Ambient Temp</label>
          <div class="value"><span id="ambT">--</span>°C</div>
        </div>
        <div class="sensor-box">
          <label id="lblAmbHum">Ambient Humidity</label>
          <div class="value"><span id="ambH">--</span>%</div>
        </div>
        <div class="sensor-box">
          <label id="lblIntTemp">Internal Temp</label>
          <div class="value"><span id="intT">--</span>°C</div>
        </div>
        <div class="sensor-box">
          <label id="lblIntHum">Internal Humidity</label>
          <div class="value"><span id="intH">--</span>%</div>
        </div>
      </div>
      <div class="sensor-box" style="grid-column: 1/-1;">
        <label id="lblFan">Fan Speed</label>
        <div class="value"><span id="fan">--</span> RPM</div>
      </div>
    </div>

    <!-- Control Card -->
    <div class="card">
      <h2 id="controlTitle">Control Panel</h2>
      
      <div class="control-row">
        <label id="lblMode">Mode</label>
        <select id="modeSel" onchange="setMode()">
          <option value="1" data-en="Auto" data-bn="স্বয়ংক্রিয়">Auto</option>
          <option value="2" data-en="Manual" data-bn="ম্যানুয়াল">Manual</option>
          <option value="3" data-en="Timer" data-bn="টাইমার">Timer</option>
        </select>
      </div>

      <div class="control-row" id="priorityRow">
        <label id="lblPriority">Priority</label>
        <select id="prioSel" onchange="updatePriority()">
          <option value="Temperature" data-en="Temperature" data-bn="তাপমাত্রা">Temperature</option>
          <option value="Humidity" data-en="Humidity" data-bn="আর্দ্রতা">Humidity</option>
          <option value="None" data-en="Both" data-bn="উভয়">Both</option>
        </select>
      </div>

      <button id="relayBtn" class="btn btn-relay" onclick="toggleRelay()">
        <span id="relayText">Relay OFF</span>
      </button>

      <div id="timerBox" style="display:none;">
        <div class="timer-display">
          <label id="lblTimer" style="font-size:14px; color:#666;">Timer</label>
          <div class="time" id="elapsed">00:00:00</div>
          <div style="margin-top:8px; color:#666;">
            <span id="lblInterval">Interval:</span> <span id="timerVal">--</span> <span id="lblMin">min</span>
          </div>
        </div>
      </div>

      <div class="button-row">
        <a href="/config" class="btn btn-config" id="btnConfig">⚙️ <span id="configText">Configuration</span></a>
        <a href="/setup" class="btn btn-wifi" id="btnWifi">📶 <span id="wifiText">Wi-Fi Setup</span></a>
      </div>
    </div>
  </div>
</div>

<script>
let lang = '%LANG%';

// Bangla number conversion
function toBanglaNumber(num) {
  if (lang !== 'bn') return num.toString();
  const banglaDigits = ['০', '১', '২', '৩', '৪', '৫', '৬', '৭', '৮', '৯'];
  return num.toString().split('').map(d => {
    return d >= '0' && d <= '9' ? banglaDigits[parseInt(d)] : d;
  }).join('');
}

// Format number based on language
function formatNumber(num, decimals = 0) {
  const formatted = decimals > 0 ? num.toFixed(decimals) : num.toString();
  return toBanglaNumber(formatted);
}

const translations = {
  en: {
    title: 'Smart Cooler Dashboard',
    status: 'Online',
    sensorTitle: 'Sensor Readings',
    lblAmbTemp: 'Ambient Temp',
    lblAmbHum: 'Ambient Humidity',
    lblIntTemp: 'Internal Temp',
    lblIntHum: 'Internal Humidity',
    lblFan: 'Fan Speed',
    controlTitle: 'Control Panel',
    lblMode: 'Mode',
    lblPriority: 'Priority',
    relayOn: 'Relay ON',
    relayOff: 'Relay OFF',
    lblTimer: 'Timer',
    lblInterval: 'Interval:',
    lblMin: 'min',
    btnConfig: 'Configuration',
    configText: 'Configuration',
    wifiText: 'Wi-Fi Setup'
  },
  bn: {
    title: 'স্মার্ট কুলার ড্যাশবোর্ড',
    status: 'অনলাইন',
    sensorTitle: 'সেন্সর রিডিং',
    lblAmbTemp: 'পরিবেশের তাপমাত্রা',
    lblAmbHum: 'পরিবেশের আর্দ্রতা',
    lblIntTemp: 'অভ্যন্তরীণ তাপমাত্রা',
    lblIntHum: 'অভ্যন্তরীণ আর্দ্রতা',
    lblFan: 'ফ্যান গতি',
    controlTitle: 'নিয়ন্ত্রণ প্যানেল',
    lblMode: 'মোড',
    lblPriority: 'অগ্রাধিকার',
    relayOn: 'রিলে চালু',
    relayOff: 'রিলে বন্ধ',
    lblTimer: 'টাইমার',
    lblInterval: 'ব্যবধান:',
    lblMin: 'মিনিট',
    btnConfig: 'কনফিগারেশন',
    configText: 'কনফিগারেশন',
    wifiText: 'ওয়াই-ফাই সেটআপ'
  }
};

function setLang(l) {
  lang = l;
  document.querySelectorAll('.lang-btn').forEach(b => b.classList.remove('active'));
  event.target.classList.add('active');
  
  // Save language preference
  fetch(`/setlang?lang=${l}`);
  
  Object.keys(translations[lang]).forEach(key => {
    const el = document.getElementById(key);
    if (el) el.textContent = translations[lang][key];
  });
  
  // Update select options
  document.querySelectorAll('option').forEach(opt => {
    if (opt.dataset[lang]) opt.textContent = opt.dataset[lang];
  });
  
  updateRelayText();
}

let relayState = false;
let currentMode = 1;

async function fetchData() {
  try {
    const res = await fetch('/data');
    const d = await res.json();

    document.getElementById('ambT').textContent = formatNumber(d.ambient_temp_rt, 1);
    document.getElementById('intT').textContent = formatNumber(d.temp_rt, 1);
    document.getElementById('ambH').textContent = formatNumber(d.ambient_hum_rt, 0);
    document.getElementById('intH').textContent = formatNumber(d.hum_rt, 0);
    document.getElementById('fan').textContent = formatNumber(d.fan_speed, 0);

    currentMode = d.mode;
    relayState = d.relay_state;

    document.getElementById('modeSel').value = currentMode;
    document.getElementById('prioSel').value = d.priority;

    // Show/hide priority based on mode
    const priorityRow = document.getElementById('priorityRow');
    if (currentMode == 1) {
      priorityRow.style.display = 'flex';
    } else {
      priorityRow.style.display = 'none';
    }

    updateRelayText();

    const timerBox = document.getElementById('timerBox');
    if (currentMode == 3) {
      timerBox.style.display = 'block';
      document.getElementById('timerVal').textContent = formatNumber(d.timer_value, 0);
      const sec = d.timer_elapsed;
      const totalSec = d.timer_value * 60;
      const remainingSec = (sec < totalSec) ? (totalSec - sec) : 0;
      const hh = String(Math.floor(remainingSec / 3600)).padStart(2, '0');
      const mm = String(Math.floor((remainingSec % 3600) / 60)).padStart(2, '0');
      const ss = String(remainingSec % 60).padStart(2, '0');
      document.getElementById('elapsed').textContent = toBanglaNumber(`${hh}:${mm}:${ss}`);
    } else {
      timerBox.style.display = 'none';
    }
  } catch(e) {
    console.error('Fetch error:', e);
  }
}

function updateRelayText() {
  const btn = document.getElementById('relayBtn');
  const text = document.getElementById('relayText');
  text.textContent = relayState ? translations[lang].relayOn : translations[lang].relayOff;
  btn.className = relayState ? 'btn btn-relay on' : 'btn btn-relay';
}

async function toggleRelay() {
  relayState = !relayState;
  await fetch(`/relay?state=${relayState ? 1 : 0}`);
  fetchData();
}

async function setMode() {
  currentMode = parseInt(document.getElementById('modeSel').value);
  await fetch(`/mode?set=${currentMode}`);
  fetchData();
}

async function updatePriority() {
  const pr = document.getElementById('prioSel').value;
  await fetch(`/priority?set=${pr}`);
  fetchData();
}

setInterval(fetchData, 1000);
fetchData();

// Set initial language on load
window.addEventListener('DOMContentLoaded', () => {
  document.querySelectorAll('.lang-btn').forEach(b => {
    if (b.textContent.includes(lang === 'en' ? 'English' : 'বাংলা')) {
      b.classList.add('active');
    } else {
      b.classList.remove('active');
    }
  });
  if (lang === 'bn') {
    Object.keys(translations[lang]).forEach(key => {
      const el = document.getElementById(key);
      if (el) el.textContent = translations[lang][key];
    });
    document.querySelectorAll('option').forEach(opt => {
      if (opt.dataset[lang]) opt.textContent = opt.dataset[lang];
    });
  }
});
</script>
</body>
</html>
)rawliteral";

// ================================================
// WIFI SETUP PAGE
// ================================================
const char WIFI_SETUP_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>WiFi Setup</title>
<style>
* {margin:0; padding:0; box-sizing:border-box;}
body {
  font-family: 'Segoe UI', Arial, sans-serif;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  min-height: 100vh;
  padding: 20px;
  display: flex;
  align-items: center;
  justify-content: center;
}
.container {
  max-width: 500px;
  width: 100%;
}
.card {
  background: rgba(255,255,255,0.95);
  padding: 30px;
  border-radius: 20px;
  box-shadow: 0 20px 60px rgba(0,0,0,0.3);
}
.logo {
  font-size: 60px;
  text-align: center;
  margin-bottom: 20px;
}
h1 {
  color: #667eea;
  margin-bottom: 10px;
  font-size: 28px;
  text-align: center;
}
.subtitle {
  color: #666;
  margin-bottom: 30px;
  font-size: 14px;
  text-align: center;
}
.scan-btn {
  width: 100%;
  padding: 15px;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  border: none;
  border-radius: 10px;
  font-size: 16px;
  font-weight: 600;
  cursor: pointer;
  margin-bottom: 20px;
  transition: all 0.3s;
}
.scan-btn:hover {
  transform: translateY(-2px);
  box-shadow: 0 10px 25px rgba(102,126,234,0.3);
}
.network-list {
  max-height: 300px;
  overflow-y: auto;
  margin-bottom: 20px;
  border: 2px solid #e0e0e0;
  border-radius: 10px;
  display: none;
}
.network-item {
  padding: 15px;
  border-bottom: 1px solid #f0f0f0;
  cursor: pointer;
  transition: all 0.2s;
  display: flex;
  justify-content: space-between;
  align-items: center;
}
.network-item:hover {
  background: #f5f7fa;
}
.network-item.selected {
  background: #667eea;
  color: white;
}
.network-name {
  font-weight: 600;
  font-size: 16px;
}
.network-info {
  display: flex;
  gap: 10px;
  align-items: center;
  font-size: 12px;
}
.signal {
  display: flex;
  align-items: flex-end;
  gap: 2px;
  height: 16px;
}
.signal span {
  width: 3px;
  background: currentColor;
  border-radius: 2px;
}
.signal.strong span:nth-child(1) {height: 6px;}
.signal.strong span:nth-child(2) {height: 10px;}
.signal.strong span:nth-child(3) {height: 14px;}
.signal.strong span:nth-child(4) {height: 16px;}
.signal.medium span:nth-child(1) {height: 6px;}
.signal.medium span:nth-child(2) {height: 10px;}
.signal.medium span:nth-child(3) {height: 14px; opacity: 0.3;}
.signal.medium span:nth-child(4) {height: 16px; opacity: 0.3;}
.signal.weak span:nth-child(1) {height: 6px;}
.signal.weak span:nth-child(2) {height: 10px; opacity: 0.3;}
.signal.weak span:nth-child(3) {height: 14px; opacity: 0.3;}
.signal.weak span:nth-child(4) {height: 16px; opacity: 0.3;}
.lock-icon {
  font-size: 14px;
}
.form-group {
  margin-bottom: 20px;
  display: none;
}
.form-group label {
  display: block;
  margin-bottom: 8px;
  font-weight: 600;
  color: #333;
  font-size: 14px;
}
.form-group input {
  width: 100%;
  padding: 15px;
  border: 2px solid #e0e0e0;
  border-radius: 10px;
  font-size: 16px;
  transition: all 0.3s;
}
.form-group input:focus {
  outline: none;
  border-color: #667eea;
  box-shadow: 0 0 0 3px rgba(102,126,234,0.1);
}
.btn {
  width: 100%;
  padding: 15px;
  border: none;
  border-radius: 10px;
  font-size: 16px;
  font-weight: 600;
  cursor: pointer;
  transition: all 0.3s;
  margin-top: 10px;
}
.btn-primary {
  background: linear-gradient(135deg, #43e97b 0%, #38f9d7 100%);
  color: white;
  display: none;
}
.btn-primary:hover {
  transform: translateY(-2px);
  box-shadow: 0 10px 25px rgba(67,233,123,0.3);
}
.btn-secondary {
  background: #f5f7fa;
  color: #667eea;
  border: 2px solid #667eea;
}
.alert {
  padding: 15px;
  border-radius: 10px;
  margin-top: 20px;
  display: none;
}
.alert.success {
  background: #d4edda;
  color: #155724;
}
.alert.error {
  background: #f8d7da;
  color: #721c24;
}
.spinner {
  border: 3px solid #f3f3f3;
  border-top: 3px solid #667eea;
  border-radius: 50%;
  width: 40px;
  height: 40px;
  animation: spin 1s linear infinite;
  margin: 20px auto;
  display: none;
}
@keyframes spin {
  0% { transform: rotate(0deg); }
  100% { transform: rotate(360deg); }
}
.manual-entry {
  text-align: center;
  margin-top: 15px;
}
.manual-link {
  color: #667eea;
  text-decoration: none;
  font-size: 14px;
  font-weight: 600;
}
.manual-link:hover {
  text-decoration: underline;
}
</style>
</head>
<body>

<div class="container">
  <div class="card">
    <div class="logo">📶</div>
    <h1>WiFi Setup</h1>
    <p class="subtitle">Configure your Smart Cooler</p>
    
    <button class="scan-btn" onclick="scanNetworks()">🔍 Scan WiFi Networks</button>
    
    <div class="network-list" id="networkList"></div>
    
    <div class="spinner" id="spinner"></div>
    
    <form id="wifiForm">
      <div class="form-group" id="ssidGroup">
        <label>Network Name (SSID)</label>
        <input type="text" id="ssid" required placeholder="Selected network">
      </div>
      
      <div class="form-group" id="passwordGroup">
        <label>Password</label>
        <input type="password" id="password" required placeholder="Enter WiFi password">
      </div>
      
      <button type="submit" class="btn btn-primary" id="connectBtn">💾 Connect</button>
    </form>
    
    <div class="manual-entry">
      <a href="#" class="manual-link" onclick="showManualEntry(); return false;">📝 Enter manually</a>
    </div>
    
    <button type="button" class="btn btn-secondary" onclick="skipSetup()">Skip</button>
    
    <div class="alert" id="alert"></div>
  </div>
</div>

<script>
let selectedSSID = '';
let networks = [];

async function scanNetworks() {
  const spinner = document.getElementById('spinner');
  const networkList = document.getElementById('networkList');
  const scanBtn = document.querySelector('.scan-btn');
  
  scanBtn.disabled = true;
  scanBtn.textContent = '⏳ Scanning...';
  spinner.style.display = 'block';
  networkList.style.display = 'none';
  
  try {
    const res = await fetch('/scan');
    networks = await res.json();
    
    spinner.style.display = 'none';
    scanBtn.disabled = false;
    scanBtn.textContent = '🔄 Rescan';
    
    if (networks.length === 0) {
      networkList.innerHTML = '<div style="padding:20px; text-align:center; color:#666;">No networks found</div>';
      networkList.style.display = 'block';
      return;
    }
    
    networkList.innerHTML = '';
    networks.forEach(network => {
      const item = document.createElement('div');
      item.className = 'network-item';
      item.onclick = () => selectNetwork(network.ssid, network.encryption > 0);
      
      let signalClass = 'weak';
      if (network.rssi > -60) signalClass = 'strong';
      else if (network.rssi > -75) signalClass = 'medium';
      
      item.innerHTML = `
        <div class="network-name">${network.ssid}</div>
        <div class="network-info">
          <div class="signal ${signalClass}">
            <span></span><span></span><span></span><span></span>
          </div>
          ${network.encryption > 0 ? '<span class="lock-icon">🔒</span>' : '<span class="lock-icon">🔓</span>'}
        </div>
      `;
      
      networkList.appendChild(item);
    });
    
    networkList.style.display = 'block';
  } catch(e) {
    spinner.style.display = 'none';
    scanBtn.disabled = false;
    scanBtn.textContent = '🔍 Scan WiFi Networks';
    showAlert('Error scanning networks', 'error');
  }
}

function selectNetwork(ssid, needsPassword) {
  selectedSSID = ssid;
  document.getElementById('ssid').value = ssid;
  
  // Highlight selected
  document.querySelectorAll('.network-item').forEach(item => {
    item.classList.remove('selected');
    if (item.querySelector('.network-name').textContent === ssid) {
      item.classList.add('selected');
    }
  });
  
  // Show form
  document.getElementById('ssidGroup').style.display = 'block';
  document.getElementById('passwordGroup').style.display = needsPassword ? 'block' : 'none';
  document.getElementById('connectBtn').style.display = 'block';
  
  if (needsPassword) {
    document.getElementById('password').focus();
  }
}

function showManualEntry() {
  document.getElementById('networkList').style.display = 'none';
  document.getElementById('ssid').value = '';
  document.getElementById('ssid').readOnly = false;
  document.getElementById('ssidGroup').style.display = 'block';
  document.getElementById('passwordGroup').style.display = 'block';
  document.getElementById('connectBtn').style.display = 'block';
  document.getElementById('ssid').focus();
}

function showAlert(message, type) {
  const alert = document.getElementById('alert');
  alert.className = 'alert ' + type;
  alert.textContent = message;
  alert.style.display = 'block';
  setTimeout(() => alert.style.display = 'none', 5000);
}

document.getElementById('wifiForm').addEventListener('submit', async (e) => {
  e.preventDefault();
  
  const ssid = document.getElementById('ssid').value;
  const password = document.getElementById('password').value;
  const spinner = document.getElementById('spinner');
  const connectBtn = document.getElementById('connectBtn');
  
  connectBtn.disabled = true;
  connectBtn.textContent = '⏳ Connecting...';
  spinner.style.display = 'block';
  
  try {
    const res = await fetch('/setwifi', {
      method: 'POST',
      headers: {'Content-Type': 'application/json'},
      body: JSON.stringify({ssid, password})
    });
    
    const data = await res.json();
    
    spinner.style.display = 'none';
    
    if (data.success) {
      showAlert('✓ Connected! Restarting...', 'success');
      setTimeout(() => {
        window.location.href = '/';
      }, 3000);
    } else {
      connectBtn.disabled = false;
      connectBtn.textContent = '💾 Connect';
      showAlert('✗ Connection failed: ' + data.message, 'error');
    }
  } catch(e) {
    spinner.style.display = 'none';
    connectBtn.disabled = false;
    connectBtn.textContent = '💾 Connect';
    showAlert('✗ Error: ' + e.message, 'error');
  }
});

function skipSetup() {
  window.location.href = '/';
}

// Auto-scan on load
window.onload = () => {
  setTimeout(scanNetworks, 500);
};
</script>
</body>
</html>
)rawliteral";

// ================================================
// CONFIG PAGE
// ================================================
const char CONFIG_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Configuration</title>
<style>
* {margin:0; padding:0; box-sizing:border-box;}
body {
  font-family: 'Segoe UI', Arial, sans-serif;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  min-height: 100vh;
  padding: 20px;
}
.container {
  max-width: 600px;
  margin: 0 auto;
}
.header {
  background: rgba(255,255,255,0.95);
  padding: 20px;
  border-radius: 15px;
  box-shadow: 0 8px 32px rgba(0,0,0,0.1);
  margin-bottom: 20px;
  display: flex;
  justify-content: space-between;
  align-items: center;
}
.header h1 {
  color: #667eea;
  font-size: 24px;
}
.lang-switch {
  display: flex;
  gap: 10px;
}
.lang-btn {
  padding: 6px 12px;
  border: 2px solid #667eea;
  background: white;
  color: #667eea;
  border-radius: 8px;
  cursor: pointer;
  font-weight: 600;
  transition: all 0.3s;
  font-size: 14px;
}
.lang-btn.active {
  background: #667eea;
  color: white;
}
.card {
  background: rgba(255,255,255,0.95);
  padding: 30px;
  border-radius: 15px;
  box-shadow: 0 8px 32px rgba(0,0,0,0.1);
}
.card h2 {
  color: #667eea;
  margin-bottom: 25px;
  font-size: 20px;
}
.form-group {
  margin-bottom: 20px;
}
.form-group label {
  display: block;
  margin-bottom: 8px;
  font-weight: 600;
  color: #333;
}
.form-group input {
  width: 100%;
  padding: 12px;
  border: 2px solid #e0e0e0;
  border-radius: 8px;
  font-size: 16px;
  transition: border 0.3s;
}
.form-group input:focus {
  outline: none;
  border-color: #667eea;
}
.form-row {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 15px;
}
.btn {
  width: 100%;
  padding: 15px;
  border: none;
  border-radius: 10px;
  font-size: 16px;
  font-weight: 600;
  cursor: pointer;
  transition: all 0.3s;
  margin-top: 10px;
}
.btn-save {
  background: linear-gradient(135deg, #43e97b 0%, #38f9d7 100%);
  color: white;
}
.btn-back {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  text-decoration: none;
  display: block;
  text-align: center;
}
.btn:hover {
  transform: translateY(-2px);
  box-shadow: 0 6px 20px rgba(0,0,0,0.15);
}
.section {
  border-bottom: 2px solid #f0f0f0;
  padding-bottom: 20px;
  margin-bottom: 20px;
}
.section:last-child {
  border-bottom: none;
  padding-bottom: 0;
  margin-bottom: 0;
}
.section h3 {
  color: #667eea;
  font-size: 16px;
  margin-bottom: 15px;
}
.alert {
  padding: 15px;
  border-radius: 8px;
  margin-top: 15px;
  display: none;
}
.alert.success {
  background: #d4edda;
  color: #155724;
  border: 1px solid #c3e6cb;
}
@media (max-width: 600px) {
  .form-row {grid-template-columns: 1fr;}
  .header {flex-direction: column; gap: 15px;}
}
</style>
</head>
<body>

<div class="container">
  <div class="header">
    <h1 id="title">Configuration</h1>
    <div class="lang-switch">
      <button class="lang-btn active" onclick="setLang('en')">EN</button>
      <button class="lang-btn" onclick="setLang('bn')">বাংলা</button>
    </div>
  </div>

  <div class="card">
    <h2 id="configTitle">System Settings</h2>
    
    <form id="configForm">
      <div class="section">
        <h3 id="tempTitle">Temperature Thresholds</h3>
        <div class="form-row">
          <div class="form-group">
            <label id="lblUpperTemp">Upper Temp (°C)</label>
            <input type="number" step="0.5" id="upperT" required>
          </div>
          <div class="form-group">
            <label id="lblLowerTemp">Lower Temp (°C)</label>
            <input type="number" step="0.5" id="lowerT" required>
          </div>
        </div>
      </div>

      <div class="section">
        <h3 id="humTitle">Humidity Thresholds</h3>
        <div class="form-row">
          <div class="form-group">
            <label id="lblUpperHum">Upper Humidity (%)</label>
            <input type="number" step="1" id="upperH" required>
          </div>
          <div class="form-group">
            <label id="lblLowerHum">Lower Humidity (%)</label>
            <input type="number" step="1" id="lowerH" required>
          </div>
        </div>
      </div>

      <div class="section">
        <h3 id="timerTitle">Timer Settings</h3>
        <div class="form-group">
          <label id="lblTimerMin">Timer Interval (minutes)</label>
          <input type="number" step="1" id="timerCfg" required>
        </div>
      </div>

      <button type="submit" class="btn btn-save" id="btnSave">💾 Save Configuration</button>
      <div class="alert success" id="successAlert">
        <span id="successMsg">✓ Configuration saved successfully!</span>
      </div>
    </form>

    <a href="/" class="btn btn-back" id="btnBack">← Back to Dashboard</a>
  </div>
</div>

<script>
let lang = 'en';
const translations = {
  en: {
    title: 'Configuration',
    configTitle: 'System Settings',
    tempTitle: 'Temperature Thresholds',
    lblUpperTemp: 'Upper Temp (°C)',
    lblLowerTemp: 'Lower Temp (°C)',
    humTitle: 'Humidity Thresholds',
    lblUpperHum: 'Upper Humidity (%)',
    lblLowerHum: 'Lower Humidity (%)',
    timerTitle: 'Timer Settings',
    lblTimerMin: 'Timer Interval (minutes)',
    btnSave: '💾 Save Configuration',
    btnBack: '← Back to Dashboard',
    successMsg: '✓ Configuration saved successfully!'
  },
  bn: {
    title: 'কনফিগারেশন',
    configTitle: 'সিস্টেম সেটিংস',
    tempTitle: 'তাপমাত্রার সীমা',
    lblUpperTemp: 'উচ্চ তাপমাত্রা (°সে)',
    lblLowerTemp: 'নিম্ন তাপমাত্রা (°সে)',
    humTitle: 'আর্দ্রতার সীমা',
    lblUpperHum: 'উচ্চ আর্দ্রতা (%)',
    lblLowerHum: 'নিম্ন আর্দ্রতা (%)',
    timerTitle: 'টাইমার সেটিংস',
    lblTimerMin: 'টাইমার ব্যবধান (মিনিট)',
    btnSave: '💾 কনফিগারেশন সংরক্ষণ',
    btnBack: '← ড্যাশবোর্ডে ফিরুন',
    successMsg: '✓ কনফিগারেশন সফলভাবে সংরক্ষিত!'
  }
};

function setLang(l) {
  lang = l;
  document.querySelectorAll('.lang-btn').forEach(b => b.classList.remove('active'));
  event.target.classList.add('active');
  
  Object.keys(translations[lang]).forEach(key => {
    const el = document.getElementById(key);
    if (el) el.textContent = translations[lang][key];
  });
}

async function loadConfig() {
  try {
    const res = await fetch('/data');
    const d = await res.json();
    
    document.getElementById('upperT').value = d.upper_temp_threshold;
    document.getElementById('lowerT').value = d.lower_temp_threshold;
    document.getElementById('upperH').value = d.upper_hum_threshold;
    document.getElementById('lowerH').value = d.lower_hum_threshold;
    document.getElementById('timerCfg').value = d.timer_value;
  } catch(e) {
    console.error('Load error:', e);
  }
}

document.getElementById('configForm').addEventListener('submit', async (e) => {
  e.preventDefault();
  
  const cfg = {
    upper_temp_threshold: parseFloat(upperT.value),
    lower_temp_threshold: parseFloat(lowerT.value),
    upper_hum_threshold: parseFloat(upperH.value),
    lower_hum_threshold: parseFloat(lowerH.value),
    timer_value: parseInt(timerCfg.value)
  };
  
  try {
    await fetch('/save', {
      method: 'POST',
      headers: {'Content-Type':'application/json'},
      body: JSON.stringify(cfg)
    });
    
    const alert = document.getElementById('successAlert');
    alert.style.display = 'block';
    setTimeout(() => alert.style.display = 'none', 3000);
  } catch(e) {
    console.error('Save error:', e);
  }
});

loadConfig();
</script>
</body>
</html>
)rawliteral";

// ================================================
// SERVER LOGIC
// ================================================
void startWebServer(){
  // WiFi Setup page
  server.on("/setup", HTTP_GET, [](AsyncWebServerRequest *req){
    String page = WIFI_SETUP_page;
    page.replace("%IP%", getIPAddress());
    req->send(200, "text/html", page);
  });

  // WiFi scan endpoint
  server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request){
    String networks = scanWiFiNetworks();
    request->send(200, "application/json", networks);
  });

  // Set WiFi credentials
  server.on("/setwifi", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
  [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    JsonDocument doc;
    if(deserializeJson(doc, data)) {
      request->send(400, "application/json", "{\"success\":false,\"message\":\"Invalid JSON\"}");
      return;
    }
    
    String ssid = String((const char*)doc["ssid"]);
    String password = String((const char*)doc["password"]);
    
    // Save credentials
    saveWiFiCredentials(ssid.c_str(), password.c_str());
    
    // Try to connect
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
      delay(100);
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      request->send(200, "application/json", "{\"success\":true,\"ip\":\"" + WiFi.localIP().toString() + "\"}");
      delay(1000);
      ESP.restart(); // Restart to apply new WiFi settings
    } else {
      request->send(200, "application/json", "{\"success\":false,\"message\":\"Connection timeout\"}");
    }
  });

  // Main dashboard
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req){
    // Redirect to setup if in AP mode
    if (getWiFiMode() == WIFI_AP_MODE) {
      req->redirect("/setup");
      return;
    }
    
    String page = MAIN_page;
    String lang = getLanguage();
    page.replace("%LANG%", lang);
    req->send(200, "text/html", page);
  });

  // Config page
  server.on("/config", HTTP_GET, [](AsyncWebServerRequest *req){
    String page = CONFIG_page;
    String lang = getLanguage();
    page.replace("%LANG%", lang);
    req->send(200, "text/html", page);
  });

  // Live data endpoint
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    JsonDocument doc;
    doc["ambient_temp_rt"] = ambient_temp;
    doc["ambient_hum_rt"]  = ambient_hum;
    doc["temp_rt"]         = internal_temp;
    doc["hum_rt"]          = internal_hum;
    doc["fan_speed"]       = fan_speed;
    doc["relay_state"]     = relay_state;
    doc["mode"]            = mode;
    doc["priority"]        = priority;
    doc["timer_value"]     = timer_value;
    doc["upper_temp_threshold"] = upper_temp_threshold;
    doc["lower_temp_threshold"] = lower_temp_threshold;
    doc["upper_hum_threshold"]  = upper_hum_threshold;
    doc["lower_hum_threshold"]  = lower_hum_threshold;
    
    // Calculate elapsed time for timer mode
    unsigned long elapsedSec = 0;
    if (mode == 3) {
      elapsedSec = (millis() - coolerTimerStart) / 1000UL;
    }
    doc["timer_elapsed"] = elapsedSec;
    
    String out; 
    serializeJson(doc, out);
    request->send(200, "application/json", out);
  });

  // Relay toggle
  server.on("/relay", HTTP_GET, [](AsyncWebServerRequest *request){
    if(request->hasParam("state")){
      String state = request->getParam("state")->value();
      relay_state = (state == "1");
      manual_override = true;
      digitalWrite(11, relay_state ? HIGH : LOW);
      Serial.printf("[WEB] Relay manually set -> %s\n", relay_state ? "ON" : "OFF");
    }
    request->send(200, "text/plain", "OK");
  });

  // Mode change
  server.on("/mode", HTTP_GET, [](AsyncWebServerRequest *request){
    if(request->hasParam("set")){
      int oldMode = mode;
      mode = request->getParam("set")->value().toInt();
      
      // Clear manual override when switching modes
      if (oldMode != mode) {
        manual_override = false;
      }
      
      // Initialize timer mode
      if (mode == 3) {
        coolerTimerStart = millis();
      }
      
      saveConfiguration(); // Save to preferences
      Serial.printf("[WEB] Mode -> %d (saved)\n", mode);
    }
    request->send(200, "text/plain", "Mode Updated");
  });

  // Priority change
  server.on("/priority", HTTP_GET, [](AsyncWebServerRequest *request){
    if(request->hasParam("set")){
      priority = request->getParam("set")->value();
      saveConfiguration(); // Save to preferences
      Serial.printf("[WEB] Priority -> %s (saved)\n", priority.c_str());
    }
    request->send(200, "text/plain", "Priority Updated");
  });

  // Set language preference
  server.on("/setlang", HTTP_GET, [](AsyncWebServerRequest *request){
    if(request->hasParam("lang")){
      String lang = request->getParam("lang")->value();
      saveLanguage(lang.c_str());
      Serial.printf("[WEB] Language -> %s (saved)\n", lang.c_str());
    }
    request->send(200, "text/plain", "Language Saved");
  });

  // Save configuration
  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
  [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, data);
    
    if(err) {
      Serial.printf("[WEB] JSON parse error: %s\n", err.c_str());
      request->send(400, "text/plain", "Invalid JSON"); 
      return;
    }
    
    // Update thresholds
    upper_temp_threshold = doc["upper_temp_threshold"];
    lower_temp_threshold = doc["lower_temp_threshold"];
    upper_hum_threshold  = doc["upper_hum_threshold"];
    lower_hum_threshold  = doc["lower_hum_threshold"];
    timer_value          = doc["timer_value"];
    
    // Save to flash memory
    saveConfiguration();
    
    Serial.println("[WEB] Configuration saved to flash.");
    Serial.printf("  Temp: %.1f - %.1f°C\n", lower_temp_threshold, upper_temp_threshold);
    Serial.printf("  Hum: %.0f - %.0f%%\n", lower_hum_threshold, upper_hum_threshold);
    Serial.printf("  Timer: %d min\n", timer_value);
    
    request->send(200, "text/plain", "Saved");
  });

  server.begin();
  Serial.println("[HTTP] Web server started.");
  Serial.printf("[HTTP] Access: http://%s/\n", getIPAddress().c_str());
}