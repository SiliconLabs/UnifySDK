FROM ubuntu:20.04

ENV TZ=Europe/Copenhagen
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
    cmake ninja-build cmake python gcovr build-essential doxygen graphviz zip ca-certificates wget unzip \
    && rm -rf /var/lib/apt/lists/*
RUN mkdir /opt/sonarqube-wrapper \
    && cd opt/sonarqube-wrapper/ \
    && wget https://sonarqube.silabs.net/static/cpp/build-wrapper-linux-x86.zip \
    && unzip -d . build-wrapper-linux-x86.zip \
    && rm build-wrapper-linux-x86.zip \
    && ln -s /opt/sonarqube-wrapper/build-wrapper-linux-x86/build-wrapper-linux-x86-64 /usr/bin/build-wrapper
RUN mkdir /opt/SonarScanner \
    && cd opt/SonarScanner/ \
    && wget https://binaries.sonarsource.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-4.4.0.2170-linux.zip \
    && unzip -d . sonar-scanner-cli-4.4.0.2170-linux.zip \
    && rm sonar-scanner-cli-4.4.0.2170-linux.zip \
    && ln -s /opt/SonarScanner/sonar-scanner-4.4.0.2170-linux/bin/sonar-scanner /usr/bin/sonar-scanner
