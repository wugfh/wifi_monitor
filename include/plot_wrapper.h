#pragma once

#include "matplotlibcpp.h"

#ifdef __cplusplus
extern "C" {
#endif

int macrate_plot(std::vector<double> &us, std::vector<std::vector<double>> &rate, char** ip);
int packloss_mac_plot(std::vector<double> &us, std::vector<std::vector<double>> &loss, char** ip);
int packloss_app_plot(std::vector<double> &us, std::vector<std::vector<double>> &loss, char** ip);

#ifdef __cplusplus
}
#endif