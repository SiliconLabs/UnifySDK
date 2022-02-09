This repository holds binary resource dependencies for UIC

# Overview of Artifacts

## Silicon Labs Configurator(SLC)
slc is a tool used to create Silicon Labs Universal Configurator(UC) projects.

Artifact paths:
- linux/slc_cli_linux.zip
- mac/slc_cli_mac.zip

See [UG520: Software Project Generation and
Configuration with SLC-CLI](https://www.silabs.com/documents/public/user-guides/ug520-software-project-generation-configuration-with-slc-cli.pdf) for more details.


## ZCL Configurator (ZAP)
zap is a tool used to generate source code to handle ZCL cluster command and attribute consumption.
Build information: ZAP internal build #[490](https://jenkins-cbs-gecko-sdk.silabs.net/job/ZAP/job/silabs/490/)

Artifact paths:
- linux/zap-2022.1.10_amd64.deb
- linux/zap_apack_linux.zip
- mac/zap-2022.1.10.dmg
- mac/zap_apack_mac.zip


See [AN1325: Zigbee Cluster Configurator User's Guide](https://www.silabs.com/documents/public/application-notes/an1325-zigbee-cluster-configurator-users-guide.pdf) for more details.

> ### **WARNING**: The ZAP adaptor pack archives have been modified according to the changes below. Ensure these changes are made on subsequent updates to these archives.

### Modifications Made to Adapter Pack

Change in apack.json: Add `--no-sandbox` parameter in the `uc_generate` function:
```diff
diff --git a/apack.json b/apack.json
index cf87a910..8bce7281 100644
--- a/apack.json
+++ b/apack.json
@@ -36,7 +36,7 @@
       "cmd": "$(zap) --version"
     },
     "uc_generate": {
-      "cmd": "$(zap) generate --noUi --noServer -o ${generationOutput} --zcl ${sdkRoot}/app/zcl/zcl-zap.json --generationTemplate ${sdkRoot}/protocol/zigbee/app/framework/gen-temp
late/gen-templates.json --in ${contentFolder}"
+      "cmd": "$(zap) generate --noUi --noServer --no-sandbox -o ${generationOutput} --zcl ${sdkRoot}/app/zcl/zcl-zap.json --generationTemplate ${sdkRoot}/protocol/zigbee/app
/framework/gen-template/gen-templates.json --in ${contentFolder}"
     },
     "uc_upgrade": {
       "cmd": "$(zap) convert --noUi --noServer --in ${contentFolder} --zcl ${sdkRoot}/app/zcl/zcl-zap.json --generationTemplate ${sdkRoot}/protocol/zigbee/app/framework/gen-template/g
en-templates.json"
```

#### Instructions to modify zip archives:

*ZIP_FILE = zap_apack_linux.zip OR zap_apack_mac.zip*

```bash
# Extract only apack.json from zip file
$ unzip -p ZIP_FILE.zip apack.json > apack.json
# Add --no-sandbox parameter to slc zap generate config
$ sed -i 's/--noServer -o/--noServer --no-sandbox -o/' apack.json
# Update apack.json in zip file
$ zip ZIP_FILE.zip apack.json
```
