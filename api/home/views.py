from django.shortcuts import render
from django.views.generic import TemplateView

from sensors.models import Sensor

# Create your views here.


class HomeView(TemplateView):
    template_name = "home.html"

    def get_context_data(self, **kwargs):
        context = super().get_context_data(**kwargs)
        context["sensors"] = Sensor.objects.all()
        return context
