from django.db import models


# Basic models for telemetry dashboard display
class Telemetry(models.Model):
    device_id = models.CharField(max_length=100, default='LDOS-001')
    timestamp = models.DateTimeField(auto_now_add=True)

    # flight state
    altitude = models.FloatField(default=0.0)
    speed = models.FloatField(default=0.0)
    distance = models.FloatField(default=0.0)

    # radio / power
    rssi = models.IntegerField(null=True, blank=True)
    battery_percent = models.IntegerField(null=True, blank=True)
    voltage = models.FloatField(null=True, blank=True)
    current = models.FloatField(null=True, blank=True)

    # environment / sensors
    temperature = models.FloatField(null=True, blank=True)
    sats = models.IntegerField(null=True, blank=True)
    fix3d = models.BooleanField(default=False)

    # location
    latitude = models.DecimalField(max_digits=10, decimal_places=6, null=True, blank=True)
    longitude = models.DecimalField(max_digits=10, decimal_places=6, null=True, blank=True)

    # misc
    uptime = models.CharField(max_length=32, blank=True, default='')
    live_image_url = models.URLField(blank=True, default='')

    class Meta:
        ordering = ['-timestamp']

    def __str__(self):
        return f"Telemetry {self.device_id} @ {self.timestamp:%Y-%m-%d %H:%M:%S}"


class Mission(models.Model):
    name = models.CharField(max_length=140, default='Survey Mission')
    time_estimate = models.DurationField(null=True, blank=True)
    waypoints_count = models.PositiveIntegerField(default=0)
    current_wp = models.PositiveIntegerField(default=0)
    total_distance = models.FloatField(default=0.0)

    def __str__(self):
        return self.name


class PayloadStatus(models.Model):
    STATUS_CHOICES = [
        ('ACTIVE', 'Active'),
        ('INACTIVE', 'Inactive'),
        ('STABILIZED', 'Stabilized'),
        ('READY', 'Ready'),
    ]
    name = models.CharField(max_length=80)
    status = models.CharField(max_length=16, choices=STATUS_CHOICES, default='INACTIVE')

    def __str__(self):
        return f"{self.name}: {self.status}"


class SensorStatus(models.Model):
    name = models.CharField(max_length=80)
    status = models.CharField(max_length=32, default='NORMAL')

    def __str__(self):
        return f"{self.name} - {self.status}"


class WarningAlert(models.Model):
    LEVEL_CHOICES = [('low', 'Low'), ('high', 'High')]
    message = models.CharField(max_length=256)
    level = models.CharField(max_length=8, choices=LEVEL_CHOICES, default='low')
    created = models.DateTimeField(auto_now_add=True)

    def __str__(self):
        return f"[{self.level.upper()}] {self.message} @ {self.created:%H:%M:%S}"
    