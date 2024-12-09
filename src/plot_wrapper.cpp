#include <sys/time.h>
#include <string.h>
#include <vector>
#include <stdlib.h>
#include "plot_wrapper.h"

namespace plt = matplotlibcpp;

using std::vector;

int macrate_plot(vector<double> &us, vector<vector<double>> &rate, char** ip){
    plt::figure(1);
    plt::clf();
    size_t stream_cnt = rate.size();
    for(size_t i = 0; i < stream_cnt; ++i){
        plt::named_plot(ip[i], us, rate[i]);
    }
    // fprintf(stderr, "plot done\n");
    plt::xlabel("time");
    plt::ylabel("mac rate");
    plt::title("rate");
    plt::legend();
    plt::pause(0.1);
    return 0;
}

int packloss_mac_plot(vector<double> &us, vector<vector<double>> &loss, char** ip){
    plt::figure(2);
    plt::clf();
    size_t stream_cnt = loss.size();
    for(size_t i = 0; i < stream_cnt; ++i){
        plt::named_plot(ip[i], us, loss[i]);
    }
    plt::xlabel("time");
    plt::ylabel("loss");
    plt::title("pack loss in mac");
    plt::legend();
    plt::pause(0.1);
    return 0;
}

int packloss_app_plot(vector<double> &us, vector<vector<double>> &loss, char** ip){
    plt::figure(3);
    plt::clf();
    size_t stream_cnt = loss.size();
    for(size_t i = 0; i < stream_cnt; ++i){
        plt::named_plot(ip[i], us, loss[i]);
    }
    plt::xlabel("time");
    plt::ylabel("loss");
    plt::title("pack loss in app");
    plt::legend();
    plt::pause(0.1);
    return 0;
}

