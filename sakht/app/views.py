from django.shortcuts import render
from django.http import HttpResponse
from app.models import HealthHistory
from django.views.decorators.csrf import csrf_exempt
import json

def table(request):
	health_history = HealthHistory.objects.all().order_by('-id')[:1000]
	return render(request, 'table.html', {'health_history': health_history})

def table_view(request):
    health_history_data = HealthHistory.objects.all()

    context = {'health_history': health_history_data}
    return render(request, 'table_conventional.html', context)

@csrf_exempt
def api(request):
	health_history = HealthHistory.objects.all().order_by('-id')[:1000]
	data = []
	for h in health_history:
		data.append(
			{
				'SPO2': h.SPO2,
				'BPM': h.BPM,
				'location_x': h.location_x,
				'location_y': h.location_y,
				'time': h.created_at.strftime('%H:%M:%S')
			}
			)


	return HttpResponse(json.dumps({'health_history': data}))
