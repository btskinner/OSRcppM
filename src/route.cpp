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

// [[Rcpp::export]]
Rcpp::List OSRMroute(Rcpp::DataFrame FromDF,
                     Rcpp::DataFrame ToDF,
                     std::string OSRMdata
                     ) {

  using namespace osrm;

  // set up configuration based on pre-compilied OSRM data
  EngineConfig config;
  config.storage_config = {OSRMdata};
  config.use_shared_memory = false;
  OSRM osrm{config};

  // init vectors for coordinates
  Rcpp::NumericVector xlat = FromDF["lat"];
  Rcpp::NumericVector xlon = FromDF["lon"];
  Rcpp::NumericVector ylat = ToDF["lat"];
  Rcpp::NumericVector ylon = ToDF["lon"];

  int n = xlat.size();
  int k = ylat.size();
  Rcpp::NumericMatrix dist(n,k);
  Rcpp::NumericMatrix time(n,k);

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < k; j++) {

      // init route parameters
      RouteParameters params;

      params.coordinates.push_back({util::FloatLongitude{xlon[i]},
          util::FloatLatitude{xlat[i]}});
      params.coordinates.push_back({util::FloatLongitude{ylon[j]},
          util::FloatLatitude{ylat[j]}});

      // init JSON response object
      engine::api::ResultT result = json::Object();

      // compute route
      const auto status = osrm.Route(params, result);
      auto &json_result = std::get<json::Object>(result);
      auto &routes = std::get<json::Array>(json_result.values["routes"]);

      // take first response which is shortest (?) trip
      auto &route = std::get<json::Object>(routes.values.at(0));
      const auto distance = std::get<json::Number>(route.values["distance"]).value;
      const auto duration = std::get<json::Number>(route.values["duration"]).value;

      // store in matrices
      dist(i,j) = distance;
      time(i,j) = duration;
    }
  }

  // add row and column names
  Rcpp::CharacterVector rnms = FromDF["id"];
  Rcpp::CharacterVector cnms = ToDF["id"];
  Rcpp::List dimnms = Rcpp::List::create(rnms, cnms);
  dist.attr("dimnames") = dimnms;
  time.attr("dimnames") = dimnms;

  return Rcpp::List::create(Rcpp::Named("meters") = dist,
                            Rcpp::Named("seconds") = time);
}

// // [[Rcpp::export]]
// Rcpp::DataFrame OSRMrouteDF(Rcpp::DataFrame DF,
//                             std::string OSRMdata,
//                             std::string fromLon = "flon",
//                             std::string fromLat = "void name()lat",
//                             std::string toLon = "tlon",
//                             std::string toLat = "tlat"
//                             ) {

//   using namespace osrm;

//   // set up configuration based on pre-compilied OSRM data
//   EngineConfig config;
//   config.storage_config = {OSRMdata};
//   config.use_shared_memory = false;
//   OSRM osrm{config};

//   // init vectors for coordinates
//   Rcpp::NumericVector xlat = DF["flat"];
//   Rcpp::NumericVector xlon = DF["flon"];
//   Rcpp::NumericVector ylat = DF["tlat"];
//   Rcpp::NumericVector ylon = DF["tlon"];

//   int n = xlat.size();
//   Rcpp::NumericVector dist(n);
//   Rcpp::NumericVector time(n);

//   for (int i = 0; i < n; i++) {

//     // init route parameters
//     RouteParameters params;

//     params.coordinates.push_back({util::FloatLongitude{xlon[i]},
//         util::FloatLatitude{xlat[i]}});
//     params.coordinates.push_back({util::FloatLongitude{ylon[i]},
//         util::FloatLatitude{ylat[i]}});

//     // init JSON response object
//     json::Object result;

//     // compute route
//     const auto status = osrm.Route(params, result);
//     auto &routes = result.values["routes"].get<json::Array>();

//     // take first response which is shortest (?) trip
//     auto &route = routes.values.at(0).get<json::Object>();
//     const auto distance = route.values["distance"].get<json::Number>().value;
//     const auto duration = route.values["duration"].get<json::Number>().value;

//     // store in matrices
//     dist[i] = distance;
//     time[i] = duration;
//   }

//   // add row and column names
//   return Rcpp::DataFrame::create(Rcpp::Named("flon") = xlon,
//                                  Rcpp::Named("flat") = xlat,
//                                  Rcpp::Named("tlon") = ylon,
//                                  Rcpp::Named("tlat") = ylat,
//                                  Rcpp::Named("meters") = dist,
//                                  Rcpp::Named("seconds") = time);
// }

// // [[Rcpp::export]]
// Rcpp::NumericVector OSRMrouteVec(const Rcpp::NumericVector& xlon,
//                                  const Rcpp::NumericVector& xlat,
//                                  const Rcpp::NumericVector& ylon,
//                                  const Rcpp::NumericVector& ylat,
//                                  std::string OSRMdata,
//                                  std::string measure = "distance"
//                                  ) {

//   using namespace osrm;

//   // set up configuration based on pre-compilied OSRM data
//   EngineConfig config;
//   config.storage_config = {OSRMdata};
//   config.use_shared_memory = false;
//   OSRM osrm{config};

//   // init out vector
//   int n = xlat.size();
//   Rcpp::NumericVector out(n);

//   for (int i = 0; i < n; i++) {

//     // init route parameters
//     RouteParameters params;

//     params.coordinates.push_back({util::FloatLongitude{xlon[i]},
//         util::FloatLatitude{xlat[i]}});
//     params.coordinates.push_back({util::FloatLongitude{ylon[i]},
//         util::FloatLatitude{ylat[i]}});

//     // init JSON response object
//     json::Object result;

//     // compute route
//     const auto status = osrm.Route(params, result);
//     auto &routes = result.values["routes"].get<json::Array>();

//     // take first response which is shortest (?) trip
//     auto &route = routes.values.at(0).get<json::Object>();

//     // pull chosen measure (distance or duration)
//     const auto meas = route.values[measure].get<json::Number>().value;

//     // store in matrices
//     out[i] = meas;

//   }

//   return out;

// }
