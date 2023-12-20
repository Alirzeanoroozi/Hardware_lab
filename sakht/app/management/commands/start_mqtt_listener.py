import sys
import logging
from django.core.management.base import BaseCommand
from app.mqtt_listener import start_mqtt_listener

class Command(BaseCommand):
    help = 'Starts the MQTT listener'

    def handle(self, *args, **options):
        log_file = 'mqtt_listener.log'  # Set your desired log file path

        # Configure logging to both console and file
        logging.basicConfig(level=logging.INFO,
                            format='%(asctime)s - %(levelname)s - %(message)s',
                            handlers=[
                                logging.StreamHandler(sys.stdout),
                                logging.FileHandler(log_file),
                            ])

        logging.info('MQTT listener started')
        start_mqtt_listener()