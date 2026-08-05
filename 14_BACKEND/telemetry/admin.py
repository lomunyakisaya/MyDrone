from django.contrib import admin
from .models import Telemetry, Mission, PayloadStatus

# Register your models here.
admin.site.register(Telemetry)
admin.site.register(Mission)
admin.site.register(PayloadStatus)
