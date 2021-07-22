import pymysql
import paho.mqtt.client as mqtt
import os

########### MQTT ###############
def on_connect(client, userdata, flags, rc):
    client.subscribe("NeRDDOOR/nerd/RFID")

def publish(topic, message, waitForAck = False):
    client.publish(topic, message, 2)

def on_message(client, userdata, msg):
    global mqttReceived
    global newMsg
    mqttReceived = msg.payload.decode('utf-8')
    newMsg = True

currentMqtt = "nerd"

client = mqtt.Client()
client.username_pw_set("","") # MQTT User and Pwd
client.on_connect = on_connect
client.on_message = on_message
client.connect("",1883,60) # MQTT Server IP
client.loop_start()
################################

########### Database ###########
db = pymysql.connect("localhost","","","") # SQL User, Pwd and Database
cursor = db.cursor()

currentDB = "nerd"

def fetchDB():
    cursor.execute("SELECT * FROM rfid_" + currentDB)
    data = cursor.fetchall()
    return data

def addUserDB(name,rfid):
    print(name)
    print(rfid)
    cursor.execute('INSERT INTO rfid_' + currentDB + '(name,rfid) VALUES ("' + name + '","' + rfid + '");')
    db.commit()

def removeUserDBbyName(name):
    cursor.execute('DELETE FROM rfid_' + currentDB + ' WHERE name = "' + name + '";')
    db.commit()

def removeUserDBbyID(userid):
    cursor.execute('DELETE FROM rfid_' + currentDB + ' WHERE id = ' + str(userid) + ';')
    db.commit()

################################

########### Menu ###############
def printMenu():
    os.system('clear')
    print("===================NeRDDOOR Menu===================")
    print("Currently working on: " + currentDB)
    print("1 - Add user by Reader")
    print("2 - Add user by RFID")
    print("3 - Remove user by entry ID")
    print("4 - Remove user by Name")
    print("5 - Print database")
    print("6 - Open")
    print("7 - Close")

def inputAction(choice):
    if choice == 0:
        pass
    if choice == 1:
        addId()
    if choice == 2:
        addRFID()
    if choice == 3:
        rmID()
    if choice == 4:
        rmName()
    if choice == 5:
        printDB()
    if choice == 6:
        publish("NeRDDOOR/" + currentMqtt + "/State","Open")
    if choice == 7:
        quit()

def printDB():
    data = fetchDB()
    print("{0:5s} {1:20s} {2:20s}".format("ID","Name","RFID"))
    print("====================================================")
    for i in range(len(data)):
        print("{0:5s} {1:20s} {2:20s}".format(str(data[i][0]),data[i][1],data[i][2]))
    input("Press <ENTER> to continue")

def addId():
    global mqttReceived
    global newMsg
    name = input("Name: ")
    newMsg = False
    while not newMsg:
        pass
    addUserDB(name,mqttReceived)
    print("Added user " + name + " with RFID : " + mqttReceived)
    input("Press <ENTER> to continue")

def addRFID():
    name = input("Name: ")
    rfid = input("RFID: ")
    addUserDB(name,rfid)
    print("Added user " + name + " with RFID : " + rfid)
    input("Press <ENTER> to continue")

def rmID():
    userid = int(input("Entry to remove: "))
    removeUserDBbyID(userid)
    print("User removed!")
    input("Press <ENTER> to continue")

def rmName():
    name = input("Name to remove: ")
    removeUserDBbyName(name)
    print("User removed!")
    input("Press <ENTER> to continue")

################################

if __name__ == "__main__":
    while 1:
        printMenu()
        inputAction(int(input("->")))
