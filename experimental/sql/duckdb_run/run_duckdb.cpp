#include "src/duckdb.hpp"
#include "tsc_x86.h"

#define NR 32
#define CYCLES_REQUIRED 1e10
#define REP 1
#define CYCLES_PER_SECOND 2.3e9

using namespace duckdb;

int benchmark(int64_t scale_factor) {
        DuckDB db(nullptr);
        Connection con(db);
        double start;
        double end;
        // double multiplier = 1;
        double cycles = 0;
        int64_t num_runs = 100;

        char data[95];
        snprintf(data, 95,
                 "COPY lineitem FROM "
                 "'./experimental/sql/tables/lineitem_%ld.csv' ( "
                 "DELIMITER ',', HEADER )",
                 scale_factor);

        con.Query(
            "CREATE TABLE lineitem(ORDERKEY INTEGER, PARTKEY INTEGER, SUPPKEY "
            "INTEGER, LINENUMBER INTEGER, QUANTITY INTEGER, EXTENDEDPRICE "
            "INTEGER, DISCOUNT INTEGER, TAX INTEGER, RETURNFLAG INTEGER, "
            "LINESTATUS INTEGER, SHIPDATE INTEGER, COMMITDATE INTEGER, "
            "RECEIPTDATE INTEGER, SHIPINSTRUCT INTEGER, SHIPMODE INTEGER, "
            "COMMENT STRING)");
        con.Query(data);
        // con.Query("PRAGMA threads=1");
        con.Query("SET threads TO 1;");
        con.Query("SET memory_limit='32GB';");

        std::unique_ptr<PreparedStatement> prepare = con.Prepare(
            "SELECT "
            "SUM(EXTENDEDPRICE * DISCOUNT) AS revenue "
            "FROM "
            "lineitem "
            "WHERE "
            "SHIPDATE >= 757382400 "
            "AND SHIPDATE < 788918400 "
            "AND DISCOUNT >= 5 "
            "AND DISCOUNT <= 7 "
            "AND QUANTITY < 24 ");

        double total_cycles = 0;
        printf("%ld\n", scale_factor);
        for (size_t j = 0; j < REP; j++) {
                for (size_t i = 0; i < num_runs; ++i) {
                        start = start_tsc();
                        std::unique_ptr<QueryResult> result =
                            prepare->Execute();
                        auto &stream = (StreamQueryResult &)*result;
                        stream.Materialize();
                        end = stop_tsc(start);
                        printf("%lf\n", ((double)end) / CYCLES_PER_SECOND);
                }

                cycles = ((double)end) / num_runs;
                total_cycles += cycles;
        }
        total_cycles /= REP;
        return 0;
}

int main(int argc, char *argv[]) {
        benchmark(1);
        benchmark(2);
        benchmark(4);
        benchmark(8);
        benchmark(16);
        return 0;
}
