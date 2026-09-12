#include "html_pages.h"
#include "config.h"

// Generate mode options HTML based on enabled modes
String getModeOptionsHTML() {
  String html = "";
  
  #if ENABLE_AUTO_MODE
    html += "<option value=\"1\" data-en=\"Auto\" data-bn=\"স্বয়ংক্রিয়\">Auto</option>";
  #endif
  
  #if ENABLE_MANUAL_MODE
    html += "<option value=\"2\" data-en=\"Manual\" data-bn=\"ম্যানুয়াল\">Manual</option>";
  #endif
  
  #if ENABLE_TIMER_MODE
    html += "<option value=\"3\" data-en=\"Timer\" data-bn=\"টাইমার\">Timer</option>";
  #endif
  
  return html;
}

// Generate priority options HTML based on enabled priorities
String getPriorityOptionsHTML() {
  String html = "";
  
  #if ENABLE_PRIORITY_TEMPERATURE
    html += "<option value=\"Temperature\" data-en=\"Temperature\" data-bn=\"তাপমাত্রা\">Temperature</option>";
  #endif
  
  #if ENABLE_PRIORITY_HUMIDITY
    html += "<option value=\"Humidity\" data-en=\"Humidity\" data-bn=\"আর্দ্রতা\">Humidity</option>";
  #endif
  
  #if ENABLE_PRIORITY_BOTH
    html += "<option value=\"None\" data-en=\"Both\" data-bn=\"উভয়\">Both</option>";
  #endif
  
  return html;
}

// WiFi Setup Page
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
      <a href="#" class="manual-link" onclick="showManualEntry(); return false;">🔧 Enter manually</a>
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
  
  document.querySelectorAll('.network-item').forEach(item => {
    item.classList.remove('selected');
    if (item.querySelector('.network-name').textContent === ssid) {
      item.classList.add('selected');
    }
  });
  
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
      showAlert('✔ Connected! Restarting...', 'success');
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

window.onload = () => {
  setTimeout(scanNetworks, 500);
};
</script>
</body>
</html>
)rawliteral";

// Configuration Page
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
.btn-calibrate {
  background: linear-gradient(135deg, #fa709a 0%, #fee140 100%);
  color: white;
}
.btn-danger {
  background: linear-gradient(135deg, #f5576c 0%, #d63447 100%);
  color: white;
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

      <div class="section">
        <h3 id="calibrateTitle">Sensor Calibration</h3>

        <!-- Auto Calibration -->
        <div style="margin-bottom: 20px; padding: 15px; background: #f5f7fa; border-radius: 8px;">
          <p id="calibrateDesc" style="font-size: 13px; color: #666; margin-bottom: 15px;">
            Place both sensors next to each other and click to auto-calibrate.
          </p>
          <button type="button" class="btn btn-calibrate" id="btnCalibrate" onclick="calibrateSensors()">
            🔧 <span id="btnCalibrateText">Auto Calibrate</span>
          </button>
          <div class="alert success" id="calibrateAlert" style="display: none; margin-top: 15px;">
            <div id="calibrateResult"></div>
          </div>
        </div>

        <!-- Manual Calibration -->
        <div style="margin-bottom: 20px; padding: 15px; background: #f5f7fa; border-radius: 8px;">
          <p id="manualCalibDesc" style="font-size: 13px; color: #666; margin-bottom: 15px;">
            Or manually set calibration offsets:
          </p>
          <div class="form-row">
            <div class="form-group">
              <label id="lblTempOffset">Temperature Offset (°C)</label>
              <input type="number" step="0.1" id="manualTempOffset" placeholder="e.g., -1.7">
            </div>
            <div class="form-group">
              <label id="lblHumOffset">Humidity Offset (%)</label>
              <input type="number" step="0.1" id="manualHumOffset" placeholder="e.g., -14.0">
            </div>
          </div>
          <button type="button" class="btn btn-save" id="btnSetOffsets" onclick="setOffsets()" style="margin-top: 10px;">
            ✓ <span id="btnSetOffsetsText">Apply Offsets</span>
          </button>
          <div class="alert success" id="setOffsetsAlert" style="display: none; margin-top: 15px;">
            <span id="setOffsetsResult">✔ Offsets applied!</span>
          </div>
        </div>

        <!-- Reset Calibration -->
        <div style="padding: 15px; background: #ffe5e5; border-radius: 8px;">
          <p id="resetCalibDesc" style="font-size: 13px; color: #666; margin-bottom: 15px;">
            Reset calibration to defaults (remove all offsets):
          </p>
          <button type="button" class="btn btn-danger" id="btnResetCalibrate" onclick="resetCalibration()">
            🔄 <span id="btnResetCalibrateText">Reset Calibration</span>
          </button>
        </div>
      </div>

      <button type="submit" class="btn btn-save" id="btnSave">💾 Save Configuration</button>
      <div class="alert success" id="successAlert">
        <span id="successMsg">✔ Configuration saved successfully!</span>
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
    calibrateTitle: 'Sensor Calibration',
    calibrateDesc: 'Place both sensors next to each other and click to auto-calibrate.',
    btnCalibrateText: 'Auto Calibrate',
    manualCalibDesc: 'Or manually set calibration offsets:',
    lblTempOffset: 'Temperature Offset (°C)',
    lblHumOffset: 'Humidity Offset (%)',
    btnSetOffsetsText: 'Apply Offsets',
    resetCalibDesc: 'Reset calibration to defaults (remove all offsets):',
    btnResetCalibrateText: 'Reset Calibration',
    btnSave: '💾 Save Configuration',
    btnBack: '← Back to Dashboard',
    successMsg: '✔ Configuration saved successfully!'
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
    calibrateTitle: 'সেন্সর ক্যালিব্রেশন',
    calibrateDesc: 'উভয় সেন্সর একসাথে রাখুন এবং স্বয়ংক্রিয় ক্যালিব্রেশনের জন্য ক্লিক করুন।',
    btnCalibrateText: 'স্বয়ংক্রিয় ক্যালিব্রেট',
    manualCalibDesc: 'অথবা ম্যানুয়ালি ক্যালিব্রেশন অফসেট সেট করুন:',
    lblTempOffset: 'তাপমাত্রা অফসেট (°সে)',
    lblHumOffset: 'আর্দ্রতা অফসেট (%)',
    btnSetOffsetsText: 'অফসেট প্রয়োগ করুন',
    resetCalibDesc: 'ক্যালিব্রেশন ডিফল্টে রিসেট করুন (সমস্ত অফসেট সরান):',
    btnResetCalibrateText: 'ক্যালিব্রেশন রিসেট করুন',
    btnSave: '💾 কনফিগারেশন সংরক্ষণ',
    btnBack: '← ড্যাশবোর্ডে ফিরুন',
    successMsg: '✔ কনফিগারেশন সফলভাবে সংরক্ষিত!'
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
  
  fetch(`/setlang?lang=${l}`);
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

async function calibrateSensors() {
  const btn = document.getElementById('btnCalibrate');
  const alert = document.getElementById('calibrateAlert');
  const result = document.getElementById('calibrateResult');

  btn.disabled = true;
  btn.style.opacity = '0.6';

  try {
    const res = await fetch('/calibrate');
    const data = await res.json();

    if (data.success) {
      const msg = `<strong>✔ Calibration Complete!</strong><br><br>` +
        `<strong>Ambient:</strong> ${data.ambient_temp.toFixed(2)}°C, ${data.ambient_hum.toFixed(2)}%<br>` +
        `<strong>Internal:</strong> ${data.internal_temp.toFixed(2)}°C, ${data.internal_hum.toFixed(2)}%<br><br>` +
        `<strong>Applied Offsets:</strong><br>` +
        `Temperature: ${data.offset_temp > 0 ? '+' : ''}${data.offset_temp.toFixed(2)}°C<br>` +
        `Humidity: ${data.offset_hum > 0 ? '+' : ''}${data.offset_hum.toFixed(2)}%`;

      result.innerHTML = msg;
      alert.style.display = 'block';
      alert.style.backgroundColor = '#43e97b';
      setTimeout(() => alert.style.display = 'none', 5000);
    } else {
      result.innerHTML = '✗ Calibration failed: ' + (data.error || 'Unknown error');
      alert.style.backgroundColor = '#f5576c';
      alert.style.display = 'block';
    }
  } catch(e) {
    result.innerHTML = '✗ Error: ' + e.message;
    alert.style.backgroundColor = '#f5576c';
    alert.style.display = 'block';
  } finally {
    btn.disabled = false;
    btn.style.opacity = '1';
  }
}

async function setOffsets() {
  const tempOffset = parseFloat(document.getElementById('manualTempOffset').value);
  const humOffset = parseFloat(document.getElementById('manualHumOffset').value);

  if (isNaN(tempOffset) || isNaN(humOffset)) {
    alert('Please enter valid numbers for both offsets');
    return;
  }

  const btn = document.getElementById('btnSetOffsets');
  const alert = document.getElementById('setOffsetsAlert');
  const result = document.getElementById('setOffsetsResult');

  btn.disabled = true;
  btn.style.opacity = '0.6';

  try {
    const res = await fetch(`/setoffsets?temp=${tempOffset}&hum=${humOffset}`);
    const data = await res.json();

    if (data.success) {
      result.innerHTML = `✔ Offsets applied!<br>Temperature: ${tempOffset}°C<br>Humidity: ${humOffset}%`;
      alert.style.display = 'block';
      alert.style.backgroundColor = '#43e97b';
      setTimeout(() => alert.style.display = 'none', 3000);
    } else {
      result.innerHTML = '✗ Failed to apply offsets';
      alert.style.backgroundColor = '#f5576c';
      alert.style.display = 'block';
    }
  } catch(e) {
    result.innerHTML = '✗ Error: ' + e.message;
    alert.style.backgroundColor = '#f5576c';
    alert.style.display = 'block';
  } finally {
    btn.disabled = false;
    btn.style.opacity = '1';
  }
}

async function resetCalibration() {
  if (!confirm('Are you sure you want to reset all calibration offsets to 0?')) {
    return;
  }

  try {
    const res = await fetch('/resetcalibrate');
    const data = await res.json();

    if (data.success) {
      document.getElementById('manualTempOffset').value = '';
      document.getElementById('manualHumOffset').value = '';

      const alert = document.getElementById('setOffsetsAlert');
      const result = document.getElementById('setOffsetsResult');
      result.innerHTML = '✔ Calibration reset to defaults (offsets = 0)';
      alert.style.display = 'block';
      alert.style.backgroundColor = '#43e97b';
      setTimeout(() => alert.style.display = 'none', 3000);
    }
  } catch(e) {
    alert('Error resetting calibration: ' + e.message);
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

// Main Dashboard Page
const char MAIN_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Smart Cooler</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}body{font-family:Arial,sans-serif;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);min-height:100vh;padding:20px}.container{max-width:1200px;margin:0 auto}.header{background:rgba(255,255,255,0.95);padding:20px;border-radius:15px;box-shadow:0 8px 32px rgba(0,0,0,0.1);margin-bottom:20px;display:flex;justify-content:space-between;align-items:center;flex-wrap:wrap}.header h1{color:#667eea;font-size:28px}.status-badge{display:inline-block;padding:6px 12px;border-radius:20px;font-size:12px;font-weight:600;margin-left:10px}.status-online{background:#43e97b;color:white}.status-offline{background:#f5576c;color:white}.lang-switch{display:flex;gap:10px}.lang-btn{padding:8px 16px;border:2px solid #667eea;background:white;color:#667eea;border-radius:8px;cursor:pointer;font-weight:600;transition:all 0.3s;font-size:14px}.lang-btn.active{background:#667eea;color:white}.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(300px,1fr));gap:20px;margin-bottom:20px}.card{background:rgba(255,255,255,0.95);padding:25px;border-radius:15px;box-shadow:0 8px 32px rgba(0,0,0,0.1)}.card h2{color:#667eea;margin-bottom:20px;font-size:22px;border-bottom:2px solid #667eea;padding-bottom:10px}.sensor-grid{display:grid;grid-template-columns:1fr 1fr;gap:15px;margin-bottom:15px}.sensor-box{background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:white;padding:15px;border-radius:10px;text-align:center}.sensor-box label{font-size:12px;opacity:0.9;display:block;margin-bottom:5px}.sensor-box .value{font-size:24px;font-weight:bold}.threshold-info{background:#f5f7fa;padding:12px;border-radius:8px;margin-top:10px;font-size:13px;color:#555}.threshold-info .info-row{display:flex;justify-content:space-between;margin:4px 0;padding:4px 0}.threshold-info .info-row .label{font-weight:600;color:#667eea}.threshold-info .info-row .value{font-family:monospace;font-weight:bold}.control-row{display:flex;justify-content:space-between;align-items:center;margin:15px 0;padding:10px;background:#f5f7fa;border-radius:8px}select{padding:8px 15px;border:2px solid #667eea;border-radius:8px;background:white;color:#333;font-size:14px;cursor:pointer;min-width:150px}.btn{padding:12px 30px;border:none;border-radius:10px;font-size:16px;font-weight:600;cursor:pointer;transition:all 0.3s;width:100%;margin-top:10px}.btn-relay{background:linear-gradient(135deg,#f093fb 0%,#f5576c 100%);color:white;font-size:28px;padding:40px;height:120px;display:flex;align-items:center;justify-content:center}.btn-relay.on{background:linear-gradient(135deg,#4facfe 0%,#00f2fe 100%)}.btn-config{background:linear-gradient(135deg,#43e97b 0%,#38f9d7 100%);color:white;text-decoration:none;display:flex;align-items:center;justify-content:center;gap:10px;font-size:20px;padding:20px;height:70px}.button-row{display:grid;grid-template-columns:1fr 1fr;gap:10px;margin-top:10px}.timer-display{background:#f5f7fa;padding:15px;border-radius:10px;text-align:center;margin-top:15px}.timer-display .time{font-size:32px;font-weight:bold;color:#667eea;font-family:monospace}
</style>
</head>
<body>
<div class="container">
  <div class="header">
    <div>
      <h1 id="title">Smart Cooler</h1>
      <span class="status-badge status-online" id="stat">● <span id="status">Connecting...</span></span>
    </div>
    <div class="lang-switch">
      <button class="lang-btn active" onclick="setLang('en')">English</button>
      <button class="lang-btn" onclick="setLang('bn')">বাংলা</button>
    </div>
  </div>
  <div class="grid">
    <div class="card">
      <h2 id="sensorTitle">Sensor Readings</h2>
      <div class="sensor-grid">
        <div class="sensor-box"><label id="lblAmbTemp">Ambient Temp</label><div class="value"><span id="ambT">--</span>°C</div></div>
        <div class="sensor-box"><label id="lblAmbHum">Ambient Humidity</label><div class="value"><span id="ambH">--</span>%</div></div>
        <div class="sensor-box"><label id="lblIntTemp">Internal Temp</label><div class="value"><span id="intT">--</span>°C</div></div>
        <div class="sensor-box"><label id="lblIntHum">Internal Humidity</label><div class="value"><span id="intH">--</span>%</div></div>
      </div>
      <div class="threshold-info" id="thresholdInfo">
        <div class="info-row"><span class="label" id="lblThreshTemp">Temp Thresholds:</span><span class="value" id="threshTemp">--</span></div>
        <div class="info-row"><span class="label" id="lblThreshHum">Hum Thresholds:</span><span class="value" id="threshHum">--</span></div>
        <div class="info-row"><span class="label" id="lblCalcTemp">Active Temp:</span><span class="value" id="calcTemp">--</span></div>
        <div class="info-row"><span class="label" id="lblCalcHum">Active Hum:</span><span class="value" id="calcHum">--</span></div>
        <div class="info-row" id="timerRow" style="display:none"><span class="label" id="lblTimerInterval">Timer Interval:</span><span class="value" id="timerInterval">--</span></div>
      </div>
    </div>
    <div class="card">
      <h2 id="controlTitle">Control Panel</h2>
      <div class="control-row"><label id="lblMode">Mode</label><select id="mode" onchange="setMode()">%MODE_OPTIONS%</select></div>
      <div class="control-row" id="prioRow"><label id="lblPriority">Priority</label><select id="prio" onchange="setPrio()">%PRIORITY_OPTIONS%</select></div>
      <button id="relay" class="btn btn-relay" onclick="toggle()"><span id="relayTxt">Relay OFF</span></button>
      <div id="timer" style="display:none"><div class="timer-display"><label id="lblTimer">Timer</label><div class="time" id="time">00:00:00</div><div style="margin-top:8px;color:#666;"><span id="lblInterval">Interval:</span> <span id="interval">--</span> <span id="lblMin">min</span></div></div></div>
      <div class="button-row">
        <a href="/config" class="btn btn-config" id="btnConfig">⚙️ <span id="configText">Config</span></a>
        <a href="/setup" class="btn btn-config" id="btnWifi">📶 <span id="wifiText">WiFi</span></a>
      </div>
    </div>
  </div>
</div>
<script>
let ws,data={},lang='%LANG%';
const tr={en:{title:'Smart Cooler',status:'Online',sensorTitle:'Sensor Readings',lblAmbTemp:'Ambient Temp',lblAmbHum:'Ambient Humidity',lblIntTemp:'Internal Temp',lblIntHum:'Internal Humidity',lblThreshTemp:'Temp Thresholds:',lblThreshHum:'Hum Thresholds:',lblCalcTemp:'Active Temp:',lblCalcHum:'Active Hum:',lblTimerInterval:'Timer Interval:',controlTitle:'Control Panel',lblMode:'Mode',lblPriority:'Priority',relayOn:'Relay ON',relayOff:'Relay OFF',lblTimer:'Timer',lblInterval:'Interval:',lblMin:'min',configText:'Config',wifiText:'WiFi'},bn:{title:'স্মার্ট ক্লার',status:'অনলাইন',sensorTitle:'সেন্সর রিডিং',lblAmbTemp:'পরিবেশের তাপমাত্রা',lblAmbHum:'পরিবেশের আর্দ্রতা',lblIntTemp:'অভ্যন্তরীণ তাপমাত্রা',lblIntHum:'অভ্যন্তরীণ আর্দ্রতা',lblThreshTemp:'তাপমাত্রা সীমা:',lblThreshHum:'আর্দ্রতা সীমা:',lblCalcTemp:'সক্রিয় তাপমাত্রা:',lblCalcHum:'সক্রিয় আর্দ্রতা:',lblTimerInterval:'টাইমার ব্যবধান:',controlTitle:'নিয়ন্ত্রণ প্যানেল',lblMode:'মোড',lblPriority:'অগ্রাধিকার',relayOn:'রিলে চালু',relayOff:'রিলে বন্ধ',lblTimer:'টাইমার',lblInterval:'ব্যবধান:',lblMin:'মিনিট',configText:'কনফিগারেশন',wifiText:'ওয়াই-ফাই'}};
function toBangla(n){if(lang!=='bn')return n.toString();const d=['০','১','২','৩','৪','৫','৬','৭','৮','৯'];return n.toString().split('').map(c=>c>='0'&&c<='9'?d[parseInt(c)]:c).join('')}
function setLang(l){lang=l;document.querySelectorAll('.lang-btn').forEach(b=>b.classList.remove('active'));event.target.classList.add('active');fetch(`/setlang?lang=${l}`);Object.keys(tr[lang]).forEach(k=>{const e=document.getElementById(k);if(e)e.textContent=tr[lang][k]});document.querySelectorAll('option').forEach(o=>{if(o.dataset[lang])o.textContent=o.dataset[lang]});updateRelayText();updateThresholdDisplay()}
function conn(){ws=new WebSocket(`ws://${location.host}/ws`);ws.onopen=()=>{document.getElementById('status').textContent=tr[lang].status;document.getElementById('stat').className='status-badge status-online'};ws.onclose=()=>{document.getElementById('status').textContent='Offline';document.getElementById('stat').className='status-badge status-offline';setTimeout(conn,3000)};ws.onmessage=(e)=>{data=JSON.parse(e.data);update()}}
function updateThresholdDisplay(){document.getElementById('threshTemp').textContent=toBangla(data.lower_temp_threshold?.toFixed(1)||'--')+' - '+toBangla(data.upper_temp_threshold?.toFixed(1)||'--')+'°C';document.getElementById('threshHum').textContent=toBangla(data.lower_hum_threshold?.toFixed(0)||'--')+' - '+toBangla(data.upper_hum_threshold?.toFixed(0)||'--')+'%';document.getElementById('calcTemp').textContent=toBangla(data.calculated_lower_temp?.toFixed(1)||'--')+' - '+toBangla(data.calculated_upper_temp?.toFixed(1)||'--')+'°C';document.getElementById('calcHum').textContent=toBangla(data.calculated_lower_hum?.toFixed(0)||'--')+' - '+toBangla(data.calculated_upper_hum?.toFixed(0)||'--')+'%';const timerRow=document.getElementById('timerRow');if(data.mode==3){timerRow.style.display='flex';document.getElementById('timerInterval').textContent=toBangla(data.timer_value||'--')+' '+(lang=='bn'?'মিনিট':'min')}else{timerRow.style.display='none'}}
function update(){document.getElementById('ambT').textContent=toBangla(data.ambient_temp_rt.toFixed(1));document.getElementById('intT').textContent=toBangla(data.temp_rt.toFixed(1));document.getElementById('ambH').textContent=toBangla(data.ambient_hum_rt.toFixed(0));document.getElementById('intH').textContent=toBangla(data.hum_rt.toFixed(0));document.getElementById('mode').value=data.mode;document.getElementById('prio').value=data.priority;document.getElementById('prioRow').style.display=data.mode==1?'flex':'none';updateThresholdDisplay();updateRelayText();if(data.mode==3){document.getElementById('timer').style.display='block';document.getElementById('interval').textContent=toBangla(data.timer_value);const s=data.timer_elapsed,t=data.timer_value*60,r=(s<t)?(t-s):0,h=String(Math.floor(r/3600)).padStart(2,'0'),m=String(Math.floor((r%3600)/60)).padStart(2,'0'),ss=String(r%60).padStart(2,'0');document.getElementById('time').textContent=toBangla(`${h}:${m}:${ss}`)}else{document.getElementById('timer').style.display='none'}}
function updateRelayText(){document.getElementById('relayTxt').textContent=data.relay_state?tr[lang].relayOn:tr[lang].relayOff;document.getElementById('relay').className=data.relay_state?'btn btn-relay on':'btn btn-relay'}
async function toggle(){await fetch(`/relay?state=${data.relay_state?0:1}`)}
async function setMode(){await fetch(`/mode?set=${document.getElementById('mode').value}`)}
async function setPrio(){await fetch(`/priority?set=${document.getElementById('prio').value}`)}
window.onload=()=>{if(lang==='bn'){document.querySelectorAll('.lang-btn').forEach(b=>{if(b.textContent.includes('বাংলা'))b.classList.add('active');else b.classList.remove('active')});Object.keys(tr[lang]).forEach(k=>{const e=document.getElementById(k);if(e)e.textContent=tr[lang][k]});document.querySelectorAll('option').forEach(o=>{if(o.dataset[lang])o.textContent=o.dataset[lang]})}conn()}
</script>
</body>
</html>
)rawliteral";