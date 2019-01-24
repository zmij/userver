#include <logging/component.hpp>

#include <chrono>
#include <stdexcept>

// this header must be included before any spdlog headers
// to override spdlog's level names
#include <logging/spdlog.hpp>

#include <spdlog/async.h>

#include <engine/async.hpp>
#include <logging/log.hpp>
#include <logging/logger.hpp>
#include <logging/reopening_file_sink.hpp>
#include <yaml_config/value.hpp>

#include "config.hpp"

namespace components {
namespace {

std::chrono::seconds kDefaultFlushInterval{2};

}  // namespace

Logging::Logging(const ComponentConfig& config,
                 const ComponentContext& context) {
  const auto& yaml = config.Yaml();
  auto loggers = yaml["loggers"];
  auto loggers_full_path = config.FullPath() + ".loggers";
  yaml_config::CheckIsMap(loggers, loggers_full_path);
  auto fs_task_processor_name =
      yaml_config::ParseString(yaml, "fs-task-processor-name",
                               config.FullPath(), config.ConfigVarsPtr());
  fs_task_processor_ = &context.GetTaskProcessor(fs_task_processor_name);

  for (auto it = loggers.begin(); it != loggers.end(); ++it) {
    auto logger_name = it->first.as<std::string>();
    auto logger_full_path = loggers_full_path + '.' + logger_name;
    bool is_default_logger = logger_name == "default";

    auto logger_config = logging::LoggerConfig::ParseFromYaml(
        it->second, logger_full_path, config.ConfigVarsPtr());

    auto overflow_policy = spdlog::async_overflow_policy::overrun_oldest;
    if (logger_config.queue_overflow_behavior ==
        logging::LoggerConfig::QueueOveflowBehavior::kBlock) {
      overflow_policy = spdlog::async_overflow_policy::block;
    }

    auto file_sink =
        std::make_shared<logging::ReopeningFileSinkMT>(logger_config.file_path);
    std::shared_ptr<spdlog::details::thread_pool> tp;
    if (is_default_logger) {
      spdlog::init_thread_pool(logger_config.message_queue_size,
                               logger_config.thread_pool_size);
      tp = spdlog::thread_pool();
    } else {
      tp = std::make_shared<spdlog::details::thread_pool>(
          logger_config.message_queue_size, logger_config.thread_pool_size);
      thread_pools_.push_back(tp);
    }
    auto logger = std::make_shared<spdlog::async_logger>(
        logger_name, std::move(file_sink), tp, overflow_policy);
    logger->set_level(
        static_cast<spdlog::level::level_enum>(logger_config.level));
    logger->set_pattern(logger_config.pattern);
    logger->flush_on(
        static_cast<spdlog::level::level_enum>(logger_config.flush_level));

    if (is_default_logger) {
      auto old_default_logger = logging::SetDefaultLogger(std::move(logger));
      if (old_default_logger) {
        // Close file sinks
        for (auto s : old_default_logger->sinks()) {
          auto reop =
              std::dynamic_pointer_cast<logging::ReopeningFileSinkMT>(s);
          if (reop) {
            reop->Close();
          }
        }
      }
    } else {
      auto insertion_result =
          loggers_.emplace(std::move(logger_name), std::move(logger));
      if (!insertion_result.second) {
        throw std::runtime_error("duplicate logger '" +
                                 insertion_result.first->first + '\'');
      }
    }
  }
  flush_task_.Start("log_flusher",
                    utils::PeriodicTask::Settings(
                        std::chrono::duration_cast<std::chrono::milliseconds>(
                            kDefaultFlushInterval),
                        {}, logging::Level::kTrace),
                    GetTaskFunction());
}

Logging::~Logging() { flush_task_.Stop(); }

logging::LoggerPtr Logging::GetLogger(const std::string& name) {
  auto it = loggers_.find(name);
  if (it == loggers_.end()) {
    throw std::runtime_error("logger '" + name + "' not found");
  }
  return it->second;
}

namespace {
void ReopenAll(std::vector<spdlog::sink_ptr>& sinks) {
  for (auto s : sinks) {
    auto reop = std::dynamic_pointer_cast<logging::ReopeningFileSinkMT>(s);
    if (reop) {
      // TODO Handle exceptions here
      reop->Reopen(/* truncate = */ false);
    }
  }
};
}  // namespace

void Logging::OnLogRotate() {
  std::vector<engine::TaskWithResult<void>> tasks;
  tasks.reserve(loggers_.size() + 1);

  // this must be a copy as the default logger may change
  auto default_logger = logging::DefaultLogger();
  tasks.push_back(engine::CriticalAsync(*fs_task_processor_, ReopenAll,
                                        std::ref(default_logger->sinks())));

  for (const auto& item : loggers_) {
    tasks.push_back(engine::CriticalAsync(*fs_task_processor_, ReopenAll,
                                          std::ref(item.second->sinks())));
  }

  for (auto& task : tasks) {
    try {
      task.Get();
    } catch (const std::exception& e) {
      LOG_ERROR() << "Exception on log reopen: " << e.what();
    }
  }
}

void Logging::FlushLogs() {
  logging::DefaultLogger()->flush();
  for (auto& item : loggers_) {
    item.second->flush();
  }
}

}  // namespace components
