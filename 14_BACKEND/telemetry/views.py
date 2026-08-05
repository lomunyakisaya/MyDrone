from django.shortcuts import render
from django.http import JsonResponse, request
from .models import Telemetry
#from django.contrib.auth.decorators import login_required

# Create your views here.
#@login_required
def home(request):
    return render(request, 'telemetry/home.html')

#@login_required
def telemetry_data(request):
    if request.method == 'GET':
        telemetry_data = Telemetry.objects.all().order_by('-timestamp')[:10]  # Get the last 10 entries
        return render(request, 'telemetry/telemetry.html', {'telemetry_data': telemetry_data})
        