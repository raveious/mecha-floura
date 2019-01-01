FROM centos:7.6.1810

# Setup some of out environment variables for the toolchain
ENV IDF_PATH=/esp-idf
ENV PATH=$PATH:/xtensa-esp32-elf/bin:/esp-idf/tools

# Pull in the archive of the toolchain. Adding it from context, instead of using a URL, to more easily version the toolchain.
ADD esp32-elf-linux64-toolchain.tar.gz /

#RUN apk add --no-cache --update git cmake make build-base gcc g++ linux-headers openssl openssl-dev openjdk8 gradle flex bison gperf grep gettext ncurses-dev python python-dev automake texinfo help2man libtool gawk py-cryptography autoconf sed

# Install some pre-req packages, including some things from EPEL like cmake 3+ and pip
RUN yum install -y epel-release && \
    yum update -y && \
    yum install -y \
        git \
        make \
        cmake3 \
        gcc \
        wget \
        ncurses-devel \
        flex \
        bison \
        gperf \
        ninja-build \
        ccache \
        screen \
        python \
        python-pip && \
    yum clean all

# Need newer version of cmake to be used in the place of the older version of cmake.
RUN ln -sv /usr/bin/cmake3 /usr/bin/cmake

# Cloning IDF
RUN git clone --recursive --depth 1 -b v3.1.1 https://github.com/espressif/esp-idf.git /esp-idf

# Installing required packages for the IDF
RUN pip install --upgrade pip && \
    pip install -r /esp-idf/requirements.txt

#RUN git clone --depth 1 -b xtensa-1.22.x https://github.com/espressif/crosstool-NG.git /crosstool-NG
#WORKDIR /crosstool-NG
#RUN ./bootstrap && ./configure --enable-local && make install && ./ct-ng xtensa-esp32-elf && ./ct-ng build

#RUN git clone --depth 1 --recursive https://github.com/eProsima/Micro-XRCE-DDS.git /Micro-XRCE-DDS
#WORKDIR /Micro-XRCE-DDS/build
#RUN cmake -DTHIRDPARTY=ON -DCOMPILE_EXAMPLES=ON .. && make && make install

CMD /bin/bash
