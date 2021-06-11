from typing import Any, cast

from django.contrib import admin
from django.urls import path
from django.urls.conf import include
from django.utils import timezone
from rest_framework import routers, serializers, viewsets

from .models import SensorData


class SensorDataSerializer(serializers.HyperlinkedModelSerializer):
    datetime = serializers.DateTimeField(default=timezone.now)

    class Meta:
        model = SensorData
        fields = [
            "sensor_mac_addr",
            "datetime",
            "numeric_value",
            "string_value",
            "sensor_id",
            "id",
        ]


class SensorDataViewSet(viewsets.ModelViewSet):
    queryset = cast(Any, SensorData).objects.all()
    serializer_class = SensorDataSerializer
