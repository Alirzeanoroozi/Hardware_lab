from django.db import models


class HealthHistory(models.Model):
	location_x = models.IntegerField()
	location_y = models.IntegerField()
	SPO2 = models.IntegerField(default=0)
	BPM = models.IntegerField(default=0)
	created_at = models.DateTimeField(auto_now_add=True, auto_now=False, null=True)

	@staticmethod
	def add_health_history(location_x, location_y, SPO2, BPM):
		health_history = HealthHistory()
		health_history.location_x = location_x
		health_history.location_y = location_y
		health_history.SPO2 = SPO2
		health_history.BPM = BPM
		health_history.save()
		return health_history
