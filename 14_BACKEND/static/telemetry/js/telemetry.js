// telemetry.js - live WebSocket telemetry for the LDOS dashboard
(function(){
    const el = id => document.getElementById(id);
    const configuredSocketUrl = window.LDOS_CONFIG && window.LDOS_CONFIG.websocketUrl;
    const socketUrl = configuredSocketUrl || 'ws://192.168.0.106:81/';
    let socket = null;
    let lastPacketAt = null;
    let lastSeenTimestamp = null;

    function setConnectionState(state, connected) {
        const badge = el('connection-state');
        const dot = el('status-dot');
        if (badge) badge.textContent = state;
        if (dot) {
            dot.classList.toggle('connected', connected);
        }
    }

    function updateClock() {
        const clock = el('clock');
        if (!clock) return;
        const now = new Date();
        clock.textContent = now.toLocaleTimeString([], { hour12: false }) + ' UTC';
    }

    function formatValue(value, suffix, digits = 2) {
        if (value === null || value === undefined || value === '') return '--';
        const number = Number(value);
        if (Number.isNaN(number)) return String(value);
        return `${number.toFixed(digits)}${suffix}`;
    }

    function normalizePayload(payload) {
        if (!payload) return null;

        if (typeof payload === 'object') return payload;

        if (typeof payload === 'string') {
            const trimmed = payload.trim();
            if (!trimmed) return null;

            try {
                return JSON.parse(trimmed);
            } catch (error) {
                const result = {};
                trimmed.split(/[\n,;]+/).forEach(part => {
                    const item = part.trim();
                    if (!item) return;
                    const separatorIndex = item.indexOf(':');
                    if (separatorIndex < 0) return;

                    const key = item.slice(0, separatorIndex).trim();
                    let value = item.slice(separatorIndex + 1).trim();

                    if ((value.startsWith('"') && value.endsWith('"')) || (value.startsWith("'") && value.endsWith("'"))) {
                        value = value.slice(1, -1);
                    } else if (value === 'true') {
                        value = true;
                    } else if (value === 'false') {
                        value = false;
                    } else if (/^-?\d+(\.\d+)?$/.test(value)) {
                        value = Number(value);
                    }

                    result[key] = value;
                });

                return Object.keys(result).length ? result : null;
            }
        }

        return null;
    }

    function updateUI(data) {
        if (!data) return;

        const batteryValue = data.battery_percent ?? data.battery;
        const latitude = data.latitude;
        const longitude = data.longitude;
        const incomingTimestamp = data.timestamp ? new Date(data.timestamp).getTime() : Date.now();

        if (lastSeenTimestamp && incomingTimestamp <= lastSeenTimestamp) {
            return;
        }

        lastSeenTimestamp = incomingTimestamp;
        lastPacketAt = incomingTimestamp;
        setConnectionState('LIVE', true);

        if (el('altitude')) el('altitude').textContent = data.altitude != null ? formatValue(data.altitude, ' m', 1) : '--';
        if (el('speed')) el('speed').textContent = data.speed != null ? formatValue(data.speed, ' m/s', 1) : '--';
        if (el('distance')) el('distance').textContent = data.distance != null ? formatValue(data.distance, ' km', 2) : '--';
        if (el('rssi')) el('rssi').textContent = data.rssi != null ? `${data.rssi} dBm` : '--';
        if (el('battery')) el('battery').textContent = batteryValue != null ? `${batteryValue}%` : '--';
        if (el('voltage')) el('voltage').textContent = data.voltage != null ? formatValue(data.voltage, ' V', 2) : '--';
        if (el('current')) el('current').textContent = data.current != null ? formatValue(data.current, ' A', 2) : '--';
        if (el('temperature')) el('temperature').textContent = data.temperature != null ? formatValue(data.temperature, ' °C', 1) : '--';
        if (el('sats')) el('sats').textContent = data.sats != null ? data.sats : '--';
        if (el('fix')) el('fix').textContent = data.fix3d ? 'YES' : 'NO';
        if (el('lat')) el('lat').textContent = latitude != null ? Number(latitude).toFixed(6) : '--';
        if (el('lon')) el('lon').textContent = longitude != null ? Number(longitude).toFixed(6) : '--';
        if (el('live-image') && data.live_image_url) el('live-image').src = data.live_image_url;
    }

    async function refreshFromBackend() {
        try {
            const response = await fetch('/telemetry/latest/');
            if (!response.ok) return;
            const payload = await response.json();
            if (!payload || !Object.keys(payload).length) return;

            updateUI(payload);
        } catch (error) {
            console.warn('Telemetry poll failed', error);
        }
    }

    function connectWebSocket() {
        if (!window.WebSocket) return;
        if (socket) socket.close();

        socket = new WebSocket(socketUrl);

        socket.addEventListener('open', () => {
            console.log('Telemetry WebSocket connected to', socketUrl);
            setConnectionState('CONNECTED', true);
        });

        socket.addEventListener('message', event => {
            console.log('Telemetry message received:', event.data);
            const payload = normalizePayload(event.data);
            if (payload) updateUI(payload);
        });

        socket.addEventListener('close', event => {
            console.log('Telemetry WebSocket closed', event.code, event.reason);
            setConnectionState('OFFLINE', false);
            setTimeout(connectWebSocket, 3000);
        });

        socket.addEventListener('error', event => {
            console.error('Telemetry WebSocket error', event);
            setConnectionState('ERROR', false);
        });
    }

    document.addEventListener('DOMContentLoaded', () => {
        updateClock();
        setInterval(updateClock, 1000);
        setInterval(() => {
            if (!lastPacketAt) {
                setConnectionState('OFFLINE', false);
                return;
            }

            if (Date.now() - lastPacketAt > 7000) {
                setConnectionState('OFFLINE', false);
            }
        }, 1000);

        connectWebSocket();
        refreshFromBackend();
        setInterval(refreshFromBackend, 1000);

        const takeoff = document.getElementById('takeoff');
        if (takeoff) takeoff.addEventListener('click', () => {
            alert('Takeoff triggered (demo)');
        });

        const auto = document.getElementById('auto-mode');
        if (auto) auto.addEventListener('click', () => {
            auto.classList.toggle('active');
            alert('Toggled AUTO MODE (demo)');
        });
    });
})();
