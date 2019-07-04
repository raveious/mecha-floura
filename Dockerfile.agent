# Build environment for the application
FROM alpine:3.8

# Install build dependences
RUN apk add --update --no-cache git cmake make build-base gcc g++ linux-headers net-tools

# Build Micro XRCE DDS Agent
RUN git clone --depth 1 --recursive --single-branch https://github.com/eProsima/Micro-XRCE-DDS.git /micro
WORKDIR /micro/build
RUN cmake -DTHIRDPARTY=ON -DVERBOSE=ON -DEPROSIMA_BUILD_EXAMPLES=ON .. && make install

# Hot fix until the agent can be moved into a real location
WORKDIR /micro/build/uagent/src/agent-build
CMD ./MicroXRCEAgent $AGENT_CONN_TYPE -p $AGENT_CONN_PORT -d --disport $AGENT_DISC_PORT -v 6 < /dev/null

# Production container should only container the executable
#FROM alpine:3.8

#RUN apk add --update --no-cache libgcc libstdc++

# Bring in the default profile
#WORKDIR /agent
#COPY --from=builder /agent/build/DEFAULT_FASTRTPS_PROFILES.xml /agent/DEFAULT_FASTRTPS_PROFILES.xml

# Copy in the install artifacts
#COPY --from=builder /usr/local/include/* /usr/local/include/
#COPY --from=builder /usr/local/lib/* /usr/local/lib/
#COPY --from=builder /usr/local/share/* /usr/local/share/
#COPY --from=builder /usr/local/examples/* /usr/local/examples/FastRTPS/
#COPY --from=builder /usr/local/bin/* /usr/local/bin/

#CMD MicroXRCEAgent $AGENT_CONN_TYPE -p $AGENT_CONN_PORT -d --disport $AGENT_DISC_PORT < /dev/null
