import json
import re

from django.conf import settings
from django.http import JsonResponse
from django.shortcuts import render
from django.utils import timezone
from django.views.decorators.csrf import csrf_exempt

from .models import Mission, Telemetry


def _extract_payload(data):
    if isinstance(data, (bytes, bytearray)):
        text = data.decode('utf-8', errors='ignore')
    else:
        text = str(data)

    text = text.strip()
    if not text:
        return {}

    if text.startswith('{'):
        return json.loads(text)

    if 'Broadcast:' in text:
        text = text.split('Broadcast:', 1)[-1].strip()

    match = re.search(r'\{.*\}', text, re.DOTALL)
    if match:
        candidate = match.group(0)
        try:
            return json.loads(candidate)
        except json.JSONDecodeError:
            return {}

    try:
        return json.loads(text)
    except json.JSONDecodeError:
        return {}


def _discover_esp8266_ws_url(request):
    configured = getattr(settings, 'ESP8266_WS_URL', None)
    if configured:
        return configured

    host = request.get_host().split(':', 1)[0].strip()
    if host in {'localhost', '127.0.0.1', '0.0.0.0'}:
        return 'ws://192.168.0.101:81/'

    parts = host.split('.')
    if len(parts) == 4 and all(part.isdigit() for part in parts):
        first = int(parts[0])
        second = int(parts[1])
        if first == 10 or (first == 172 and 16 <= second <= 31) or (first == 192 and second == 168):
            prefix = '.'.join(parts[:3])
            return f'ws://{prefix}.101:81/'

    return 'ws://192.168.0.101:81/'


def _dashboard_context(request):
    latest = Telemetry.objects.order_by('-timestamp').first()
    mission = Mission.objects.order_by('-id').first()

    telemetry = {
        'altitude': latest.altitude if latest else 0.0,
        'speed': latest.speed if latest else 0.0,
        'distance': latest.distance if latest else 0.0,
        'rssi': latest.rssi if latest else None,
        'battery': latest.battery if latest else None,
        'battery_percent': latest.battery if latest and latest.battery is not None else None,
        'voltage': latest.voltage if latest else None,
        'current': latest.current if latest else None,
        'temperature': latest.temperature if latest else None,
        'sats': latest.sats if latest else None,
        'fix3d': latest.fix3d if latest else False,
        'latitude': latest.latitude if latest else None,
        'longitude': latest.longitude if latest else None,
        'uptime': latest.uptime if latest else 0,
        'device_id': latest.device_id if latest else 'LDOS-001',
    }

    return {
        'telemetry': telemetry,
        'telemetry_socket_url': _discover_esp8266_ws_url(request),
        'now': timezone.now(),
        'mission': {
            'name': mission.name if mission else 'Survey Mission',
            'time': '00:15:42',
            'waypoints_count': mission.waypoints_count if mission else 5,
            'current_wp': mission.current_wp if mission else 4,
        },
        'sensors': {
            'imu': 'NORMAL',
            'gps': 'NORMAL',
            'camera': 'NORMAL',
            'esc': 'NORMAL',
        },
        'payload': {
            'CAMERA': 'ACTIVE',
            'LIDAR': 'ACTIVE',
            'PAYLOAD': 'READY',
        },
        'warnings': [],
    }


def home(request):
    context = _dashboard_context(request)
    return render(request, 'telemetry/telemetry.html', context)


def telemetry_data(request):
    if request.method == 'GET':
        context = _dashboard_context(request)
        context['telemetry_data'] = Telemetry.objects.order_by('-timestamp')[:10]
        return render(request, 'telemetry/telemetry.html', context)
    return JsonResponse({'error': 'GET required'}, status=405)


def latest_telemetry_json(request):
    if request.method != 'GET':
        return JsonResponse({'error': 'GET required'}, status=405)

    latest = Telemetry.objects.order_by('-timestamp').first()
    if not latest:
        return JsonResponse({})

    return JsonResponse({
        'device_id': latest.device_id,
        'timestamp': latest.timestamp.isoformat() if latest.timestamp else None,
        'altitude': latest.altitude,
        'speed': latest.speed,
        'distance': latest.distance,
        'rssi': latest.rssi,
        'battery': latest.battery,
        'battery_percent': latest.battery,
        'voltage': latest.voltage,
        'current': latest.current,
        'temperature': latest.temperature,
        'sats': latest.sats,
        'fix3d': latest.fix3d,
        'latitude': float(latest.latitude) if latest.latitude is not None else None,
        'longitude': float(latest.longitude) if latest.longitude is not None else None,
        'uptime': latest.uptime,
    })

# DATA RECEIVED FROM ESP8266
@csrf_exempt
def esp8266(request):
    if request.method == 'POST':
        try:
            payload = _extract_payload(request.body)
            if not payload:
                return JsonResponse({'status': 'invalid_payload'}, status=400)

            Telemetry.objects.create(
                device_id=payload.get('device_id', 'LDOS-001'),
                timestamp=payload.get('timestamp'),
                temperature=payload.get('temperature'),
                altitude=payload.get('altitude'),
                battery=payload.get('battery'),
                speed=payload.get('speed'),
                distance=payload.get('distance'),
                latitude=payload.get('latitude'),
                longitude=payload.get('longitude'),
                rssi=payload.get('rssi'),
                voltage=payload.get('voltage'),
                current=payload.get('current'),
                sats=payload.get('sats'),
                uptime=payload.get('uptime')
            )
            return JsonResponse({'status': 'ok'})
        except Exception:
            return JsonResponse({'status': 'invalid_payload'}, status=400)

    return JsonResponse({'error': 'POST required'}, status=405)
    