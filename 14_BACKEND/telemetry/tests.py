import json

from django.test import TestCase
from django.urls import reverse

from .models import Telemetry


class Esp8266TelemetryTests(TestCase):
    def test_accepts_prefixed_json_payload(self):
        payload = (
            'Broadcast: {"device_id":"LDOS-001","temperature":34.7,"altitude":23,'
            '"battery":78,"timestamp":"2023-07-15 12:34:56","speed":211,'
            '"distance":120.3,"latitude":37.7749,"longitude":-122.4194,'
            '"rssi":-65,"voltage":3.7,"current":0.5,"sats":7,"uptime":123456}'
        )

        response = self.client.post(
            reverse('esp8266'),
            payload,
            content_type='application/json',
        )

        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.json()['status'], 'ok')
        self.assertTrue(Telemetry.objects.filter(device_id='LDOS-001').exists())

        latest = Telemetry.objects.latest('id')
        self.assertEqual(latest.temperature, 34.7)
        self.assertEqual(latest.battery, 78)
        self.assertEqual(latest.altitude, 23)
