import paho.mqtt.client as mqtt
import time
import json

from tweepy.streaming import StreamListener
from tweepy import OAuthHandler
from tweepy import Stream

#Variables that contains yours credentials to access Twitter API 
access_token = ""
access_token_secret = ""
consumer_key = ""
consumer_secret = ""
hashtag="#mmbbshack"
topic="mmbbs/hack"
msg=""

def on_disconnect(client, userdata, rc):
    print("Disconnected with result code "+str(rc))

    # The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))



# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
#client.loop_forever()
#This is a basic listener that just prints received tweets to stdout.
class StdOutListener(StreamListener):
    global client
    client = mqtt.Client()


    def __init__(self):        
        client.on_connect = on_connect
        client.on_message = on_message
        client.on_disconnect = on_disconnect
        client.connect("service.joerg-tuttas.de", 1883, 60)
        client.loop_start()

    def on_data(self, data):
        global client 
        global msg
        j = json.loads(data)
        obj = {}
        obj['text'] = j['text']
        obj['username'] = j['user']['name']
        obj['screen_name'] = j['user']['screen_name']
        json_data = json.dumps(obj)
        print (json_data)
        client.publish(topic, json_data, 1, True)
        print("Published!")
        return True

    def on_error(self, status):
        print(status)

def start_stream():
    while True:
        try:
            #This handles Twitter authetification and the connection to Twitter Streaming API
            l = StdOutListener()
            auth = OAuthHandler(consumer_key, consumer_secret)
            auth.set_access_token(access_token, access_token_secret)
            stream = Stream(auth, l)

            #This line filter tweets from the words.
            stream.filter(track=[hashtag])
            #client.loop_forever()
        except: 
            continue

if __name__ == '__main__':
    start_stream()
   


    