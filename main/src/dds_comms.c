#include <uxr/client/client.h>
#include <ucdr/microcdr.h>
#include <freertos/task.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <esp_wifi.h>

// Handle to save for the dds task
TaskHandle_t dds_data_task = NULL;
// TaskHandle_t agend_discovery_task = NULL;

// Selected agent to use
uxrAgentAddress chosen_agent = { CONFIG_UXR_DISCOVERY_IP, CONFIG_UXR_DISCOVERY_PORT };

// Session that was created
uxrSession session;

// Transport information
#ifdef CONFIG_PROFILE_UDP_TRANSPORT
uxrUDPTransport transport;
uxrUDPPlatform platform;
#elif defined(CONFIG_PROFILE_TCP_TRANSPORT)
uxrTCPTransport transport;
uxrTCPPlatform platform;
#else
#error No transport was selected.
#endif

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

    // Setup the transport
    #ifdef CONFIG_PROFILE_UDP_TRANSPORT
    if (!uxr_init_udp_transport(&transport, &platform, chosen_agent.ip, chosen_agent.port)) {
        printf("Failed to create transport.");
        // return;
    }
    #else
    if (!uxr_init_tcp_transport(&transport, &platform, chosen_agent.ip, chosen_agent.port)) {
        printf("Failed to create transport.");
        // return;
    }
    #endif

    uint8_t mac_addr[6];
    esp_wifi_get_mac(ESP_IF_WIFI_STA, mac_addr);

    uint32_t session_id = 0;
    session_id |= mac_addr[2];
    session_id <<= 8;
    session_id |= mac_addr[3];
    session_id <<= 8;
    session_id |= mac_addr[4];
    session_id <<= 8;
    session_id |= mac_addr[5];

    uxr_init_session(&session, &transport.comm, session_id);
    if (!uxr_create_session(&session))
    {
        printf("Failed to create session with agent.\n");
        printf("ID: %hhu\n", session.info.id);
        printf("Key: 0x%02hhX 0x%02hhX 0x%02hhX 0x%02hhX\n", session.info.key[0], session.info.key[1], session.info.key[2], session.info.key[3]);
        printf("Requested Status: %hu\n", session.info.last_requested_status);
        printf("Request ID: %hhu\n", session.info.last_request_id);
        // return;
    }

    printf("Node online!");
    fflush(stdout);
    while(true) {
        vTaskDelay(10000 / portTICK_PERIOD_MS);
        printf("Sleepy node ZZZZZZZzzzzzz......");
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