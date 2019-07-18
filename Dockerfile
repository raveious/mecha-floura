FROM centos:7.6.1810

# Setup some of out environment variables for the toolchain
ENV IDF_PATH=/esp-idf
ENV PATH=$PATH:/xtensa-esp32-elf/bin:/esp-idf/tools

# Pull in the archive of the toolchain. Adding it from context, instead of using a URL, to more easily version the toolchain.
ADD esp32-elf-linux64-toolchain.tar.gz /

# Bringing in modified microxrceddsgen script with a fixed path in it, instead of the one from the repository
COPY scripts/microxrceddsgen /usr/local/bin

# Install some pre-req packages, including some things from EPEL like cmake 3+ and pip
RUN yum install -y epel-release && \
    yum update -y && \
    yum install -y \
        git \
        vim \
        make \
        cmake3 \
        gcc \
        gcc-c++ \
        gdb \
        gdb-gdbserver \
        openocd \
        wget \
        ncurses-devel \
        flex \
        bison \
        gperf \
        ninja-build \
        ccache \
        screen \
        unzip \
        java-1.8.0-openjdk \
        java-1.8.0-openjdk-devel \
        python \
        python-pip && \
    yum clean all

# Need newer version of cmake to be used in the place of the older version of cmake.
RUN ln -sv /usr/bin/cmake3 /usr/bin/cmake

# Get and install Gradle
RUN wget https://services.gradle.org/distributions/gradle-4.10.2-bin.zip && unzip gradle-4.10.2-bin.zip -d /opt/gradle/

# Cloning IDF
RUN git clone --recursive --depth 1 --branch v3.3-beta3 https://github.com/espressif/esp-idf.git /esp-idf

# Installing required packages for the IDF
RUN pip install --upgrade pip && \
    pip install -r /esp-idf/requirements.txt

RUN git clone --depth 1 --recursive https://github.com/eProsima/Micro-XRCE-DDS-Gen.git /Micro-XRCE-DDS-Gen
WORKDIR /Micro-XRCE-DDS-Gen

ENV GRADLE_HOME=/opt/gradle/gradle-4.10.2
ENV PATH=$PATH:$GRADLE_HOME/bin

RUN gradle build --stacktrace --info && cp /Micro-XRCE-DDS-Gen/share/microxrcedds/microxrceddsgen.jar /usr/local/lib

CMD /bin/bash
