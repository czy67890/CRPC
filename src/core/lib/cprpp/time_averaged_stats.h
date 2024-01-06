//
// Created by chenzeyu on 2024/1/6.
//

#ifndef CZYSERVER_TIME_AVERAGED_STATS_H
#define CZYSERVER_TIME_AVERAGED_STATS_H
namespace crpc_core{

    class TimeAveragedStats{

    public:

        TimeAveragedStats(double init_avg,double regress_weight,double persistence_factor);


        void AddSample(double value);


        double UpdateAverage();

        double aggregate_weight_avg() const{
            return aggregate_weighted_avg_;
        }

        double aggregate_total_weight() const{
            return aggregate_total_weight_;
        }

    private:
        const double init_avg_;

        const double regress_weight_;

        const double persistence_factor_;

        double batch_total_value_{0};

        double batch_num_sample_{0};

        double aggregate_total_weight_{0};

        double aggregate_weighted_avg_{init_avg_};
    };



}



#endif //CZYSERVER_TIME_AVERAGED_STATS_H
