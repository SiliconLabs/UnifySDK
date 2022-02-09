FROM ubuntu:20.04
ENV TZ=Europe/Copenhagen
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        texlive-base texlive-latex-extra texlive-font-utils openjdk-13-jre \
        make doxygen texlive-fonts-recommended texlive-plain-generic ghostscript \
        git graphviz zip
