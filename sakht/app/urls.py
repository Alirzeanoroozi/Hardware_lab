from django.urls import path
from . import views


urlpatterns = [
    path(
        '',
        views.table, 
        name='table'
    ),
    path(
        'api/',
        views.api, 
        name='api'
    ),
    path(
        'table/', 
        views.table_view, 
        name='table_view'),
]
