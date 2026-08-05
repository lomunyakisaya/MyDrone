import json

from django.http import JsonResponse
from django.shortcuts import render
from django.utils import timezone
from django.views.decorators.csrf import csrf_exempt

from .models import Mission, Telemetry


def _dashboard_context():
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
    context = _dashboard_context()
    return render(request, 'telemetry/telemetry.html', context)


def telemetry_data(request):
    if request.method == 'GET':
        context = _dashboard_context()
        context['telemetry_data'] = Telemetry.objects.order_by('-timestamp')[:10]
        return render(request, 'telemetry/telemetry.html', context)
    return JsonResponse({'error': 'GET required'}, status=405)

#DATA RECEIVED FROM ESP8266
@csrf_exempt
def esp8266(request):
    if request.method == 'POST':
        data = json.loads(request.body)
        Telemetry.objects.create(
            device_id=data.get('device_id', 'LDOS-001'),
            timestamp=data.get('timestamp'),
            temperature=data.get('temperature'),
            altitude=data.get('altitude'),
            battery=data.get('battery'),
            speed=data.get('speed'),
            distance=data.get('distance'),
            latitude=data.get('latitude'),
            longitude=data.get('longitude'),
            rssi=data.get('rssi'),
            voltage=data.get('voltage'),
            current=data.get('current'),
            sats=data.get('sats'),
            uptime=data.get('uptime')
        )
        return JsonResponse({'status': 'ok'})

    return JsonResponse({'error': 'POST required'}, status=405)
    