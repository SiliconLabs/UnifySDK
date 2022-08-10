# How To Make an IoT Service

To build an IoT Service from scratch, it is recommended to follow the steps
below. IoT Services come in many forms, and exact steps will depend on the
detailed requirements of the IoT Service.

1. Find API and SDK bindings for target service. An IoT service connects the
   Unify Framework with an external service. Service should be interpreted in the
   broadest sense; it can be a cloud service, or it can be a UI operated by end
   users. Or something in between. The first step in writing an IoT Service is
   determining which APIs and SDKs to use for the target service. If the IoT
   Service connects to a cloud service, then the cloud service SDK is a good
   place to start. Alternatively, choosing a framework with good bindings to the
   cloud service. If the IoT Service is a UI, then a UI framework should be
   chosen based on the product requirements.
2. Choose an MQTT client. Now choose an MQTT client based on the programming
   language and frameworks selected in the previous step. Most MQTT clients
   should be compatible with the Unify Framework. If encryption is desired, make sure
   to choose a client that supports certificate-based TLS.
3. Discover nodes using MQTT topics. Discovering nodes and resources that can be
   displayed or manipulated is fundamental to any IoT Service. This is achieved
   by subscribing to particular MQTT topics. See the Service Discovery chapter
   of the [Unify Specification](https://siliconlabs.github.io/UnifySDK/unify-spec/html/index.html).
4. Implement Network Management. If the IoT Service needs to do network
   management, consider which operations are needed and implement them as
   described in the Network Management chapter of the
   [Unify Specification](https://siliconlabs.github.io/UnifySDK/unify-spec/html/index.html).
5. Interact with nodes using MQTT topics. Interacting with the nodes by either
   reading out status or controlling their state is fundamental to most IoT
   services. Subscribe and publish to the relevant MQTT topics as described in
   the Application Level Control chapter of the
   [Unify Specification](https://siliconlabs.github.io/UnifySDK/unify-spec/html/index.html).
6. Consider re-using appropriate shared components from the Unify Framework codebase.
   A valuable feature of Unify is the common components shared by many modules
   of the SDK. Code reuse saves both development time and offers IoT Service
   users a uniform experience operating and configuring different IoT services.

The Unify Framework provides a number of components that are candidates for re-use in
IoT Services. They are listed below.

1. Unify MQTT Client. The MQTT client is built on top of Eclipse Mosquitto MQTT
   and handles MQTT connection, subscribing and publishing. It implements
   re-connections and caching of published messages, which have been published
   while the client has been disconnected from the broker. See the section Unify
   Library Framework/MQTT Client in Unify Documentation for details.
2. Logging System. The logging system features logging with severity level
   tagging and log filtering. See the section Unify Library Framework/Logging
   System in the Unify Documentation for details.
3. Config System. The config system implements parsing of config files and
   command line arguments. See the Unify Library Framework/Config System in
   Unify Documentation for details.

The UPVL and GMS applications use the shared components and are good examples to
peruse. They can be found in the Unify source code in the `applications/gms/`
and `applications/upvl/` directories.
