from django.test import TestCase
from django.utils import timezone
from rest_framework.test import APIClient

from .models import SensorData


class AnimalTestCase(TestCase):
    def setUp(self):
        SensorData.objects.create(
            sensor_mac_addr="lion", datetime=timezone.now(), sensor_id="lion:1"
        )

    def test_animals_can_speak(self):
        data_point = SensorData.objects.get(sensor_id="lion:1")
        self.assertEqual(data_point.sensor_mac_addr, "lion")

    def test_can_get_sensor_data(self):
        client = APIClient()
        resp = client.get("/sensor-data/")
        self.assertEqual(resp.data["results"][0]["sensor_id"], "lion:1")
        self.assertEqual(resp.data["count"], 1)
        self.assertEqual(resp.status_code, 200)

    def test_can_post_sensor_data(self):
        client = APIClient()
        data = {
            "sensor_mac_addr": "ab:ce",
            "datetime": timezone.now(),
            "numeric_value": 12,
            "string_value": "howdy",
            "sensor_id": 32,
        }
        resp = client.post("/sensor-data/", data, format="json")
        self.assertEqual(resp.status_code, 201)
        self.assertEqual(SensorData.objects.all().count(), 2)
