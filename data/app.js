// WebSocket connection
let ws = null;
let reconnectInterval = null;
let pm25Gauge = null;
let historyData = {
    pm1: [], pm25: [], pm4: [], pm10: [],
    temperature: [], humidity: [], voc: [], nox: []
};

// Initialize PM2.5 gauge with canvas-gauges
function initGauge() {
    pm25Gauge = new RadialGauge({
        renderTo: 'pm25-gauge',
        width: 300,
        height: 300,
        units: 'µg/m³',
        minValue: 0,
        maxValue: 100,
        majorTicks: ['0', '15', '35', '55', '100'],
        minorTicks: 5,
        strokeTicks: true,
        highlights: [
            { from: 0, to: 15, color: '#48bb78' },
            { from: 15, to: 35, color: '#ecc94b' },
            { from: 35, to: 55, color: '#ed8936' },
            { from: 55, to: 100, color: '#e53e3e' }
        ],
        colorPlate: 'transparent',
        borderShadowWidth: 0,
        borders: false,
        needleType: 'arrow',
        needleWidth: 3,
        needleCircleSize: 8,
        needleCircleOuter: true,
        needleCircleInner: false,
        animationDuration: 500,
        animationRule: 'linear',
        colorNeedle: '#ffffff',
        colorNeedleEnd: '#ffffff',
        colorNeedleCircleOuter: '#ffffff',
        colorNeedleCircleOuterEnd: '#ffffff',
        valueBox: true,
        valueBoxStroke: 0,
        valueBoxWidth: 0,
        valueBoxBorderRadius: 0,
        colorValueBoxRect: 'transparent',
        colorValueBoxRectEnd: 'transparent',
        colorValueBoxBackground: 'transparent',
        valueText: '',
        valueTextShadow: false,
        valueDec: 1,
        valueInt: 2,
        fontValue: 'Arial',
        fontValueSize: 48,
        fontValueWeight: 'bold',
        fontValueStyle: 'normal',
        colorValueText: '#e2e8f0',
        colorValueTextShadow: 'transparent',
        ticksAngle: 180,
        startAngle: 90,
        fontNumbersSize: 20,
        fontNumbersWeight: 'normal',
        fontNumbersColor: '#e2e8f0',
        colorMajorTicks: '#cbd5e0',
        colorMinorTicks: '#4a5568',
        colorUnits: '#a0aec0',
        fontUnitsSize: 16
    });

    pm25Gauge.draw();
}// Connect to WebSocket
function connectWebSocket() {
    const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
    const wsUrl = `${protocol}//${window.location.hostname}/ws`;

    ws = new WebSocket(wsUrl);

    ws.onopen = () => {
        console.log('WebSocket connected');
        updateConnectionStatus(true);
        if (reconnectInterval) {
            clearInterval(reconnectInterval);
            reconnectInterval = null;
        }
        // Request initial data
        ws.send('getHistory');
        ws.send('getStatus');
    };

    ws.onmessage = (event) => {
        try {
            const data = JSON.parse(event.data);
            handleWebSocketMessage(data);
        } catch (e) {
            console.error('Error parsing WebSocket message:', e);
        }
    };

    ws.onerror = (error) => {
        console.error('WebSocket error:', error);
        updateConnectionStatus(false);
    };

    ws.onclose = () => {
        console.log('WebSocket disconnected');
        updateConnectionStatus(false);
        // Attempt reconnection
        if (!reconnectInterval) {
            reconnectInterval = setInterval(() => {
                console.log('Attempting to reconnect...');
                connectWebSocket();
            }, 5000);
        }
    };
}

// Handle incoming WebSocket messages
function handleWebSocketMessage(data) {
    if (data.type === 'current') {
        updateCurrentReadings(data);
        updateLastUpdateTime();
    } else if (data.history) {
        loadHistoryData(data.history);
    } else if (data.type === 'status') {
        updateSystemStatus(data);
    }
}

// Update current sensor readings
function updateCurrentReadings(data) {
    // Update values
    document.getElementById('pm1-value').textContent = data.pm1.toFixed(1);
    document.getElementById('pm4-value').textContent = data.pm4.toFixed(1);
    document.getElementById('pm10-value').textContent = data.pm10.toFixed(1);
    document.getElementById('temp-value').textContent = data.temperature.toFixed(1);
    document.getElementById('humidity-value').textContent = data.humidity.toFixed(1);
    document.getElementById('voc-value').textContent = Math.round(data.voc);
    document.getElementById('nox-value').textContent = Math.round(data.nox);

    // Update PM2.5 gauge
    if (pm25Gauge) {
        pm25Gauge.value = data.pm25;
    }

    // Update air quality label
    updateAirQualityLabel(data.quality);

    // Add to history and update sparklines
    addToHistory(data);
    updateAllSparklines();
}

// Update air quality label
function updateAirQualityLabel(quality) {
    const label = document.getElementById('air-quality-label');
    const qualityMap = {
        'GOOD': { text: 'Good', class: 'good' },
        'MODERATE': { text: 'Moderate', class: 'moderate' },
        'UNHEALTHY_SENSITIVE': { text: 'Unhealthy for Sensitive Groups', class: 'unhealthy-sensitive' },
        'UNHEALTHY': { text: 'Unhealthy', class: 'unhealthy' }
    };

    const info = qualityMap[quality] || { text: quality, class: '' };
    label.textContent = info.text;
    label.className = `quality-label ${info.class}`;
}

// Add data to history buffers
function addToHistory(data) {
    const maxHistory = 60;

    historyData.pm1.push(data.pm1);
    historyData.pm25.push(data.pm25);
    historyData.pm4.push(data.pm4);
    historyData.pm10.push(data.pm10);
    historyData.temperature.push(data.temperature);
    historyData.humidity.push(data.humidity);
    historyData.voc.push(data.voc);
    historyData.nox.push(data.nox);

    // Keep only last 60 readings
    Object.keys(historyData).forEach(key => {
        if (historyData[key].length > maxHistory) {
            historyData[key].shift();
        }
    });
}

// Load initial history data
function loadHistoryData(history) {
    historyData = {
        pm1: [], pm25: [], pm4: [], pm10: [],
        temperature: [], humidity: [], voc: [], nox: []
    };

    history.forEach(reading => {
        if (reading.pm25 !== undefined) historyData.pm25.push(reading.pm25);
        if (reading.temperature !== undefined) historyData.temperature.push(reading.temperature);
        if (reading.humidity !== undefined) historyData.humidity.push(reading.humidity);
        if (reading.voc !== undefined) historyData.voc.push(reading.voc);
    });

    updateAllSparklines();
}

// Update all sparkline charts
function updateAllSparklines() {
    drawSparkline('pm1-sparkline', historyData.pm1, '#4299e1');
    drawSparkline('pm4-sparkline', historyData.pm4, '#9f7aea');
    drawSparkline('pm10-sparkline', historyData.pm10, '#ed64a6');
    drawSparkline('temp-sparkline', historyData.temperature, '#ed8936');
    drawSparkline('humidity-sparkline', historyData.humidity, '#4299e1');
    drawSparkline('voc-sparkline', historyData.voc, '#ecc94b');
    drawSparkline('nox-sparkline', historyData.nox, '#e53e3e');
}

// Draw sparkline chart on canvas
function drawSparkline(canvasId, data, color) {
    const canvas = document.getElementById(canvasId);
    if (!canvas || data.length === 0) return;

    const ctx = canvas.getContext('2d');
    const width = canvas.width;
    const height = canvas.height;

    // Clear canvas
    ctx.clearRect(0, 0, width, height);

    // Find min/max for scaling
    const min = Math.min(...data);
    const max = Math.max(...data);
    const range = max - min || 1;

    // Draw line
    ctx.beginPath();
    ctx.strokeStyle = color;
    ctx.lineWidth = 2;

    data.forEach((value, index) => {
        const x = (index / (data.length - 1)) * width;
        const y = height - ((value - min) / range) * (height - 4) - 2;

        if (index === 0) {
            ctx.moveTo(x, y);
        } else {
            ctx.lineTo(x, y);
        }
    });

    ctx.stroke();
}

// Update system status
function updateSystemStatus(data) {
    if (data.uptime !== undefined) {
        const hours = Math.floor(data.uptime / 3600);
        const minutes = Math.floor((data.uptime % 3600) / 60);
        document.getElementById('uptime-value').textContent = `${hours}h ${minutes}m`;
    }

    if (data.clients !== undefined) {
        document.getElementById('clients-value').textContent = data.clients;
    }

    if (data.freeHeap !== undefined && data.heapSize !== undefined) {
        const usedPercent = ((data.heapSize - data.freeHeap) / data.heapSize * 100).toFixed(0);
        document.getElementById('memory-value').textContent = `${usedPercent}%`;
    }
}

// Update connection status badge
function updateConnectionStatus(connected) {
    const badge = document.getElementById('connection-status');
    if (connected) {
        badge.textContent = 'Connected';
        badge.className = 'status-badge connected';
    } else {
        badge.textContent = 'Disconnected';
        badge.className = 'status-badge disconnected';
    }
}

// Update last update timestamp
function updateLastUpdateTime() {
    const now = new Date();
    const timeStr = now.toLocaleTimeString();
    document.getElementById('last-update').textContent = `Last update: ${timeStr}`;
}

// Sidebar menu functionality
function initSidebar() {
    const hamburger = document.getElementById('hamburger');
    const sidebar = document.getElementById('sidebar');
    const overlay = document.getElementById('sidebar-overlay');
    const closeBtn = document.getElementById('close-sidebar');
    const resetBtn = document.getElementById('reset-device');

    function openSidebar() {
        sidebar.classList.add('open');
        overlay.classList.add('show');
    }

    function closeSidebar() {
        sidebar.classList.remove('open');
        overlay.classList.remove('show');
    }

    hamburger.addEventListener('click', openSidebar);
    closeBtn.addEventListener('click', closeSidebar);
    overlay.addEventListener('click', closeSidebar);

    // Reset device functionality
    resetBtn.addEventListener('click', (e) => {
        e.preventDefault();
        if (confirm('Are you sure you want to reset the device? This will restart the ESP32.')) {
            fetch('/api/reset', { method: 'POST' })
                .then(response => {
                    if (response.ok) {
                        alert('Device reset initiated. The ESP32 will restart.');
                        closeSidebar();
                        updateConnectionStatus(false);
                    } else {
                        alert('Failed to reset device.');
                    }
                })
                .catch(error => {
                    console.error('Reset error:', error);
                    alert('Error communicating with device.');
                });
        }
    });
}

// Initialize on page load
document.addEventListener('DOMContentLoaded', () => {
    console.log('Air Quality Dashboard initialized');
    initGauge();
    initSidebar();
    connectWebSocket();

    // Request status update every 10 seconds
    setInterval(() => {
        if (ws && ws.readyState === WebSocket.OPEN) {
            ws.send('getStatus');
        }
    }, 10000);
});
