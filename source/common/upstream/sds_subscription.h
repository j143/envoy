#pragma once

#include <cstdint>
#include <string>

#include "envoy/config/subscription.h"

#include "common/common/assert.h"
#include "common/http/rest_api_fetcher.h"

#include "api/eds.pb.h"

namespace Envoy {
namespace Upstream {

/**
 * Subscription implementation that reads host information from the v1 REST Service Discovery
 * Service.
 */
class SdsSubscription : public Http::RestApiFetcher,
                        public Config::Subscription<envoy::api::v2::ClusterLoadAssignment>,
                        Logger::Loggable<Logger::Id::upstream> {
public:
  SdsSubscription(ClusterStats& stats, const SdsConfig& sds_config, ClusterManager& cm,
                  Event::Dispatcher& dispatcher, Runtime::RandomGenerator& random);

private:
  // Config::Subscription
  void
  start(const std::vector<std::string>& resources,
        Config::SubscriptionCallbacks<envoy::api::v2::ClusterLoadAssignment>& callbacks) override {
    // We can only handle a single cluster here, it's a design error to ever use this type of
    // Subscription with more than a single cluster.
    ASSERT(resources.size() == 1);
    cluster_name_ = resources[0];
    callbacks_ = &callbacks;
    RestApiFetcher::initialize();
  }

  void updateResources(const std::vector<std::string>& resources) override {
    // We should never hit this at runtime, since this legacy adapter is only used by EdsClusterImpl
    // that doesn't do dynamic modification of resources.
    UNREFERENCED_PARAMETER(resources);
    NOT_IMPLEMENTED;
  }

  // Http::RestApiFetcher
  void createRequest(Http::Message& request) override;
  void parseResponse(const Http::Message& response) override;
  void onFetchComplete() override;
  void onFetchFailure(const EnvoyException* e) override;

  std::string cluster_name_;
  Config::SubscriptionCallbacks<envoy::api::v2::ClusterLoadAssignment>* callbacks_;
  ClusterStats& stats_;
};

} // Upstream
} // Envoy
