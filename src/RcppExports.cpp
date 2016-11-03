// Generated by using Rcpp::compileAttributes() -> do not edit by hand
// Generator token: 10BE3573-1514-4C36-9D1C-5A225CD40393

#include <Rcpp.h>

using namespace Rcpp;

// OSRMroute
Rcpp::List OSRMroute(Rcpp::DataFrame FromDF, Rcpp::DataFrame ToDF, std::string OSRMdata);
RcppExport SEXP OSRcppM_OSRMroute(SEXP FromDFSEXP, SEXP ToDFSEXP, SEXP OSRMdataSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< Rcpp::DataFrame >::type FromDF(FromDFSEXP);
    Rcpp::traits::input_parameter< Rcpp::DataFrame >::type ToDF(ToDFSEXP);
    Rcpp::traits::input_parameter< std::string >::type OSRMdata(OSRMdataSEXP);
    rcpp_result_gen = Rcpp::wrap(OSRMroute(FromDF, ToDF, OSRMdata));
    return rcpp_result_gen;
END_RCPP
}