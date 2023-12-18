# Django MQTT Integration

This Django project integrates with an MQTT broker to receive and display messages on a simple frontend page.

## Getting Started

### Prerequisites

- [Python](https://www.python.org/) (>=3.6)
- [Django](https://www.djangoproject.com/)
- [paho-mqtt](https://pypi.org/project/paho-mqtt/)

### Installation

1. Install Django:

   ```bash
   pip install django

2. Create a Django project:
  django-admin startproject myproject
  cd myproject

3. pip install paho-mqtt

4. Update myproject/settings.py to include your app:
  INSTALLED_APPS = [
     ...
    'myapp',
  ]

5. Create a Django model in myapp/models.py:
   
  from django.db import models
  
  class MqttMessage(models.Model):
  
      topic = models.CharField(max_length=255)
      
      message = models.TextField()
      
      timestamp = models.DateTimeField(auto_now_add=True)
  
      def __str__(self):
      
          return f"{self.topic}: {self.message}"

6. Run migrations:
  python manage.py makemigrations

  python manage.py migrate

8. Create an MQTT listener management command in myapp/management/commands/listen_mqtt.py:
   
   Copy the content from the provided listen_mqtt.py script

9. Run the listener:
  python manage.py listen_mqtt

10. Create a Django view in myapp/views.py:
  
  from django.shortcuts import render
  
  from myapp.models import MqttMessage
  
  def mqtt_messages(request):
      
      messages = MqttMessage.objects.all()
      
      return render(request, 'myapp/mqtt_messages.html', {'messages': messages})

11. Create a Django template in myapp/templates/myapp/mqtt_messages.html:
  <!-- Copy the content from the provided mqtt_messages.html template -->

12. Create a Django URL pattern in myapp/urls.py:

  from django.urls import path
  
  from .views import mqtt_messages
  
  urlpatterns = [
      path('mqtt_messages/', mqtt_messages, name='mqtt_messages'),
  ]

13. Include app URLs in project URLs (myproject/urls.py):
    
  from django.contrib import admin
  
  from django.urls import path, include
  
  urlpatterns = [
      path('admin/', admin.site.urls),
      path('myapp/', include('myapp.urls')),
  ]

14. Run the Django development server:
  python manage.py runserver

14. Visit http://localhost:8000/myapp/mqtt_messages/ in your browser to view MQTT messages.






