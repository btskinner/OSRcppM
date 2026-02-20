#include <Rcpp.h>
#include "osrm/match_parameters.hpp"
#include "osrm/nearest_parameters.hpp"
#include "osrm/route_parameters.hpp"
#include "osrm/table_parameters.hpp"
#include "osrm/trip_parameters.hpp"
#include "osrm/coordinate.hpp"
#include "osrm/engine_config.hpp"
#include "osrm/json_container.hpp"
#include "osrm/osrm.hpp"
#include "osrm/status.hpp"
#include <exception>
#include <iostream>
#include <string>
#include <utility>
#include <cstdlib>

const double M2MILES = 0.00062137119223733;

// [[Rcpp::export]]
Rcpp::List osrm_route(double &xlon,
                      double &xlat,
                      double &ylon,
                      double &ylat,
                      std::string& osmfile,
                      const std::string& algo = "CH"
                      ) {

  // set up configuration based on pre-compilied OSRM data; osm_file should be
  // the extracted, partitioned, and customized suite of data files ending with
  // *.osrm; eg., ../path/to/united-states-latest.osrm
  osrm::EngineConfig config;
  config.storage_config = {osmfile};
  config.use_shared_memory = false;
  if (algo == "MLD") {
    config.algorithm = osrm::EngineConfig::Algorithm::MLD;
  } else {
    config.algorithm = osrm::EngineConfig::Algorithm::CH;
  }
  const osrm::OSRM osrm{config};

  // init route parameters
  osrm::RouteParameters params;

  params.coordinates.push_back({osrm::util::FloatLongitude{xlon},
      osrm::util::FloatLatitude{xlat}});
  params.coordinates.push_back({osrm::util::FloatLongitude{ylon},
      osrm::util::FloatLatitude{ylat}});

  // init JSON response object
  osrm::engine::api::ResultT result = osrm::json::Object();

  // execute routing request
  const auto status = osrm.Route(params, result);
  auto &json_result = std::get<osrm::json::Object>(result);
  if (status == osrm::Status::Error) {
    const auto code = std::get<osrm::json::String>(json_result.values["code"]).value;
    const auto message = std::get<osrm::json::String>(json_result.values["message"]).value;
    std::cout << "Code: " << code << "\n";
    std::cout << "Message: " << message << "\n";
    Rcpp::stop("");
  }
  auto &routes = std::get<osrm::json::Array>(json_result.values["routes"]);
  // take first response which is shortest (?) trip
  auto &route = std::get<osrm::json::Object>(routes.values.at(0));
  const auto distance = std::get<osrm::json::Number>(route.values["distance"]).value;
  const auto duration = std::get<osrm::json::Number>(route.values["duration"]).value;
  // store in list
  return Rcpp::List::create(Rcpp::_["miles"] = distance * M2MILES,
                            Rcpp::_["minutes"] = duration / 60.0);
}

// [[Rcpp::export]]
double osrm_minutes(double &xlon,
                    double &xlat,
                    double &ylon,
                    double &ylat,
                    std::string& osmfile,
                    const std::string& algo = "CH"
                    ) {
  Rcpp::List route = osrm_route(xlon, xlat, ylon, ylat, osmfile, algo);
  return route["minutes"];
}

// [[Rcpp::export]]
double osrm_miles(double &xlon,
                  double &xlat,
                  double &ylon,
                  double &ylat,
                  std::string& osmfile,
                  const std::string& algo = "CH"
                  ) {
  Rcpp::List route = osrm_route(xlon, xlat, ylon, ylat, osmfile, algo);
  return route["miles"];
}

// [[Rcpp::export]]
Rcpp::NumericVector osrm_route_vec(Rcpp::NumericVector& xlon,
                                   Rcpp::NumericVector& xlat,
                                   Rcpp::NumericVector& ylon,
                                   Rcpp::NumericVector& ylat,
                                   std::string& osmfile,
                                   const std::string& measure = "miles",
                                   const std::string& algo = "CH"
                                   ) {
  // set metric
  const auto metric = (measure == "miles") ? "distance" : "duration";
  osrm::EngineConfig config;
  config.storage_config = {osmfile};
  config.use_shared_memory = false;
  if (algo == "MLD") {
    config.algorithm = osrm::EngineConfig::Algorithm::MLD;
  } else {
    config.algorithm = osrm::EngineConfig::Algorithm::CH;
  }
  const osrm::OSRM osrm{config};

  // init out vector
  int n = xlon.size();
  Rcpp::NumericVector outvec(n);

  for (int i=0; i<n; i++) {
    // init route parameters
    osrm::RouteParameters params;
    params.coordinates.push_back({osrm::util::FloatLongitude{xlon[i]},
        osrm::util::FloatLatitude{xlat[i]}});
    params.coordinates.push_back({osrm::util::FloatLongitude{ylon[i]},
        osrm::util::FloatLatitude{ylat[i]}});
    // init JSON response object
    osrm::engine::api::ResultT result = osrm::json::Object();
    // execute routing request
    const auto status = osrm.Route(params, result);
    auto &json_result = std::get<osrm::json::Object>(result);
    if (status == osrm::Status::Error) {
      outvec[i] = -1.0;
    } else {
      auto &routes = std::get<osrm::json::Array>(json_result.values["routes"]);
      // take first response which is shortest (?) trip
      auto &route = std::get<osrm::json::Object>(routes.values.at(0));
      auto val = std::get<osrm::json::Number>(route.values[metric]).value;
      if (measure == "miles") {
        val = val * M2MILES;
      } else {
        val = val / 60.0;
      }
     outvec[i] = val;
    }
  }
  return outvec;
}
