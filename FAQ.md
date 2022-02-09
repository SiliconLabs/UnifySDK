# Unify FAQ

- *How can I see if the Unify components are running?*

  Unify applications are running as Linux `systemd` services. To get a status
  overview of all installed Unify components, run the following command:

  ``` sh
  systemctl status `ls /lib/systemd/system/uic-*.service | xargs -n 1 basename`
  ```

  The output of this command will tell if the service is loaded and/or is active:

  ``` sh
  - uic-dev-gui-api.service - Unify User Interface API
    Loaded: loaded (/lib/systemd/system/uic-dev-gui-api.service; enabled; vendor preset: enabled)
    Active: active (running) since Sun 2021-07-25 12:17:05 BST; 21min ago
  ```

- *How do I get logging of a specific Unify component?*

  Logging is collected via Linux `systemd`. In the terminal `journalctl` is able
  to print the collected logs from all Unify components. Getting the log of, for
  instance `uic-upvl`, is done via command:

  ``` sh
  journalctl -u uic-upvl
  ```

- *How do I clear Unify caches and databases?*

  Execute the following command in a terminal on the Unify gateway:

  ``` sh
  unify_services=`ls /lib/systemd/system/uic-*.service | xargs -n 1 basename`; sudo systemctl stop mosquitto $unify_services && sudo rm /var/lib/mosquitto/mosquitto.db /var/lib/uic/zpc.db /var/lib/uic/zigpc.db /var/lib/uic/upvl.db && sudo systemctl start mosquitto $unify_services
  ```

  >Note: database paths can vary depending on your configuration

- *I cant access the Dev UI/ the Dev UI is not responding anymore!*

  1. Test if the gateway can still be reached over TCP/IP. Ping the ip address of
     the gateway.
  2. Open a terminal on the Unify gateway, check the current status of the
     `uic-dev-gui-api` and `uic-dev-gui-client`. (See previous questions how to
     print the status)
  3. If status is inactive, restart the UI:

    ``` sh
    sudo systemctl restart uic-dev-gui-*
    ```

- *My Z-Wave device doesn't seem to include with my gateway*

  1. Make sure your device and gateway match frequency region. i.e., an EU device
     is incompatible with an US gateway. Information of the region can be find on
     the packaging of the device.

     To verify the rf region on your gateway, execute the following command in
     your gateway terminal:

      ``` sh
      cat /etc/uic/uic.cfg
      ```

      It outputs the configured rf_region:

      ```bash
      pi@raspberrypi:/etc/uic $ cat uic.cfg
      zpc:
        serial: /dev/ttyUSB0
        rf_region: US
        datatore_file: /var/lib/uic/zpc.db
      ```

  2. Make sure your device isn't still connected to a previous network. To remove
     a previous inclusion, hit the action button on your product and put the
     gateway in remove state.
  3. Power-cycling your device could make it to include.
  4. To change the frequency region of the gateway, see the startup options for
     the ZPC.

- *The gateway doesn't respond anymore!*

  1. It could be that the gateway experiences some delays while communicating with
     the Z-Wave node (missing Acknowledgment/Report, routing issues...). In this
     case, it may look like the gateway does not react to IoT Services commands.
     You could verify that the gateway still responds by modifying the state of
     another Z-Wave device in the gateway and see if the gateway responds to these
     changes. A way to modify state is to open MQTT explorer and to publish a
     command to a cluster for a particular node.
  2. As last resort, try to restart the Unify gateway.

- *How do I run the ZPC in isolation?*

  Stop the ZPC systemd service:

  ``` sh
  sudo systemctl stop uic-zpc
  ```

  run:

  ``` sh
  /usr/bin/zpc --zpc.serial /dev/ttyUSB0
  ```

  Replace the serial argument with the port the Z-Wave controller is connected to.

- *I have an old Z-Wave device, is it still supported?*

  Yes. However, older devices may be inconsistent in reporting their actual
  state. This usually means that operating the same command for a second time,
  the state get updated correctly.

- *Can I get detailed diagnostics about devices, its clusters and the
  network?*

  Using application [`mqtt explorer`](http://mqtt-explorer.com) can give more
  insights in your Unify network.

- *How do I determine the root cause for my failed firmware update?*

  The lastError field in the OTA tab of the dev_gui is giving a rough error
  code. The logging of the uic-zpc usually gives a more detailed explanation of
  the reason why a firmware update is not accepted.

- *My device shows up with more clusters than expected!*

  This is intended behavior. A protocol cluster can overlap in functionality
  with multiple UCL clusters. Rather then mapping to one, The Unify gateway
  exposes the protocol cluster to multiple clusters.

- *Can't connect 3rd party MQTT client to my device*

  This is because the Mosquitto broker by-default requires authentication and
  doesn’t listen on a network address. To change this behavior, create a file
  `/etc/mosquitto/conf.d/open_access.conf` and fill it with:

  ``` txt
  listener 1883
  allow_anonymous true
  ```

  Read more [here](http://www.steves-internet-guide.com/mosquitto-broker/).

### Development

- *Is mac supported for developing?*

  Our software should build and run on a mac machine. However, it is not
  officially supported.

### ZPC

- *ZPC complains about var/lib/uic/zpc.db permissions, How do I fix it?*

  This means your probably running the ZPC as an executable and not as a
  `systemd` service. Advised is to run ZPC with a database path that is writable
  in this case:

  ``` sh
  /usr/bin/zpc --zpc.datastore_file ~/zpc.db
  ```

- *Inclusion of a device keeps failing* try to reset the UPVL database:

  ``` sh
  sudo systemctl stop uic-upvl && rm /var/lib/uic/upvl.db && sudo systemctl start uic-upvl
  ```

## Known issues and fixes

### Windows (git Bash)

#### "winpty" issue

``` sh
user@winmachine MINGW64 ~/path/to/UnifySDK (main)
$ docker run -it --rm  -v`pwd`:`pwd` -w `pwd` uic_armhf
the input device is not a TTY.  If you are using mintty, try prefixing the command with 'winpty'
```

Workaround: add a `winpty` prefix to the command:

``` sh
winpty docker run -it --rm  -v`pwd`:`pwd` -w `pwd` uic_armhf
```

#### The "Docker directory path" issue

``` sh
user@winmachine MINGW64 ~/path/to/UnifySDK (main)
$ winpty docker run -it --rm  -v`pwd`:`pwd` -w `pwd` uic_armhf
docker: Error response from daemon: the working directory 'C:/path/to/UnifySDK' is invalid, it needs to be an absolute path.
See 'docker run --help'.
```

Workarround: add the `/` symbol before `pwd`:

``` sh
user@winmachine MINGW64 ~/path/to/UnifySDK (main)
$ winpty docker run -it --rm  -v/`pwd`:/`pwd` -w /`pwd` uic_armhf
user@dockercontainer:/c/path/to/UnifySDK$
```

Explanation:
[link](https://stackoverflow.com/questions/40213524/using-absolute-path-with-docker-run-command-not-working#comment109755641_40214650).

#### The "CRLF" issue

After running `ninja` inside the Docker container you may explore a lot of
`expected primary expression before 'case:'` errors. This is because of Windows
line endings (CRLF vs Linux's CR-only).

Workarround: run in your repository folder

``` sh
git config --global core.autocrlf input
git rm --cached -r .
git reset --hard
```

Explanations:
[link](https://stackoverflow.com/questions/1967370/git-replacing-lf-with-crlf).