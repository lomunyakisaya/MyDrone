// telemetry.js - lightweight frontend for the LDOS dashboard
(function(){
    const el = id=>document.getElementById(id);
    const endpoint = '/api/telemetry/latest'; // adjust to your backend

    function updateUI(data){
        if(!data) return;
        if(el('altitude')) el('altitude').textContent = data.altitude ? data.altitude + ' m' : '--';
        if(el('speed')) el('speed').textContent = data.speed ? data.speed + ' m/s' : '--';
        if(el('distance')) el('distance').textContent = data.distance ? data.distance + ' km' : '--';
        if(el('rssi')) el('rssi').textContent = data.rssi ? data.rssi + ' dBm' : '--';
        if(el('battery')) el('battery').textContent = (data.battery_percent!=null) ? data.battery_percent + '%' : '--';
        if(el('voltage')) el('voltage').textContent = data.voltage ? data.voltage + ' V' : '--';
        if(el('current')) el('current').textContent = data.current ? data.current + ' A' : '--';
        if(el('temperature')) el('temperature').textContent = data.temperature ? data.temperature + ' °C' : '--';
        if(el('sats')) el('sats').textContent = data.sats!=null ? data.sats : '--';
        if(el('fix')) el('fix').textContent = data.fix3d ? 'YES' : 'NO';
        if(el('lat')) el('lat').textContent = data.latitude || '--';
        if(el('lon')) el('lon').textContent = data.longitude || '--';
        if(el('live-image') && data.live_image_url){ el('live-image').src = data.live_image_url }
    }

    async function fetchTelemetry(){
        try{
            const res = await fetch(endpoint, {cache:'no-store'});
            if(!res.ok) throw new Error('no data');
            const json = await res.json();
            updateUI(json);
        }catch(e){
            // fallback: simulate data for demo
            const sim = {
                altitude: (Math.random()*200).toFixed(1),
                speed: (Math.random()*20).toFixed(1),
                distance: (Math.random()*5).toFixed(2),
                rssi: -40 - Math.floor(Math.random()*40),
                battery_percent: 60 + Math.floor(Math.random()*40),
                voltage: (22 + Math.random()).toFixed(2),
                current: (1 + Math.random()*6).toFixed(2),
                temperature: (20 + Math.random()*15).toFixed(1),
                sats: 8 + Math.floor(Math.random()*8),
                fix3d: true,
                latitude: ( -2.09576 + Math.random()*0.01).toFixed(6),
                longitude: (37.00827 + Math.random()*0.01).toFixed(6),
                live_image_url: '/static/telemetry/img/camera.svg'
            };
            updateUI(sim);
        }
    }

    // polling
    setInterval(fetchTelemetry, 3000);
    document.addEventListener('DOMContentLoaded', ()=>{
        fetchTelemetry();
        const takeoff = document.getElementById('takeoff');
        if(takeoff) takeoff.addEventListener('click', ()=>{alert('Takeoff triggered (demo)')});
        const auto = document.getElementById('auto-mode');
        if(auto) auto.addEventListener('click', ()=>{auto.classList.toggle('active'); alert('Toggled AUTO MODE (demo)')});
    });
})();
