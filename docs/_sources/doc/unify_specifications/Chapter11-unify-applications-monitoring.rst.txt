.. meta::
  :description: Application Monitoring Chapter
  :keywords: Application, Monitoring

.. raw:: latex

 \newpage

.. _unify_specifications_chapter_application_monitoring:

Unify Applications monitoring
=============================

This chapter describes how to monitor the various unify applications
deployed within the same MQTT broker.

Overview
--------

Application monitoring intend to provide application-level monitoring information.
Any application/MQTT client participating in the system should be able to be
monitored using this functionality.

An ApplicationMonitoring Unify cluster has been designed for this reporting purpose.
It operates as all other clusters, with Attributes, SupportedCommands
and Commands.

The difference about this cluster, is that it does not use any UNID, but instead
MQTT Client ID. An overview of the publications under this cluster is shown below:

.. code-block:: mqtt

  ucl/by-mqtt-client/<client-id>/ApplicationMonitoring/<AttributeName>/Reported { "value": 23 }
  ucl/by-mqtt-client/<client-id>/ApplicationMonitoring/SupportedCommands { "value": ["WriteAttributes"}] }
  ucl/by-mqtt-client/<client-id>/ApplicationMonitoring/SupportedGeneratedCommands { "value": ["LogMessage"] }
  ucl/by-mqtt-client/<client-id>/ApplicationMonitoring/Commands/<CommandName> { "field1": 42 }
  ucl/by-mqtt-client/<client-id>/ApplicationMonitoring/GeneratedCommands/<CommandName> { "field1": 42 }

No endpoints are used for this cluster.

Note that all communications and commands executions are expected to be
instantaneous in the `ucl/by-mqtt-client/` topic space. Therefore, the Desired value
of attributes MUST NOT be published under this topic space.

MQTT Client ID
--------------

Each MQTT Client connecting to the system is responsible for creating a unique
Client ID.

To allow for multiple instances of the same application avoiding to use the same
MQTT client ID, it is RECOMMENDED to use the following format:
`application_name-docker_container_id-process_id`. With the following elements:

* `application_name` is the name of the application running.
* `docker_container_id` is the docker container ID, if running in a docker container.
* `process_id` is the unique process identifier on the system.

For example, the unify_zpc running in a docker container ID d8453812a556 and
process ID 1 would be: `zpc-d8453812a556-1`.

If no docker container is used, the container ID should be skipped, for example
`zpc-37296`

MQTT Will Message
-----------------

Applications connecting to the MQTT broker that support ApplicationMonitoring
MUST publish all mandatory attributes from the ApplicationMonitoring cluster.

A special attribute called ApplicationConnected MUST alwasy be set to true. It
is used to detect non-graceful disconnects from MQTT Clients.

When establishing an MQTT connection to the broker, applications MUST set
their will message to be a publication at the following topic:

.. code-block:: mqtt

  ucl/by-mqtt-client/<client-id>/ApplicationMonitoring/ApplicationConnected/Reported { "value": false }


This way, other clients will be able to see detect if an application shut down
non-gracefully.

Applications disconnecting gracecully MUST unretaining all messages that they
published under their `ucl/by-mqtt-client/<client-id>/` topic.

The diagram below shows an example of a non-graceful disconnect of an application:

.. uml::

  'Style matching the other figures in chapter.
  !theme plain
  skinparam LegendBackgroundColor #F0F0F0

  ' Allows to do simultaneous transmissions
  !pragma teoz true

  legend top
  <font color=#0039FB>MQTT Subscription</font>
  <font color=#00003C>Retained MQTT Publication</font>
  <font color=#6C2A0D>Unretained MQTT Publication</font>
  <font color=#194D33>MQTT Will Message</font>
  endlegend

  participant "Application 1: ZPC" as zpc
  participant "MQTT Broker" as mqtt_broker

  rnote over zpc: Application starts.\nCreates a unique MQTT Client ID

  zpc -> mqtt_broker: <font color=#194D33>ucl/by-mqtt-client/zpc-12345/ApplicationMonitoring/Attributes/ApplicationConnected/Reported\n<font color=#194D33><b>{"value": false }
  zpc -> mqtt_broker: <font color=#0039FB>ucl/by-mqtt-client/zpc-12345/ApplicationMonitoring/Commands/+</font>
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-12345/ApplicationMonitoring/SupportedCommands\n<font color=#00003C><b>{"value":[]}

  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-12345/ApplicationMonitoring/Attributes/ApplicationName/Reported\n<font color=#00003C><b>{"value": "zpc" }
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-12345/ApplicationMonitoring/Attributes/ApplicationVersion/Reported\n<font color=#00003C><b>{"value": "ver_1.1.1_RC2-31-ga25147fc" }
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-12345/ApplicationMonitoring/Attributes/ApplicationConnected/Reported\n<font color=#00003C><b>{"value": true }
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-12345/ApplicationMonitoring/Attributes/UptimeMinutes/Reported\n<font color=#00003C><b>{"value": 0 }
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-12345/ApplicationMonitoring/Attributes/ProcessId/Reported\n<font color=#00003C><b>{"value": 62386 }

  rnote over zpc, mqtt_broker: ZPC gets disconnected non-gracefully
  ...

  rnote left of mqtt_broker: zpc-12345 has not checked in within the keep alive interval
  mqtt_broker <- mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-12345/ApplicationMonitoring/Attributes/ApplicationConnected/Reported\n<font color=#00003C><b>{"value": false }


Logging
-------

Applications supporting the logging functionality will advertise the following
attributes as supported:

* MQTTLoggingEnabled
* LoggingLevel

They will support the following *WriteAttributes* command to modify these
attributes and they will generate a *LogEntry* for each log message when
logging via MQTT is enabled.

.. uml::

  'Style matching the other figures in chapter.
  !theme plain
  skinparam LegendBackgroundColor #F0F0F0

  ' Allows to do simultaneous transmissions
  !pragma teoz true

  legend top
  <font color=#0039FB>MQTT Subscription</font>
  <font color=#00003C>Retained MQTT Publication</font>
  <font color=#6C2A0D>Unretained MQTT Publication</font>
  <font color=#194D33>MQTT Will Message</font>
  endlegend

  participant "Application 1: ZPC" as zpc
  participant "MQTT Broker" as mqtt_broker
  participant "IoT Service" as iot_service

  rnote over zpc: Application starts.\nCreates a unique MQTT Client ID

  zpc -> mqtt_broker: <font color=#194D33>ucl/by-mqtt-client/zpc-11425/ApplicationMonitoring/Attributes/ApplicationConnected/Reported\n<font color=#194D33><b>{"value": false }
  zpc -> mqtt_broker: <font color=#0039FB>ucl/by-mqtt-client/zpc-11425/ApplicationMonitoring/Commands/+</font>
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-11425/ApplicationMonitoring/SupportedCommands\n<font color=#00003C><b>{"value":["WriteAttributes"]}
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-11425/ApplicationMonitoring/SupportedGeneratedCommands\n<font color=#00003C><b>{"value":["LogEntry"]}

  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-11425/ApplicationMonitoring/Attributes/ApplicationName/Reported\n<font color=#00003C><b>{"value": "zpc" }
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-11425/ApplicationMonitoring/Attributes/ApplicationVersion/Reported\n<font color=#00003C><b>{"value": "ver_1.1.1_RC2-31-ga25147fc" }
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-11425/ApplicationMonitoring/Attributes/ApplicationConnected/Reported\n<font color=#00003C><b>{"value": true }
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-11425/ApplicationMonitoring/Attributes/MQTTLoggingEnabled/Reported\n<font color=#00003C><b>{"value": false }
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-11425/ApplicationMonitoring/Attributes/MQTTLoggingLevel/Reported\n<font color=#00003C><b>{"value": "Info" }

  rnote over zpc, iot_service: IoT Service wants to enable MQTT logging for ZPC

  iot_service -> mqtt_broker: <font color=#6C2A0D>ucl/by-mqtt-client/zpc-11425/ApplicationMonitoring/Commands/WriteAttributes\n<font color=#6C2A0D><b>{"MQTTLoggingEnabled": true, "MQTTLoggingLevel": "Info"}
  & mqtt_broker -> zpc

  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-11425/ApplicationMonitoring/Attributes/MQTTLoggingEnabled/Reported\n<font color=#00003C><b>{"value": true }

  loop for every log message
    zpc -> mqtt_broker: <font color=#6C2A0D>ucl/by-mqtt-client/zpc-11425/ApplicationMonitoring/GeneratedCommands/LogEntry\n<font color=#6C2A0D><b>{"Timestamp": "2022-Mar-24 09:17:59.465807",\n<font color=#6C2A0D><b>"LogLevel":"Info",\n<font color=#6C2A0D><b>"LogTag":"ncp",\n<font color=#6C2A0D><b>"LogMessage": "No TCP address specified for NCP. Falling back on serial port."}
  end


Detecting stale MQTT topics
---------------------------

Each application SHOULD publish a list of expected topic spaces that they
will be using to publish.

Using this information, an IoT Service can detect and decide to remove stale
topics when an application left the system non-gracefully.

.. uml::

  'Style matching the other figures in chapter.
  !theme plain
  skinparam LegendBackgroundColor #F0F0F0

  ' Allows to do simultaneous transmissions
  !pragma teoz true

  legend top
  <font color=#0039FB>MQTT Subscription</font>
  <font color=#00003C>Retained MQTT Publication</font>
  <font color=#6C2A0D>Unretained MQTT Publication</font>
  <font color=#194D33>MQTT Will Message</font>
  endlegend

  participant "Application 1: ZPC" as zpc
  participant "MQTT Broker" as mqtt_broker
  participant "IoT Service" as iot_service

  rnote over zpc: Application starts.\nCreates a unique MQTT Client ID

  zpc -> mqtt_broker: <font color=#194D33>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/ApplicationConnected/Reported\n<font color=#194D33><b>{"value": false }
  zpc -> mqtt_broker: <font color=#0039FB>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Commands/+</font>
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/SupportedCommands\n<font color=#00003C><b>{"value":[]}

  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/ApplicationName/Reported\n<font color=#00003C><b>{"value": "zpc" }
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/ApplicationVersion/Reported\n<font color=#00003C><b>{"value": "ver_1.1.1_RC2-31-ga25147fc" }
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/ApplicationConnected/Reported\n<font color=#00003C><b>{"value": true }

  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/ApplicationMQTTTopics/Reported\n<font color=#00003C><b>{"value": ["ucl/by-unid/zw-12345678-0001/#","ucl/by-mqtt-client/zpc-65324/#"]]}

  rnote over zpc, mqtt_broker: ZPC gets disconnected non-gracefully

  ...

  rnote left of mqtt_broker: zpc-65324 has not checked in within the keep alive interval
  mqtt_broker <- mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/ApplicationConnected/Reported\n<font color=#00003C><b>{"value": false }
  rnote over mqtt_broker, iot_service: ZPC is disconnected.\nTopics matching these filters can be removed: \n- "ucl/by-unid/zw-12345678-0001/#"\n- "ucl/by-mqtt-client/zpc-65324/#"


MQTT Statistics for each application
------------------------------------

Each application MAY report basic statistics about its MQTT communications
with the broker.

As for SystemMetrics, applications that support reporting MQTT Statistics
MUST support the *MQTTStatisticsReportingIntervalSeconds* attribute, that
will allow clients to configure how often these statistics are to be published.

.. uml::

  'Style matching the other figures in chapter.
  !theme plain
  skinparam LegendBackgroundColor #F0F0F0

  ' Allows to do simultaneous transmissions
  !pragma teoz true

  legend top
  <font color=#0039FB>MQTT Subscription</font>
  <font color=#00003C>Retained MQTT Publication</font>
  <font color=#6C2A0D>Unretained MQTT Publication</font>
  endlegend

  participant "Application 1: ZPC" as zpc
  participant "MQTT Broker" as mqtt_broker
  participant "IoT Service" as iot_service

  rnote over zpc: Application starts.\nCreates a unique MQTT Client ID

  zpc -> mqtt_broker: <font color=#0039FB>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Commands/+</font>
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/SupportedCommands\n<font color=#00003C><b>{"value":[]}

  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/ApplicationName/Reported\n<font color=#00003C><b>{"value": "zpc" }
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/ApplicationVersion/Reported\n<font color=#00003C><b>{"value": "ver_1.1.1_RC2-31-ga25147fc" }
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/ApplicationConnected/Reported\n<font color=#00003C><b>{"value": true }


  == one off reporting of MQTT statistics attributes ==

  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/MQTTStatisticsReportingIntervalSeconds/Reported\n<font color=#00003C><b>{"value": 0}
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/MQTTMessagesSent/Reported\n<font color=#00003C><b>{"value": 6}
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/MQTTMessagesReceived/Reported\n<font color=#00003C><b>{"value": 2}
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/MQTTSubscriptionCount/Reported\n<font color=#00003C><b>{"value": 56}
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/MQTTAverageDeliveryTimeSeconds/Reported\n<font color=#00003C><b>{"value": 0.002}
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/MQTTMinDeliveryTimeSeconds/Reported\n<font color=#00003C><b>{"value": 0.00001}
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/MQTTMaxDeliveryTimeSeconds/Reported\n<font color=#00003C><b>{"value": 0.093}


  == IoT Service wants MQTT statistics updates every 3600 seconds ==

  iot_service -> mqtt_broker: <font color=#6C2A0D>ucl/by-mqtt-client/zpc-65324/SystemMetrics/Commands/WriteAttributes\n<font color=#6C2A0D><b>{"MQTTStatisticsReportingIntervalSeconds":3600 }
  & mqtt_broker -> zpc
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/MQTTStatisticsReportingIntervalSeconds/Reported\n<font color=#00003C><b>{"value": 3600}

  loop Every 3600 seconds
    zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/MQTTMessagesSent/Reported\n<font color=#00003C><b>{"value": 34}
    zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/MQTTMessagesReceived/Reported\n<font color=#00003C><b>{"value": 3}
    zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/MQTTSubscriptionCount/Reported\n<font color=#00003C><b>{"value": 56}
    zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/MQTTAverageDeliveryTimeSeconds/Reported\n<font color=#00003C><b>{"value": 0.00195}
    zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/MQTTMinDeliveryTimeSeconds/Reported\n<font color=#00003C><b>{"value": 0.000009}
    zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/MQTTMaxDeliveryTimeSeconds/Reported\n<font color=#00003C><b>{"value": 0.093}
  end


System Statistics for each application
--------------------------------------

Each application MAY report System Metrics like statistics for itself.

As for SystemMetrics, applications that support reporting Application
System Statistics MUST support the *ApplicationStatisticsReportingIntervalSeconds*
attribute, that will allow MQTT clients to configure how often these statistics
are to be published.

.. uml::

  'Style matching the other figures in chapter.
  !theme plain
  skinparam LegendBackgroundColor #F0F0F0

  ' Allows to do simultaneous transmissions
  !pragma teoz true

  legend top
  <font color=#0039FB>MQTT Subscription</font>
  <font color=#00003C>Retained MQTT Publication</font>
  <font color=#6C2A0D>Unretained MQTT Publication</font>
  endlegend

  participant "Application 1: ZPC" as zpc
  participant "MQTT Broker" as mqtt_broker
  participant "IoT Service" as iot_service

  rnote over zpc: Application starts.\nCreates a unique MQTT Client ID

  zpc -> mqtt_broker: <font color=#0039FB>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Commands/+</font>
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/SupportedCommands\n<font color=#00003C><b>{"value":[]}

  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/ApplicationName/Reported\n<font color=#00003C><b>{"value": "zpc" }
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/ApplicationVersion/Reported\n<font color=#00003C><b>{"value": "ver_1.1.1_RC2-31-ga25147fc" }
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/ApplicationConnected/Reported\n<font color=#00003C><b>{"value": true }


  == one off reporting of Application System statistics attributes ==

  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/ApplicationStatisticsReportingIntervalSeconds/Reported\n<font color=#00003C><b>{"value": 0}
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/ApplicationCPUUsagePercent/Reported\n<font color=#00003C><b>{"value": 0.8}
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/ApplicationCPUAverageUsagePercent/Reported\n<font color=#00003C><b>{"value": 0.54}
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/ApplicationCPUMinUsagePercent/Reported\n<font color=#00003C><b>{"value": 0.01}
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/ApplicationCPUMaxUsagePercent/Reported\n<font color=#00003C><b>{"value": 18.62}
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/ApplicationRAMUsageMB/Reported\n<font color=#00003C><b>{"value": 0.5348}


  == IoT Service wants Application statistics updates every 60 seconds ==

  iot_service -> mqtt_broker: <font color=#6C2A0D>ucl/by-mqtt-client/zpc-65324/SystemMetrics/Commands/WriteAttributes\n<font color=#6C2A0D><b>{"ApplicationStatisticsReportingIntervalSeconds":60 }
  & mqtt_broker -> zpc
  zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/ApplicationStatisticsReportingIntervalSeconds/Reported\n<font color=#00003C><b>{"value": 60}

  loop Every 60 seconds
    zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/ApplicationCPUUsagePercent/Reported\n<font color=#00003C><b>{"value": 2.15}
    zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/ApplicationCPUAverageUsagePercent/Reported\n<font color=#00003C><b>{"value": 0.64}
    zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/ApplicationCPUMinUsagePercent/Reported\n<font color=#00003C><b>{"value": 0.01}
    zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/ApplicationCPUMaxUsagePercent/Reported\n<font color=#00003C><b>{"value": 18.93}
    zpc -> mqtt_broker: <font color=#00003C>ucl/by-mqtt-client/zpc-65324/ApplicationMonitoring/Attributes/ApplicationRAMUsageMB/Reported\n<font color=#00003C><b>{"value": 0.5846}
  end

