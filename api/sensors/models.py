from datetime import datetime
from typing import Any, cast
import arrow
from django.core.exceptions import ObjectDoesNotExist
from django.db import models
from django.db.models import Avg
from influxdb_client import InfluxDBClient
from influxdb_client.client.write_api import SYNCHRONOUS
from .secrets import INFLUXDB_BUCKET, INFLUXDB_ORG, INFLUXDB_TOKEN
from django.utils.text import slugify
# Create your models here.
from twilio.rest import Client

# Comment this out if you don't want to set up Twilio alerts
twilio_client = Client("<SECRET>", "<SECRET>")


class Sensor(models.Model):
    name = models.CharField(max_length=60)
    sensor_id = models.CharField(max_length=100, default="")
    high_critical = models.DecimalField(max_digits=30, decimal_places=10, null=True)
    high_warn = models.DecimalField(max_digits=30, decimal_places=10, null=True)

    @property
    def average(self):
        avg = self.data.aggregate(Avg("numeric_value"))["numeric_value__avg"]
        if avg:
            return round(avg, 2)
        return 0.00

    @property
    def count(self):
        return self.data.all().count()

    @property
    def last_data_point(self) -> "SensorData":
        return self.data.all().order_by("-datetime").first()

    @property
    def last_eastern(self):
        if self.last_data_point:
            dt = self.last_data_point.datetime
            adt = arrow.Arrow.fromdatetime(dt)
            eastern = arrow.now("America/New_York").tzinfo
            dt = adt.astimezone(eastern)
            adt = arrow.Arrow.fromdatetime(dt)
            return adt.format("M/D/YYYY h:mm a")
        return ""

    @property
    def last_numeric(self):
        val = self.last_data_point.numeric_value
        if val:
            return round(val, 2)
        return 0

    def __str__(self) -> str:
        return f"{self.name}: {self.sensor_id}"


class SensorData(models.Model):
    sensor_mac_addr = models.CharField(max_length=50)
    datetime = models.DateTimeField()
    numeric_value = models.DecimalField(max_digits=30, decimal_places=10, null=True)
    string_value = models.TextField(null=True)
    sensor_id = models.CharField(max_length=100)
    owner = models.ForeignKey(
        Sensor, on_delete=models.CASCADE, null=True, default=None, related_name="data"
    )

    def __str__(self) -> str:
        return f"{self.sensor_id} at {self.datetime}"

    def save(self, *arg, **kwargs):
        try:
            owner = Sensor.objects.get(sensor_id=self.sensor_id)
        except ObjectDoesNotExist:
            owner = Sensor.objects.create(name=self.sensor_id, sensor_id=self.sensor_id)
        self.owner = owner
        client = InfluxDBClient(
            url="https://us-east-1-1.aws.cloud2.influxdata.com", token=INFLUXDB_TOKEN
        )
        
        reading = f"{float(self.numeric_value):.2f}"

        if owner.high_warn:
            if self.numeric_value > owner.high_warn:
                print(f"Trying to send a text about a high value with reading {reading}")
                twilio_client.messages.create(
                    from_="+<FROM_NUMBER>",
                    to="+<TO_NUMBER>",
                    body=f"The sensor {owner.name} is reading {reading}"
                )
                print("Did I do it?")

        if owner.high_critical:
            if self.numeric_value > owner.high_critical:
                print(f"Got a critical value {reading}")
                twilio_client.calls.create(
                    twiml=f"<Response><Say>The sensor {owner.name} got a critical reading {reading}</Say></Response>",
                    from_="+<FROM_NUMBER>",
                    to="+<TO_NUMBER>",
                )
                print("Handle warning critical")

        if not self.pk:
            sensor_name = slugify(owner.name)
            print("About to save to influx db!")
            write_api = client.write_api(write_options=SYNCHRONOUS)
            timestamp = int(self.datetime.timestamp() * 1e9)
            data = f"environmentals,mac_addr={self.sensor_mac_addr},sensor_id={self.sensor_id},sensor_name={sensor_name} value={self.numeric_value} {timestamp}"
            write_api.write(INFLUXDB_BUCKET, INFLUXDB_ORG, data)
            print("Saved to influxdb")

        super().save(*arg, **kwargs)
