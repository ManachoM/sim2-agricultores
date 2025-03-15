#ifndef _POSTGRES_AGGREGATED_MONITOR_
#define _POSTGRES_AGGREGATED_MONITOR_

#include "glob.h"
#include "monitor.h"

class PostgresAggregatedMonitor final : public Monitor
{
private:
  short last_recorded_month = 0;
  std::map<std::string, std::map<int, std::map<std::string, double>>>
      agg_logs; /** Objeto que almacenará los valores por mes*/
  std::string get_current_timestamp();
  std::string _database_url;
  int execution_id;
  std::vector<SSTimeRecord> time_records;
  std::vector<SSEventRecord> event_records;

  void gather_records();

public:
  PostgresAggregatedMonitor(
      std::string const &db_url = "", bool _debug = false,
      const int record_amount = 300
  );
  void write_log(json &log) override;
  void write_duration(double t) override;
  void write_results() override;
  void write_params(const std::string &key, const std::string &value) override;
  void add_event_record(SSEventRecord e) override;
  void add_time_record(SSTimeRecord e) override;
  ~PostgresAggregatedMonitor() final;
};

#endif // !_POSTGRES_AGGREGATED_MONITOR_
