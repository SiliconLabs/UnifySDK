# Unify Host SDK

This SDK contains non-embedded applications developed by Silicon Labs. The Host
SDK is the main source for Silicon Labs customers who are working with Linux
based applications in conjunction with Silicon Labs products.

When developing IoT products that leverage a Linux operating system and a
Silicon Labs radio, such as an IoT gateway or an advanced IoT end device,
additional software is needed to interface with the IoT radio and provide
interoperability with the host application. To equip customers developing
complex IoT applications for single or multi-protocol use cases, Silicon Labs
has created a single repository for all customer software needs on a Linux
gateway. Rather than pulling software from multiple repositories including the
[Gecko SDK](https://github.com/SiliconLabs/gecko_sdk/), we've simplified the distribution of customer-facing software to enhance
ease of use. We haven't changed what we're creating for customers, and the
protocol-specific host software you expect from us is still distributed
concurrently with the GSDK. But, each protocol supported by Silicon Labs has
corresponding host application software that we've also distributed with the
Host SDK for greater accessibility. Here is an overview of the components delivered with this repo.

![Unify Host SDK](doc/assets/img/unify_host_sdk.png)

## Documentation

You will find complete documentation for quickly evaluating the Unify Host SDK in the [Unify Host SDK documentation](https://siliconlabs.github.io/UnifySDK).
Here you will also learn how to build and run the various components that make up the Unify Host SDK.

## License

**Copyright 2021 Silicon Laboratories Inc. www.silabs.com**

The licensor of this software is Silicon Laboratories Inc. Your use of this software is governed by the terms of Silicon Labs Master Software License Agreement (MSLA) available at www.silabs.com/about-us/legal/master-software-license-agreement. This software is distributed to you in Source Code format and is governed by the sections of the MSLA applicable to Source Code.
