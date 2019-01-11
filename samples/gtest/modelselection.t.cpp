#line 16 "./samples/gtest/gtest.cgfeature"

// gtest.cgfeature
#include <gtest>
#line 5 "./samples/gtest/modelselection.s.cgfeature"

#line 4 "./samples/gtest/instruments.s.cgfeature"


#line 6 "./samples/gtest/instruments.s.cgfeature"
// instruments.s.cgfeature
#include <bsl_unorderedmap.h>
#include <bsl_string.h>
#include <dlibcoretypes_model.h>
#include <dlibdealapi_contractapifactory.h>
#include <dlibdealapi_lexificontractapi.h>

#line 7 "./samples/gtest/modelselection.s.cgfeature"

// modelselection.s.cgfeature
#include <bsl_unordered_map.h>
#include <dlibcoretypes_model.h>
#include <dlibdealapi_contractapifactory.h>
#include <dlibdealapi_lexificontractapi.h>

#line 20 "./samples/gtest/gtest.cgfeature"

using namespace BloombergLP;

namespace { // Anon ns for helpers

namespace { const char LOG_CATEGORY[] = "MODELSELECTIONTEST_GTEST"; }

#line 15 "./samples/gtest/instruments.s.cgfeature"


#line 17 "./samples/gtest/instruments.s.cgfeature"

// instruments.s.cgfeature
void load_equities(
    bsl::unordered_map<bsl::string, ContractApi::Underlying> &underlyings,
    const cglang::utils::table<3> &data)
{
    for(size_t r = 0; r < data.rows(); r++) {
        underlyings.emplace(
            data.get(r, "name"), 
            dlibdealapi::ContractApi::Equity(data.get(r, "ticker"), data.get(r, "currency")));
    }
}

void load_libor_rates(
    bsl::unordered_map<bsl::string, ContractApi::Underlying> &underlyings,
    const cglang::utils::table<4> &data)
{
    for(size_t r = 0; r < data.rows(); r++) {
        underlyings.emplace(
            data.get(r, "name"), 
            dlibdealapi::ContractApi::Libor(
                data.get(r, "ticker"), 
                dlibcoretypes::BBSecurity::Libor(
                    data.get(r, "tenor"), 
                    data.get(r, "currency"))));
    }
}

void load_instruments(
    bsl::unordered_map<bsl::string, ContractApi::Underlying> &underlyings,
    bsl::vector<ContractApi::Underlying> &loaded_instruments,
    const cglang::utils::table<1> &data)
{
    for(size_t r = 0; r < data.rows(); r++) {
        auto f = underlyings.find(data.get(r, "instrument"));
        ASSERT_TRUE(f != underlyings.end());
        loaded_instruments.push_back(f.second);
    }
}


#line 28 "./samples/gtest/gtest.cgfeature"

} // anonymous namespace

namespace gtest { // Open the namespace for this module

#line 40 "./samples/gtest/gtest.cgfeature"


TEST(ModelSelectionTest, modelSelectionEQBasketIRSingleCcy)
{
    #line 16 "./samples/gtest/modelselection.s.cgfeature"

    #line 60 "./samples/gtest/instruments.s.cgfeature"

    
#line 62 "./samples/gtest/instruments.s.cgfeature"
    // instruments.s.cgfeature
    bsl::unordered_map<bsl::string, ContractApi::Underlying> underlyings;
    bsl::vector<ContractApi::Underlying> loaded_instruments;

#line 18 "./samples/gtest/modelselection.s.cgfeature"

    // modelselection.s.cgfeature
    bde::Date asOfDate;
    bde::Date expiryDate;
    bsl::vector<bsl::string> warnings;
    bsl::vector<bsl::string> models;

#line 45 "./samples/gtest/gtest.cgfeature"

    try
    {
            { 
         static const char data[] = "name\0ticker\0currency\0\0IBM\0IBM US Equity\0USD\0\0MSFT\0MSFT US Equity\0USD\0\0SPX\0SPX Index\0USD\0\0SX5E\0SX5E Index\0EUR\0\0\0";
         static const char *indexes[] = { data, data + 5, data + 12, data + 22, data + 26, data + 40, data + 45, data + 50, data + 65, data + 70, data + 74, data + 84, data + 89, data + 94, data + 105 };
         struct headers
         {
             enum
             {
                 name = 0,
                 ticker = 1,
                 currency = 2
             };
         };
         cglang::utils::table<3, char> tbl(indexes, 5);
#line 67 "./samples/gtest/instruments.s.cgfeature"
 load_equities(underlyings, tbl); 
    }
    { 
         static const char data[] = "name\0ticker\0currency\0tenor\0\0US0003M\0US0003M Index\0USD\03M\0\0\0";
         static const char *indexes[] = { data, data + 5, data + 12, data + 21, data + 28, data + 36, data + 50, data + 54 };
         struct headers
         {
             enum
             {
                 name = 0,
                 ticker = 1,
                 currency = 2,
                 tenor = 3
             };
         };
         cglang::utils::table<4, char> tbl(indexes, 2);
#line 68 "./samples/gtest/instruments.s.cgfeature"
 load_libor_rates(underlyings, tbl); 
    }
    { #line 26 "./samples/gtest/modelselection.s.cgfeature"
 asOfDate.set(2014, 10, 09); 
    }
    { #line 27 "./samples/gtest/modelselection.s.cgfeature"
 expiryDate.set(2014, 11, 08); 
    }
    { 
         static const char data[] = "instrument\0\0IBM\0\0MSFT\0\0US0003M\0\0\0";
         static const char *indexes[] = { data, data + 12, data + 17, data + 23 };
         struct headers
         {
             enum
             {
                 instrument = 0
             };
         };
         cglang::utils::table<1, char> tbl(indexes, 4);
#line 69 "./samples/gtest/instruments.s.cgfeature"
 load_instruments(underlyings, loaded_instruments, tbl); 
    }
    { 
         static const char data[] = "currency\0\0USD\0\0\0";
         static const char *indexes[] = { data, data + 10 };
         struct headers
         {
             enum
             {
                 currency = 0
             };
         };
         cglang::utils::table<1, char> tbl(indexes, 2);
#line 29 "./samples/gtest/modelselection.s.cgfeature"

    bsl::vector<bsl::string> currencies;
    for(size_t r = 0; r < tbl.rows(); r++) {
        currencies.push_back(tbl.get(r, "currency"));
    }
    select_models(asOfDate, expiryDate, loaded_instruments, currencies, models, warnings);
    }
    { #line 36 "./samples/gtest/modelselection.s.cgfeature"
 ASSERT_EQ(0, warnings.size()); 
    }
    { 
         static const char data[] = "model\0\0HULL_WHITE_1F_BS_BASKET\0\0HULL_WHITE_1F_LV\0\0LOCAL_VOLATILITY\0\0BLACK_SCHOLES\0\0HULL_WHITE_2F_BS_BASKET\0\0HULL_WHITE_2F_LV\0\0\0";
         static const char *indexes[] = { data, data + 7, data + 32, data + 50, data + 68, data + 83, data + 108 };
         struct headers
         {
             enum
             {
                 model = 0
             };
         };
         cglang::utils::table<1, char> tbl(indexes, 7);
#line 37 "./samples/gtest/modelselection.s.cgfeature"
 tbl.assert_equivalent(models.begin(), models.end()); 
    }

#line 49 "./samples/gtest/gtest.cgfeature"
    }
    catch(std::exception ex) {
        FAIL(ex.what());
    }
}


#line 40 "./samples/gtest/gtest.cgfeature"


TEST(ModelSelectionTest, modelSelectionEQBasketIRMultipleCcy)
{
    #line 16 "./samples/gtest/modelselection.s.cgfeature"

    #line 60 "./samples/gtest/instruments.s.cgfeature"

    
#line 62 "./samples/gtest/instruments.s.cgfeature"
    // instruments.s.cgfeature
    bsl::unordered_map<bsl::string, ContractApi::Underlying> underlyings;
    bsl::vector<ContractApi::Underlying> loaded_instruments;

#line 18 "./samples/gtest/modelselection.s.cgfeature"

    // modelselection.s.cgfeature
    bde::Date asOfDate;
    bde::Date expiryDate;
    bsl::vector<bsl::string> warnings;
    bsl::vector<bsl::string> models;

#line 45 "./samples/gtest/gtest.cgfeature"

    try
    {
            { 
         static const char data[] = "name\0ticker\0currency\0\0IBM\0IBM US Equity\0USD\0\0MSFT\0MSFT US Equity\0USD\0\0SPX\0SPX Index\0USD\0\0SX5E\0SX5E Index\0EUR\0\0\0";
         static const char *indexes[] = { data, data + 5, data + 12, data + 22, data + 26, data + 40, data + 45, data + 50, data + 65, data + 70, data + 74, data + 84, data + 89, data + 94, data + 105 };
         struct headers
         {
             enum
             {
                 name = 0,
                 ticker = 1,
                 currency = 2
             };
         };
         cglang::utils::table<3, char> tbl(indexes, 5);
#line 67 "./samples/gtest/instruments.s.cgfeature"
 load_equities(underlyings, tbl); 
    }
    { 
         static const char data[] = "name\0ticker\0currency\0tenor\0\0US0003M\0US0003M Index\0USD\03M\0\0\0";
         static const char *indexes[] = { data, data + 5, data + 12, data + 21, data + 28, data + 36, data + 50, data + 54 };
         struct headers
         {
             enum
             {
                 name = 0,
                 ticker = 1,
                 currency = 2,
                 tenor = 3
             };
         };
         cglang::utils::table<4, char> tbl(indexes, 2);
#line 68 "./samples/gtest/instruments.s.cgfeature"
 load_libor_rates(underlyings, tbl); 
    }
    { #line 26 "./samples/gtest/modelselection.s.cgfeature"
 asOfDate.set(2014, 10, 09); 
    }
    { #line 27 "./samples/gtest/modelselection.s.cgfeature"
 expiryDate.set(2014, 11, 08); 
    }
    { 
         static const char data[] = "instrument\0\0SPX\0\0SX5E\0\0US0003M\0\0\0";
         static const char *indexes[] = { data, data + 12, data + 17, data + 23 };
         struct headers
         {
             enum
             {
                 instrument = 0
             };
         };
         cglang::utils::table<1, char> tbl(indexes, 4);
#line 69 "./samples/gtest/instruments.s.cgfeature"
 load_instruments(underlyings, loaded_instruments, tbl); 
    }
    { 
         static const char data[] = "currency\0\0USD\0\0\0";
         static const char *indexes[] = { data, data + 10 };
         struct headers
         {
             enum
             {
                 currency = 0
             };
         };
         cglang::utils::table<1, char> tbl(indexes, 2);
#line 29 "./samples/gtest/modelselection.s.cgfeature"

    bsl::vector<bsl::string> currencies;
    for(size_t r = 0; r < tbl.rows(); r++) {
        currencies.push_back(tbl.get(r, "currency"));
    }
    select_models(asOfDate, expiryDate, loaded_instruments, currencies, models, warnings);
    }
    { #line 36 "./samples/gtest/modelselection.s.cgfeature"
 ASSERT_EQ(0, warnings.size()); 
    }
    { 
         static const char data[] = "model\0\0LOCAL_VOLATILITY\0\0BLACK_SCHOLES\0\0\0\0\0";
         static const char *indexes[] = { data, data + 7, data + 25, data + 40 };
         struct headers
         {
             enum
             {
                 model = 0
             };
         };
         cglang::utils::table<1, char> tbl(indexes, 4);
#line 37 "./samples/gtest/modelselection.s.cgfeature"
 tbl.assert_equivalent(models.begin(), models.end()); 
    }

#line 49 "./samples/gtest/gtest.cgfeature"
    }
    catch(std::exception ex) {
        FAIL(ex.what());
    }
}



#line 34 "./samples/gtest/gtest.cgfeature"

}


