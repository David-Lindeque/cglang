
// gtest.cgfeature
#include <gtest>




// instruments.s.cgfeature
#include <bsl_unorderedmap.h>
#include <bsl_string.h>
#include <dlibcoretypes_model.h>
#include <dlibdealapi_contractapifactory.h>
#include <dlibdealapi_lexificontractapi.h>


// modelselection.s.cgfeature
#include <bsl_unordered_map.h>
#include <dlibcoretypes_model.h>
#include <dlibdealapi_contractapifactory.h>
#include <dlibdealapi_lexificontractapi.h>


using namespace BloombergLP;

namespace { // Anon ns for helpers

namespace { const char LOG_CATEGORY[] = "MODELSELECTIONTEST_GTEST"; }





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



} // anonymous namespace

namespace gtest { // Open the namespace for this module



TEST(ModelSelectionTest, modelSelectionEQBasketIRSingleCcy)
{
    
    
    

    // instruments.s.cgfeature
    bsl::unordered_map<bsl::string, ContractApi::Underlying> underlyings;
    bsl::vector<ContractApi::Underlying> loaded_instruments;


    // modelselection.s.cgfeature
    bde::Date asOfDate;
    bde::Date expiryDate;
    bsl::vector<bsl::string> warnings;
    bsl::vector<bsl::string> models;


    try
    {
            { 
         static const wchar_t data[] = L"name\0ticker\0currency\0\0IBM\0IBM US Equity\0USD\0\0MSFT\0MSFT US Equity\0USD\0\0SPX\0SPX Index\0USD\0\0SX5E\0SX5E Index\0EUR\0\0\0";
         static const wchar_t *indexes[] = { data, data + 5, data + 12, data + 22, data + 26, data + 40, data + 45, data + 50, data + 65, data + 70, data + 74, data + 84, data + 89, data + 94, data + 105 };
         cglang::utils::table<3>(indexes, 5) tbl;
 load_equities(underlyings, tbl);     }
    { 
         static const wchar_t data[] = L"name\0ticker\0currency\0tenor\0\0US0003M\0US0003M Index\0USD\03M\0\0\0";
         static const wchar_t *indexes[] = { data, data + 5, data + 12, data + 21, data + 28, data + 36, data + 50, data + 54 };
         cglang::utils::table<4>(indexes, 2) tbl;
 load_libor_rates(underlyings, tbl);     }
    {  asOfDate.set(2014, 10, 09);     }
    {  expiryDate.set(2014, 11, 08);     }
    { 
         static const wchar_t data[] = L"instrument\0\0IBM\0\0MSFT\0\0US0003M\0\0\0";
         static const wchar_t *indexes[] = { data, data + 12, data + 17, data + 23 };
         cglang::utils::table<1>(indexes, 4) tbl;
 load_instruments(underlyings, loaded_instruments, tbl);     }
    { 
         static const wchar_t data[] = L"currency\0\0USD\0\0\0";
         static const wchar_t *indexes[] = { data, data + 10 };
         cglang::utils::table<1>(indexes, 2) tbl;

    bsl::vector<bsl::string> currencies;
    for(size_t r = 0; r < tbl.rows(); r++) {
        currencies.push_back(tbl.get(r, "currency"));
    }
    select_models(asOfDate, expiryDate, loaded_instruments, currencies, models, warnings);
    }
    {  ASSERT_EQ(0, warnings.size());     }
    { 
         static const wchar_t data[] = L"model\0\0HULL_WHITE_1F_BS_BASKET\0\0HULL_WHITE_1F_LV\0\0LOCAL_VOLATILITY\0\0BLACK_SCHOLES\0\0HULL_WHITE_2F_BS_BASKET\0\0HULL_WHITE_2F_LV\0\0\0";
         static const wchar_t *indexes[] = { data, data + 7, data + 32, data + 50, data + 68, data + 83, data + 108 };
         cglang::utils::table<1>(indexes, 7) tbl;
 tbl.assert_equivalent(models.begin(), models.end());     }

    }
    catch(std::exception ex) {
        FAIL(ex.what());
    }
}




TEST(ModelSelectionTest, modelSelectionEQBasketIRMultipleCcy)
{
    
    
    

    // instruments.s.cgfeature
    bsl::unordered_map<bsl::string, ContractApi::Underlying> underlyings;
    bsl::vector<ContractApi::Underlying> loaded_instruments;


    // modelselection.s.cgfeature
    bde::Date asOfDate;
    bde::Date expiryDate;
    bsl::vector<bsl::string> warnings;
    bsl::vector<bsl::string> models;


    try
    {
            { 
         static const wchar_t data[] = L"name\0ticker\0currency\0\0IBM\0IBM US Equity\0USD\0\0MSFT\0MSFT US Equity\0USD\0\0SPX\0SPX Index\0USD\0\0SX5E\0SX5E Index\0EUR\0\0\0";
         static const wchar_t *indexes[] = { data, data + 5, data + 12, data + 22, data + 26, data + 40, data + 45, data + 50, data + 65, data + 70, data + 74, data + 84, data + 89, data + 94, data + 105 };
         cglang::utils::table<3>(indexes, 5) tbl;
 load_equities(underlyings, tbl);     }
    { 
         static const wchar_t data[] = L"name\0ticker\0currency\0tenor\0\0US0003M\0US0003M Index\0USD\03M\0\0\0";
         static const wchar_t *indexes[] = { data, data + 5, data + 12, data + 21, data + 28, data + 36, data + 50, data + 54 };
         cglang::utils::table<4>(indexes, 2) tbl;
 load_libor_rates(underlyings, tbl);     }
    {  asOfDate.set(2014, 10, 09);     }
    {  expiryDate.set(2014, 11, 08);     }
    { 
         static const wchar_t data[] = L"instrument\0\0SPX\0\0SX5E\0\0US0003M\0\0\0";
         static const wchar_t *indexes[] = { data, data + 12, data + 17, data + 23 };
         cglang::utils::table<1>(indexes, 4) tbl;
 load_instruments(underlyings, loaded_instruments, tbl);     }
    { 
         static const wchar_t data[] = L"currency\0\0USD\0\0\0";
         static const wchar_t *indexes[] = { data, data + 10 };
         cglang::utils::table<1>(indexes, 2) tbl;

    bsl::vector<bsl::string> currencies;
    for(size_t r = 0; r < tbl.rows(); r++) {
        currencies.push_back(tbl.get(r, "currency"));
    }
    select_models(asOfDate, expiryDate, loaded_instruments, currencies, models, warnings);
    }
    {  ASSERT_EQ(0, warnings.size());     }
    { 
         static const wchar_t data[] = L"model\0\0LOCAL_VOLATILITY\0\0BLACK_SCHOLES\0\0\0";
         static const wchar_t *indexes[] = { data, data + 7, data + 25 };
         cglang::utils::table<1>(indexes, 3) tbl;
 tbl.assert_equivalent(models.begin(), models.end());     }

    }
    catch(std::exception ex) {
        FAIL(ex.what());
    }
}




}

