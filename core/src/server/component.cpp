#include <userver/server/component.hpp>

#include <userver/components/component.hpp>
#include <userver/components/statistics_storage.hpp>
#include <userver/server/server_config.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

USERVER_NAMESPACE_BEGIN

namespace components {

Server::Server(const components::ComponentConfig& component_config,
               const components::ComponentContext& component_context)
    : LoggableComponentBase(component_config, component_context),
      server_(std::make_unique<server::Server>(
          component_config.As<server::ServerConfig>(), component_context)) {
  auto& statistics_storage =
      component_context.FindComponent<StatisticsStorage>().GetStorage();
  server_statistics_holder_ = statistics_storage.RegisterExtender(
      "server", [this](const utils::statistics::StatisticsRequest& request) {
        return ExtendStatistics(request);
      });
  handler_statistics_holder_ = statistics_storage.RegisterExtender(
      "http.handler.total",
      [this](const utils::statistics::StatisticsRequest& /*request*/) {
        return server_->GetTotalHandlerStatistics();
      });
}

Server::~Server() {
  server_statistics_holder_.Unregister();
  handler_statistics_holder_.Unregister();
}

void Server::OnAllComponentsLoaded() { server_->Start(); }

void Server::OnAllComponentsAreStopping() {
  /* components::Server has to stop all Listeners before unloading components
   * as handlers have no ability to call smth like RemoveHandler() from
   * server::Server. Without such server stop before unloading a new request may
   * use a handler while the handler is destroying.
   */
  server_->Stop();
}

const server::Server& Server::GetServer() const { return *server_; }

server::Server& Server::GetServer() { return *server_; }

void Server::AddHandler(const server::handlers::HttpHandlerBase& handler,
                        engine::TaskProcessor& task_processor) {
  server_->AddHandler(handler, task_processor);
}

formats::json::Value Server::ExtendStatistics(
    const utils::statistics::StatisticsRequest& request) {
  return server_->GetMonitorData(request);
}

yaml_config::Schema Server::GetStaticConfigSchema() {
  return yaml_config::MergeSchemas<LoggableComponentBase>(R"(
type: object
description: server schema
additionalProperties: false
properties:
    logger_access:
        type: string
        description: set to logger name from components::Logging component to write access logs into it; do not set to avoid writing access logs
    logger_access_tskv:
        type: string
        description: set to logger name from components::Logging component to write access logs in TSKV format into it; do not set to avoid writing access logs
    max_response_size_in_flight:
        type: integer
        description: set it to the size of response in bytes and the component will drop bigger responses from handlers that allow trottling
    server-name:
        type: string
        description: value to send in HTTP Server header
        defaultDescription: value from utils::GetUserverIdentifier()
    listener:
        type: object
        description: describes the request processing socket
        additionalProperties: false
        properties:
            port:
                type: integer
                description: port to listen on
                defaultDescription: 0
            unix-socket:
                type: string
                description: unix socket to listen on instead of listening on a port
                defaultDescription: ''
            max_connections:
                type: integer
                description: max connections count to keep
                defaultDescription: 32768
            task_processor:
                type: string
                description: task processor to process incomming requests
            backlog:
                type: integer
                description: max count of new coneections pending acceptance
                defaultDescription: 1024
            connection:
                type: object
                description: connection options
                additionalProperties: false
                properties:
                    in_buffer_size:
                        type: integer
                        description: "size of the buffer to preallocate for request receive: bigger values use more RAM and less CPU"
                        defaultDescription: 32 * 1024
                    requests_queue_size_threshold:
                        type: integer
                        description: drop requests from handlers that allow trottling if there's more pending requests than allowed by this value
                        defaultDescription: 100
                    keepalive_timeout:
                        type: integer
                        description: timeout in seconds to drop connection if there's not data received from it
                        defaultDescription: 600
                    request:
                        type: object
                        description: request options
                        additionalProperties: false
                        properties:
                            type:
                                type: string
                                description: type of the request, only 'http' supported at the moment
                                defaultDescription: http
                                enum:
                                  - http
            shards:
                type: integer
                description: how many concurrent tasks harvest data from a single socket; do not set if not sure what it is doing
    listener-monitor:
        type: object
        description: describes the special monitoring socket, used for getting statistics and processing utility requests that should succeed even is the main socket is under heavy pressure
        additionalProperties: false
        properties:
            port:
                type: integer
                description: port to listen on
                defaultDescription: 0
            unix-socket:
                type: string
                description: unix socket to listen on instead of listening on a port
                defaultDescription: ''
            max_connections:
                type: integer
                description: max connections count to keep
                defaultDescription: 32768
            task_processor:
                type: string
                description: task processor to process incomming requests
            backlog:
                type: integer
                description: max count of new coneections pending acceptance
                defaultDescription: 1024
            connection:
                type: object
                description: connection options
                additionalProperties: false
                properties:
                    in_buffer_size:
                        type: integer
                        description: "size of the buffer to preallocate for request receive: bigger values use more RAM and less CPU"
                        defaultDescription: 32 * 1024
                    requests_queue_size_threshold:
                        type: integer
                        description: drop requests from handlers that allow trottling if there's more pending requests than allowed by this value
                        defaultDescription: 100
                    keepalive_timeout:
                        type: integer
                        description: timeout in seconds to drop connection if there's not data received from it
                        defaultDescription: 600
                    request:
                        type: object
                        description: request options
                        additionalProperties: false
                        properties:
                            type:
                                type: string
                                description: type of the request, only 'http' supported at the moment
                                defaultDescription: http
                                enum:
                                  - http
            shards:
                type: integer
                description: how many concurrent tasks harvest data from a single socket; do not set if not sure what it is doing
    set-response-server-hostname:
        type: boolean
        description: set to true to add the `X-YaTaxi-Server-Hostname` header with instance name, set to false to not add the header
        defaultDescription: false
)");
}

}  // namespace components

USERVER_NAMESPACE_END
