//
// Created by chenzeyu on 2024/1/6.
//

#include "core/lib/cprpp/time_averaged_stats.h"

namespace crpc_core{

    TimeAveragedStats::TimeAveragedStats(double init_avg,double regress_weight,double persistence_factor)
        :init_avg_(init_avg),regress_weight_(regress_weight),persistence_factor_(persistence_factor)
    {

    }

    void TimeAveragedStats::AddSample(double value){
        batch_total_value_ += value;
        ++batch_num_sample_;
    }

    double TimeAveragedStats::UpdateAverage(){
        double weighted_sum = batch_total_value_;
        double total_weight = batch_num_sample_;

        if(regress_weight_ > 0){
            weighted_sum += regress_weight_ * init_avg_;
            total_weight += regress_weight_;
        }

        if(persistence_factor_ > 0){
            /// first we refresh the weight
            const double pre_sample_weight = persistence_factor_ * aggregate_total_weight_;
            weighted_sum += pre_sample_weight * aggregate_weighted_avg_;
            total_weight += pre_sample_weight;
        }

        aggregate_weighted_avg_ = (total_weight > 0) ? (weighted_sum / total_weight) : init_avg_;
        aggregate_total_weight_ = total_weight;


        /// here we have a clear to zero
        /// operator
        batch_num_sample_ = 0;
        batch_total_value_ = 0;
        return aggregate_weighted_avg_;
    }


}
