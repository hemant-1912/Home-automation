// devices.tsx

import Paho from 'paho-mqtt';
import React, { useEffect, useRef, useState } from 'react';
import { Alert, FlatList, StyleSheet, Switch, Text, View } from 'react-native';

const devices = [
  { id: '1', name: 'Cooler', topic: 'home/cooler' },
  { id: '2', name: 'Fan', topic: 'home/fan' },
  { id: '3', name: 'Light', topic: 'home/light' },
  { id: '4', name: 'Heater', topic: 'home/heater' },
  ];

export default function DevicesScreen() {
  // Dynamically initialize deviceStates for all devices
  const [deviceStates, setDeviceStates] = useState<{ [key: string]: boolean }>(
    Object.fromEntries(devices.map(device => [device.id, false]))
  );
  const client = useRef<any>(null); // Avoid using Paho.Client due to TS error

  useEffect(() => {
    const mqttClient = new Paho.Client(
      'MQTT_Broker',
      8884,
      '/mqtt',
      `clientId-${Math.random().toString(16).slice(2)}`
    );

    client.current = mqttClient;

    mqttClient.onConnectionLost = (responseObject: any) => {
      if (responseObject.errorCode !== 0) {
        console.log('⚠️ MQTT connection lost:', responseObject.errorMessage);
      }
    };

    mqttClient.onMessageArrived = (message: any) => {
      const topic = message.destinationName;
      const payload = message.payloadString;

      const targetDevice = devices.find(device => device.topic === topic);
      if (targetDevice) {
        const isOn = payload.trim().toUpperCase() === 'ON';
        setDeviceStates(prev => ({
          ...prev,
          [targetDevice.id]: isOn,
        }));
      }
    };

    mqttClient.connect({
      useSSL: true,
      userName: 'USERNAME',
      password: 'PASSWORD',
      onSuccess: () => {
        console.log('✅ MQTT connected');
        devices.forEach(device => {
          mqttClient.subscribe(device.topic);
          // Force device OFF on connect
          const offMsg = new Paho.Message('OFF');
          offMsg.destinationName = device.topic;
          mqttClient.send(offMsg);
          console.log(`🔻 Forced OFF sent to ${device.topic}`);
        });
      },
      onFailure: (err: any) => {
        console.log('❌ MQTT connect failed:', err.errorMessage);
        Alert.alert('MQTT Connection Failed', err.errorMessage || 'Unknown error');
      },
    });

    return () => {
      if (mqttClient?.isConnected()) {
        mqttClient.disconnect();
      }
    };
  }, []);

  const toggleDevice = (id: string, topic: string) => {
    const newState = !deviceStates[id];

    setDeviceStates(prev => ({
      ...prev,
      [id]: newState,
    }));

    if (client.current?.isConnected()) {
      const message = new Paho.Message(newState ? 'ON' : 'OFF');
      message.destinationName = topic;
      client.current.send(message);
      console.log(`📤 Sent to ${topic}: ${message.payloadString}`);
    } else {
      console.log('⚠️ MQTT client not connected');
    }
  };

  const renderItem = ({ item }: { item: { id: string; name: string; topic: string } }) => (
    <View style={styles.deviceItem}>
      <Text style={styles.deviceName}>{item.name}</Text>
      <Switch
        value={deviceStates[item.id]}
        onValueChange={() => toggleDevice(item.id, item.topic)}
      />
    </View>
  );

  return (
    <View style={styles.container}>
      <Text style={styles.header}>Devices</Text>
      <FlatList
        data={devices}
        keyExtractor={item => item.id}
        renderItem={renderItem}
      />
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    padding: 24,
  },
  header: {
    fontSize: 28,
    fontWeight: 'bold',
    marginBottom: 20,
  },
  deviceItem: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    paddingVertical: 12,
    borderBottomWidth: 1,
    borderColor: '#ccc',
  },
  deviceName: {
    fontSize: 20,
  },
});
