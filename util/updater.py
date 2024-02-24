# Tracker OTA update utility

import paho.mqtt.client as mqtt
import configparser, sys, traceback, datetime, argparse, os, platform, time
from dataclasses import dataclass
import logging as log
from enum import Enum

__version__ = "0.1.0"

log.getLogger().setLevel(log.INFO)
log.basicConfig(
    format='%(asctime)s %(levelname)-8s %(message)s',
    level=log.INFO,
    datefmt='%Y-%m-%d %H:%M:%S')

config = configparser.ConfigParser()
config.read('config.ini')

parser = argparse.ArgumentParser()
parser.add_argument("--update-all", help="try to update all the devices that connect to the MQTT broker", action='store_true')
parser.add_argument("-d", "--device", help="give one or more device ID to update(comma separated)", type=str)
parser.add_argument("wifi_details", help="Update wifi SSID and password in format ssid:password")

args = parser.parse_args()

mqtt_errors = {-1:"MQTT_ERR_AGAIN", 
               0:"MQTT_ERR_SUCCESS", 
               1:"MQTT_ERR_NOMEM", 
               2:"MQTT_ERR_PROTOCOL", 
               3:"MQTT_ERR_INVAL", 
               4:"MQTT_ERR_NO_CONN", 
               5:"MQTT_ERR_CONN_REFUSED", 
               6:"MQTT_ERR_NOT_FOUND",
               7:"MQTT_ERR_CONN_LOST",
               8:"MQTT_ERR_TLS",
               9:"MQTT_ERR_PAYLOAD_SIZE",
               10:"MQTT_ERR_NOT_SUPPORTED",
               11:"MQTT_ERR_AUTH",
               12:"MQTT_ERR_ACL_DENIED",
               13:"MQTT_ERR_UNKNOWN",
               14:"MQTT_ERR_ERRNO",
               15:"MQTT_ERR_QUEUE_SIZE",
               16:"MQTT_ERR_KEEPALIVE"}

class UpdateState(str, Enum):
    NOT_CONNECTED = "Not connected"
    CONNECTED = "Connected"
    UPDATE_CMD_SENT = "Update command sent"
    UPDATING= "Updating"
    DISCONNECTED = "Disconnected for update"
    FINNISHED = "Finnished"
    UNKNOWN = "Unknown"

@dataclass
class Device_updating_process:
    id: str
    state: UpdateState
    version: str
    last_msg_timestamp: datetime.datetime

@dataclass 
class Wifi_details:
    ssid: str
    passwd: str


devices = []
update_all_devices = False
ota_wifi = {}

def on_connect(client, userdata, flags, rc):
    if(rc != 0):
        if(rc == 1):
            log.error("MQTT Connection refused - incorrect protocol version")
        elif(rc == 2):
            log.error("MQTT Connection refused - invalid client identifier")
        elif(rc == 3):
            log.error("MQTT Connection refused - server unavailable")
        elif(rc == 4):
            log.error("MQTT Connection refused - bad username or password")
        elif(rc == 5):
            log.error("MQTT Connection refused - not authorised")
        
        log.error("Program exiting!")
        exit(1)
    else:
        log.info("Connected to MQTT broker successfully!")
        client.subscribe("tracker/#")
        return

def on_message(client, userdata, msg):
    if(len(msg.topic.split("/", 2)) == 3):  # if node id is valid
        project, id, key = msg.topic.split("/", 2)
        if(id.isdecimal() and len(id) == 7):
            msg_data = str(msg.payload.decode("utf-8"))
            if(project == "tracker"):
                device = None
                for device_in_list in devices:
                    if device_in_list.id == id:
                        device = device_in_list
                
                if(not device and update_all_devices):
                    # Create new device
                    device = Device_updating_process(id, UpdateState.UNKNOWN, None, datetime.datetime.now())
                    devices.append(device)

                if(device or update_all_devices):
                    if(key =="connected"):
                        if(msg_data == "1"):
                            # First time connecting: UNKOWN --> Connected
                            # 
                            if(device.state == UpdateState.UNKNOWN or device.state == UpdateState.NOT_CONNECTED):
                                # Send message to start OTA
                                time.sleep(2)
                                topic = "tracker/" + str(id) + "/ota"
                                message = ota_wifi.ssid + ":" + ota_wifi.passwd
                                client.publish(topic, message)
                                device.state = UpdateState.UPDATE_CMD_SENT
                            elif(device.state == UpdateState.UPDATING):
                                device.state = UpdateState.FINNISHED
                            else:
                                device.state = UpdateState.CONNECTED                      

                        elif(msg_data == "0"):
                            # From UPDATE CMD SENT --> UPDATING
                            # From 
                            if(device.state == UpdateState.CONNECTED):
                                device.state = UpdateState.DISCONNECTED
                            elif(device.state == UpdateState.UPDATE_CMD_SENT):
                                device.state = UpdateState.UPDATING
                            else:
                                device.state = UpdateState.NOT_CONNECTED

                    elif(key == "version"):
                        if device:
                            device.version = msg_data

                    device.last_msg_timestamp = datetime.datetime.now()

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message


client.username_pw_set(config["mqtt"]["user"],
            password=config["mqtt"]["passwd"])
client.connect(config["mqtt"]["host"],
            int(config["mqtt"]["port"]), 60)

def print_status(devices: Device_updating_process):
    if platform.system() == 'Windows':
        os.system('cls')
    else:
        os.system('clear')
    
    print("Tracker OTA helper v{}".format(__version__))
    print("MQTT connection: {}".format(client.is_connected()))
    print(" Device ID | Status | Version | Seconds since last message")
    for d in devices:
        time_str = None
        if(d.last_msg_timestamp):
            now = datetime.datetime.now()
            delta = now - d.last_msg_timestamp
            time_str = int(delta.total_seconds())

        print("{}|{}|{}|{}".format(d.id, d.state, d.version, time_str))

def main():
    try:
        global update_all_devices 
        update_all_devices = bool(args.update_all)
        # Parse device IDs from the arguments

        if(":" not in args.wifi_details):
            print("ERROR! Wifi details in wrong format. It should be ssid:password")
            exit(1)
        else:
            ssid, passwd = args.wifi_details.split(":")
            global ota_wifi
            ota_wifi = Wifi_details(ssid, passwd)

        if(not args.device and not update_all_devices):
            print("ERROR! No device IDs given to be updated. Specify the device ID that should be updated with -d or update all with --update-all ")
            exit(1)
        
        if(update_all_devices):
            device_ids = []
        else:
            device_ids = args.device.strip().split(",")
        
        for device_id in device_ids:
            devices.append(Device_updating_process(device_id, UpdateState.UNKNOWN, None, None))
        
        print_timestamp = datetime.datetime.now()

        while True:
            ret_code = client.loop()
            if(ret_code > 0):
                log.error("MQTT Loop returned error: {0}".format(mqtt_errors.get(ret_code)))
                exit(1)
            
            if(datetime.datetime.now() - print_timestamp > datetime.timedelta(seconds=1)):
                print_status(devices)
                print_timestamp = datetime.datetime.now()


    except KeyboardInterrupt:
        client.disconnect()
        print("Shutdown requested...exiting")
    except Exception:
        traceback.print_exc(file=sys.stdout)
    sys.exit(0)

if __name__ == "__main__":
    main()

