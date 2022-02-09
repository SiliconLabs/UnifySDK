/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#ifndef MQTT_CLIENT_FSM_H
#define MQTT_CLIENT_FSM_H

// Forward-declare the client-struct/class.
struct mqtt_client;

class mqtt_client_fsm
{
  public:
  virtual void enter(mqtt_client *client_instance)                     = 0;
  virtual void exit(mqtt_client *client_instance)                      = 0;
  virtual void event(int incoming_event, mqtt_client *client_instance) = 0;
  virtual ~mqtt_client_fsm() {}
};

class mqtt_client_fsm_disconnected : public mqtt_client_fsm
{
  public:
  void enter(mqtt_client *client_instance) override;
  void exit(mqtt_client *client_instance) override;
  void event(int incoming_event, mqtt_client *client_instance) override;
  static mqtt_client_fsm &get_instance();

  private:
  mqtt_client_fsm_disconnected() {}
  mqtt_client_fsm_disconnected(const mqtt_client_fsm_disconnected &other);
  mqtt_client_fsm_disconnected &
    operator=(const mqtt_client_fsm_disconnected &other);
};

class mqtt_client_fsm_connecting : public mqtt_client_fsm
{
  public:
  void enter(mqtt_client *client_instance) override;
  void exit(mqtt_client *client_instance) override;
  void event(int incoming_event, mqtt_client *client_instance) override;
  static mqtt_client_fsm &get_instance();

  private:
  mqtt_client_fsm_connecting() {}
  mqtt_client_fsm_connecting(const mqtt_client_fsm_connecting &other);
  mqtt_client_fsm_connecting &
    operator=(const mqtt_client_fsm_connecting &other);
};

class mqtt_client_fsm_connected : public mqtt_client_fsm
{
  public:
  void enter(mqtt_client *client_instance) override;
  void exit(mqtt_client *client_instance) override;
  void event(int incoming_event, mqtt_client *client_instance) override;
  static mqtt_client_fsm &get_instance();

  private:
  mqtt_client_fsm_connected() {}
  mqtt_client_fsm_connected(const mqtt_client_fsm_connected &other);
  mqtt_client_fsm_connected &operator=(const mqtt_client_fsm_connected &other);
};

#endif  // MQTT_CLIENT_FSM_H
