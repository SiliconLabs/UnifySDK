.. meta::
  :description: System Metrics Chapter
  :keywords: Metrics

.. raw:: latex

 \newpage

.. _unify_specifications_chapter_system_metrics:

System Metrics
==============

This chapter describes how to report and read system metrics. System metrics
are a diagnostic tool allowing to retrieve data relating to the a machine
health and performance.

Overview
--------

System metrics intend to provide machine-level diagnostics. Any host/machine
participating in the system should be able to be configured and reports
its system metrics data.

A SystemMetrics Unify cluster has been designed for this reporting purpose.
It operates as all other clusters, with Attributes, SupportedCommands
and Commands.

The difference about this cluster, is that it does not use any UNID, but instead
Machine IDs. An overview of the publications under this cluster is shown below:

.. code-block:: mqtt

  ucl/by-machine-id/<id>/SystemMetrics/Attributes/<AttributeName>/Reported - { "value": 23 }
  ucl/by-machine-id/<id>/SystemMetrics/SupportedCommands - { "value": ["WriteAttributes"] }
  ucl/by-machine-id/<id>/SystemMetrics/Commands/<CommandName> - { "field1": 42 }

No endpoints are used for this cluster.

Note that all communications and commands executions are expected to be
instantaneous in the `ucl/by-machine-id/` topic space. Therefore, the Desired value
of attributes MUST NOT be published under this topic space.

Machine ID
----------

Each Machine connecting to the system is responsible for creating a unique
Machine ID.

If no unique machine-id is available natively on the platform running Unify
components, the following elements can be used as a Machine ID:

* `FQDN`: Fully Qualified Domain Name
* `hostname`: Hostname assigned to the machine
* `uuid`: Disk UUIDs

Developers and system integrators SHOULD ensure make sure that Machine IDs
connected to a given MQTT brokers are unique, so no collisions will happens
when reporting SystemMetrics.

Example
-------

The minimum required attribute for the System Metrics is the
*ReportingIntervalSeconds* attribute. This attribute is a setting that
controls how often the system metrics attributes are reported, in seconds.
The value 0 means that the system metrics are not reported.

System Metrics attributes MUST be published once at startup, and subsequently
following the frequency configured in the *ReportingIntervalSeconds* attribute.

*ReportingIntervalSeconds* SHOULD be set to 0 by default and reporting metrics
SHOULD be disabled by default.

.. uml::

  'Style matching the other figures in chapter.
  !theme plain
  skinparam LegendBackgroundColor #F0F0F0

  ' Allows to do simultaneous transmissions
  !pragma teoz true

  legend top
  <font color=#0039FB>MQTT Subscription</font>
  <font color=#008000>MQTT Unsubscription</font>
  <font color=#00003C>Retained MQTT Publication</font>
  <font color=#6C2A0D>Unretained MQTT Publication</font>
  endlegend

  participant "Machine 1: id raspberrypi" as rpi
  participant "MQTT Broker" as mqtt_broker
  participant "IoT Service" as iot_service

  rpi -> mqtt_broker: <font color=#0039FB>ucl/by-machine-id/+/SystemMetrics/Commands/+</font>
  iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-machine-id/+/SystemMetrics/Attributes/#</font>
  iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-machine-id/+/SystemMetrics/SupportedCommands/#</font>

  == System Metrics reporting is ready ==

  rpi -> mqtt_broker: <font color=#0039FB>ucl/by-machine-id/raspberrypi/SystemMetrics/Commands/WriteAttributes</font>
  rpi -> mqtt_broker: <font color=#00003C>ucl/by-machine-id/raspberrypi/SystemMetrics/SupportedCommands\n<font color=#00003C><b>{"value":["WriteAttributes"]}
  rpi -> mqtt_broker: <font color=#00003C>ucl/by-machine-id/raspberrypi/SystemMetrics/Attributes/ReportingIntervalSeconds/Reported\n<font color=#00003C><b>{"value":0}
  & mqtt_broker -> iot_service

  rpi -> mqtt_broker: <font color=#00003C>ucl/by-machine-id/raspberrypi/SystemMetrics/Attributes/DisksUsage/Reported\n<font color=#00003C><b>{"value":[{"UUID":"disk1","UsagePercent": 20.45,"FreeSpaceMB": 235600.9}]}
  & mqtt_broker -> iot_service
  rpi -> mqtt_broker: <font color=#00003C>ucl/by-machine-id/raspberrypi/SystemMetrics/Attributes/CPUUsagePercent/Reported\n<font color=#00003C><b>{"value":[8.2]}
  & mqtt_broker -> iot_service

  rnote over rpi: Metrics are published once\n but not refreshed.

  == IoT Service wants updates every 10 seconds ==

  iot_service -> mqtt_broker: <font color=#6C2A0D>ucl/by-machine-id/raspberrypi/SystemMetrics/Commands/WriteAttributes\n<font color=#6C2A0D><b>{"ReportingIntervalSeconds":10}
  & mqtt_broker -> rpi
  rpi -> mqtt_broker: <font color=#00003C>ucl/by-machine-id/raspberrypi/SystemMetrics/Attributes/ReportingIntervalSeconds/Reported\n<font color=#00003C><b>{"value":10}
  & mqtt_broker -> iot_service

  loop Every 10 seconds
    rpi -> mqtt_broker: <font color=#00003C>ucl/by-machine-id/raspberrypi/SystemMetrics/Attributes/DisksUsage/Reported\n<font color=#00003C><b>{"value":[{"UUID":"disk1","UsagePercent": 20.45,"FreeSpaceMB": 235600.9}]}
    & mqtt_broker -> iot_service
    rpi -> mqtt_broker: <font color=#00003C>ucl/by-machine-id/raspberrypi/SystemMetrics/Attributes/CPUUsagePercent/Reported\n<font color=#00003C><b>{"value":[8.2]}
    & mqtt_broker -> iot_service
  end

System Metrics attributes
-------------------------

System Metrics attributes help reporting the state of a machine on which the
unify applications are running.

System Metrics attributes MUST be reported following the
*ReportingIntervalSeconds*, but some exceptions MAY be made if an attribute
changes sporadically, like the *PowerPlugged* attribute.
