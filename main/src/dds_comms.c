#include <uxr/client/client.h>
#include <ucdr/microcdr.h>
#include <freertos/task.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <esp_wifi.h>

#include "dds_comms.h"

// Transport information
#ifdef CONFIG_PROFILE_UDP_TRANSPORT
#define MAX_TRANSPORT_MTU UXR_CONFIG_UDP_TRANSPORT_MTU

uxrUDPTransport transport;
uxrUDPPlatform platform;

#elif defined(CONFIG_PROFILE_TCP_TRANSPORT)
#define MAX_TRANSPORT_MTU UXR_CONFIG_TCP_TRANSPORT_MTU

uxrTCPTransport transport;
uxrTCPPlatform platform;

#else
#error No transport was selected.
#endif

#define MAX_HISTORY     8
#define STREAM_HISTORY  4
#define BUFFER_SIZE     MAX_TRANSPORT_MTU * MAX_HISTORY

static uint8_t output_reliable_stream_buffer[BUFFER_SIZE];
static uint8_t  input_reliable_stream_buffer[BUFFER_SIZE];

typedef enum {
    CREATE_PARTICIPANT = 0,
    CREATE_TOPIC,
    CREATE_SUBSCRIBER,
    CREATE_DATA_READER,
    LISTEN
} xrce_state_t;

// Handle to save for the dds task
TaskHandle_t dds_data_task = NULL;
// TaskHandle_t agent_discovery_task = NULL;

// Selected agent to use
uxrAgentAddress chosen_agent = { CONFIG_UXR_DISCOVERY_IP, CONFIG_UXR_DISCOVERY_PORT };

// Session that was created
uxrSession session;

// void on_agent_found(const uxrAgentAddress* address, void* args) {
//     (void) args;

//     printf("Found agent => ip: %s, port: %d\n", address->ip, address->port);
// }

// void discover_agent_task(void* found_agent_func) {
//     #if CONFIG_UXR_DISCOVERY_TYPE_UNICAST
//         /// TODO: Add support to list multiple IP addresses
//         size_t agent_list_size = 1;
//         uxrAgentAddress agent_list[agent_list_size];

//         agent_list[agent_list_size].ip   = CONFIG_UXR_DISCOVERY_IP;
//         agent_list[agent_list_size].port = CONFIG_UXR_DISCOVERY_PORT;

//         uxr_discovery_agents( CONFIG_UXR_DISCOVERY_ATTEMPTS,
//                                      CONFIG_UXR_DISCOVERY_PERIOD,
//                                      found_agent_func,
//                                      NULL,
//                                      agent_list,
//                                      agent_list_size
//                                     );

//     #else
//         uxr_discovery_agents_default( CONFIG_UXR_DISCOVERY_ATTEMPTS,
//                                              CONFIG_UXR_DISCOVERY_PERIOD,
//                                              found_agent_func,
//                                              NULL
//                                             );
//     #endif
// }

void on_session_status(uxrSession* session, uxrObjectId object_id, uint16_t request_id, uint8_t status, void* args)
{
    (void) session; (void) object_id; (void) request_id; (void) args;
    
    printf("XRCE Session Status: %u\n", status);
}

void on_topic_status(uxrSession* session, uxrObjectId object_id, uint16_t request_id, uxrStreamId stream_id, struct ucdrBuffer* serialization, void* args)
{
    (void) session; (void) object_id; (void) request_id; (void) stream_id; (void) serialization; (void) args;

    printf("Receiving... \n");
}

bool configure_transport(void) {

    // Setup the transport
    #ifdef CONFIG_PROFILE_UDP_TRANSPORT
    if (uxr_init_udp_transport(&transport, &platform, chosen_agent.ip, chosen_agent.port)) {
        printf("Successfully created UDP XRCE transport.\n");

        return true;
    }
    #else
    if (uxr_init_tcp_transport(&transport, &platform, chosen_agent.ip, chosen_agent.port)) {
        printf("Successfully created UDP XRCE transport.\n");

        return true;
    }
    #endif

    printf("Failed to create transport.\n");

    return false;
}

void dds_task(void* parms) {
    (void) parms;

    // Discover the agent that will be used for later
    // while (true) {
    //     if (discover_agent(&chosen_agent, &on_agent_found)) {
    //         // True -> The user returns true in the callback.
    //         printf("Chosen agent => ip: %s, port: %d\n", chosen_agent.ip, chosen_agent.port);
    //         break;
    //     } else {
    //         printf("Failed to discover an agent @ %lli.\n", esp_timer_get_time());
    //         fflush(stdout);

    //         vTaskDelay(1000 / portTICK_PERIOD_MS);

    //         //esp_restart();
    //     }
    // }

    // Configure the selected XRCE transport.
    if (!configure_transport()) {
        stop_dds_task();
    }

    // Get the mac address from the wifi radio
    uint8_t mac_addr[6];
    esp_wifi_get_mac(ESP_IF_WIFI_STA, mac_addr);

    // Take only the last 4 bytes of the MAC address, as those are going to be different from device to device.
    uint32_t session_id = 0;
    session_id |= mac_addr[2];
    session_id <<= 8;
    session_id |= mac_addr[3];
    session_id <<= 8;
    session_id |= mac_addr[4];
    session_id <<= 8;
    session_id |= mac_addr[5];

    // Setup a XRCE session with a callbacks
    uxr_init_session(&session, &transport.comm, session_id);
    uxr_set_status_callback(&session, on_session_status, NULL);
    uxr_set_topic_callback(&session, on_topic_status, NULL);

    // Establish the connection
    if (!uxr_create_session(&session))
    {
        printf("Failed to create session with agent.\n");
        printf("ID: %hhu\n", session.info.id);
        printf("Key: 0x%02hhX 0x%02hhX 0x%02hhX 0x%02hhX\n", session.info.key[0], session.info.key[1], session.info.key[2], session.info.key[3]);
        printf("Requested Status: %hu\n", session.info.last_requested_status);
        printf("Request ID: %hhu\n", session.info.last_request_id);
        
        // Stop the task if there are issues, nothing else is going to work if the session can't be created...
        stop_dds_task();
    }

    for(uint32_t i = 0; i < UXR_CONFIG_MAX_INPUT_RELIABLE_STREAMS; ++i)
    {
        (void) uxr_create_input_reliable_stream(&session, input_reliable_stream_buffer + (BUFFER_SIZE * i), transport.comm.mtu * STREAM_HISTORY, STREAM_HISTORY);
    }
    
    for(uint32_t i = 0; i < UXR_CONFIG_MAX_OUTPUT_RELIABLE_STREAMS; ++i)
    {
        (void) uxr_create_output_reliable_stream(&session, output_reliable_stream_buffer + (BUFFER_SIZE * i), transport.comm.mtu * STREAM_HISTORY, STREAM_HISTORY);
    }

    uxrStreamId input_stream   = uxr_stream_id(0, UXR_RELIABLE_STREAM, UXR_INPUT_STREAM);
    uxrStreamId output_stream  = uxr_stream_id(0, UXR_RELIABLE_STREAM, UXR_OUTPUT_STREAM);

    uxrObjectId participant_id = uxr_object_id(0x01, UXR_PARTICIPANT_ID);
    uxrObjectId topic_id       = uxr_object_id(0x01, UXR_TOPIC_ID);
    uxrObjectId subscriber_id  = uxr_object_id(0x01, UXR_SUBSCRIBER_ID);
    uxrObjectId datareader_id  = uxr_object_id(0x01, UXR_DATAREADER_ID);

    uxrDeliveryControl delivery_control = {0};
    delivery_control.max_samples = 1;

    // Create some state trackers to keep track of which stage we are in setting up the comms
    xrce_state_t conn_state = CREATE_PARTICIPANT;
    xrce_state_t next_conn_state = CREATE_TOPIC;

    uint8_t action_status = 0;
    uint16_t request_ref = 0;

    // task event loop
    while(true) {
        // Process the different connection states
        switch (conn_state)
        {
        case CREATE_PARTICIPANT:
            request_ref = uxr_buffer_create_participant_ref(&session,
                                                            output_stream,
                                                            participant_id,
                                                            42,
                                                            "default_weather_participant",
                                                            UXR_REUSE | UXR_REPLACE);

            next_conn_state = CREATE_TOPIC;
            break;

        case CREATE_TOPIC:
            request_ref = uxr_buffer_create_topic_ref(&session,
                                                      output_stream,
                                                      topic_id,
                                                      participant_id,
                                                      "weather_conditions_topic",
                                                      UXR_REUSE | UXR_REPLACE);

            next_conn_state = CREATE_SUBSCRIBER;
            break;

        case CREATE_SUBSCRIBER:
            // There doesn't seem to be a ref version of this?
            // The example seems to just pass an empty string into the xml field...
            request_ref = uxr_buffer_create_subscriber_xml(&session,
                                                           output_stream,
                                                           subscriber_id,
                                                           participant_id,
                                                           "", // Example uses empty string, not sure why this works...
                                                           UXR_REUSE | UXR_REPLACE);

            next_conn_state = CREATE_DATA_READER;
            break;
        
        case CREATE_DATA_READER:
            request_ref = uxr_buffer_create_datareader_ref(&session,
                                                           output_stream,
                                                           datareader_id,
                                                           subscriber_id,
                                                           "weather_conditions_data_reader",
                                                           UXR_REUSE | UXR_REPLACE);

            next_conn_state = LISTEN;
            break;
        
        case LISTEN:
            request_ref = uxr_buffer_request_data(&session,
                                                  output_stream,
                                                  datareader_id,
                                                  input_stream,
                                                  &delivery_control);

            next_conn_state = LISTEN;
            break;
        
        default:
            printf("Unknown state reached.\n");
            break;
        }

        // uxr_run_session* will 
        if (uxr_run_session_until_all_status(&session, 1000, &request_ref, &action_status, 1)) {

            if (conn_state != next_conn_state) {
                printf("State transition %u -> %u\n", conn_state, next_conn_state);
                conn_state = next_conn_state;
            }
        } else {
            printf("Run session returned error.\n");
        }
    }
}

void start_dds_task(void) {
    if (dds_data_task == NULL) {
        // Create the task to run DDS out of.
        xTaskCreate(dds_task,
                    "DDS_TASK",
                    (8 * 1024), //STACK_SIZE,
                    NULL,
                    configMAX_PRIORITIES - 1,
                    &dds_data_task
                    );
    } else {
        vTaskResume(dds_data_task);
    }
}

void stop_dds_task(void) {
    if (dds_data_task != NULL) {
        vTaskSuspend(dds_data_task);
    }
}