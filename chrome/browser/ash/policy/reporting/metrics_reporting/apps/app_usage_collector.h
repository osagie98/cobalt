// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ASH_POLICY_REPORTING_METRICS_REPORTING_APPS_APP_USAGE_COLLECTOR_H_
#define CHROME_BROWSER_ASH_POLICY_REPORTING_METRICS_REPORTING_APPS_APP_USAGE_COLLECTOR_H_

#include <memory>

#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/scoped_observation.h"
#include "base/time/time.h"
#include "base/unguessable_token.h"
#include "chrome/browser/apps/app_service/metrics/app_platform_metrics.h"
#include "chrome/browser/ash/policy/reporting/metrics_reporting/apps/app_platform_metrics_retriever.h"
#include "chrome/browser/profiles/profile.h"
#include "components/reporting/metrics/reporting_settings.h"
#include "components/services/app_service/public/cpp/app_types.h"

namespace reporting {

// Collector used to observe and collect app usage data from the
// `AppPlatformMetrics` component so it can be persisted in the pref store and
// reported subsequently.
class AppUsageCollector : public ::apps::AppPlatformMetrics::Observer {
 public:
  // Static helper that instantiates the `AppUsageCollector` for the given
  // profile using the specified `ReportingSettings`.
  static std::unique_ptr<AppUsageCollector> Create(
      Profile* profile,
      const ReportingSettings* reporting_settings);

  // Static test helper that instantiates the `AppUsageCollector` for the given
  // profile using the specified `ReportingSettings` and
  // `AppPlatformMetricsRetriever`.
  static std::unique_ptr<AppUsageCollector> CreateForTest(
      Profile* profile,
      const ReportingSettings* reporting_settings,
      std::unique_ptr<AppPlatformMetricsRetriever>
          app_platform_metrics_retriever);

  AppUsageCollector(const AppUsageCollector& other) = delete;
  AppUsageCollector& operator=(const AppUsageCollector& other) = delete;
  ~AppUsageCollector() override;

 private:
  AppUsageCollector(base::WeakPtr<Profile> profile,
                    const ReportingSettings* reporting_settings,
                    std::unique_ptr<AppPlatformMetricsRetriever>
                        app_platform_metrics_retriever);

  // Initializes the usage collector and starts observing app usage collection
  // tracked by the `AppPlatformMetrics` component (if initialized).
  void InitUsageCollector(::apps::AppPlatformMetrics* app_platform_metrics);

  // ::apps::AppPlatformMetrics::Observer:
  void OnAppUsage(const std::string& app_id,
                  ::apps::AppType app_type,
                  const base::UnguessableToken& instance_id,
                  base::TimeDelta running_time) override;

  // ::apps::AppPlatformMetrics::Observer:
  void OnAppPlatformMetricsDestroyed() override;

  // Aggregates the app usage entry with the specified usage/running time and
  // persists it in the pref store. Creates a new placeholder entry if one does
  // not exist for the specified instance.
  void CreateOrUpdateAppUsageEntry(const std::string& app_id,
                                   ::apps::AppType app_type,
                                   const base::UnguessableToken& instance_id,
                                   const base::TimeDelta& running_time);

  // Weak pointer to the user profile. Used to save usage data to the user pref
  // store.
  const base::WeakPtr<Profile> profile_;

  // Pointer to the reporting settings component that outlives the
  // `AppUsageCollector`. Used to control usage data collection.
  const raw_ptr<const ReportingSettings> reporting_settings_;

  // Retriever that retrieves the `AppPlatformMetrics` component so the usage
  // collector can start tracking app usage collection.
  const std::unique_ptr<AppPlatformMetricsRetriever>
      app_platform_metrics_retriever_;

  // Observer for tracking app usage collection. Will be reset if the
  // `AppPlatformMetrics` component gets destructed before the usage collector.
  base::ScopedObservation<::apps::AppPlatformMetrics,
                          ::apps::AppPlatformMetrics::Observer>
      observer_{this};

  base::WeakPtrFactory<AppUsageCollector> weak_ptr_factory_{this};
};

}  // namespace reporting

#endif  // CHROME_BROWSER_ASH_POLICY_REPORTING_METRICS_REPORTING_APPS_APP_USAGE_COLLECTOR_H_
