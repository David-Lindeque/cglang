# cglang
A compiler to generate **C++** code from **Gherkin** (C++ from Gherkin LANGuage). The Gherkin language was slightly enhanced in order to allow a flexible solution easily compatible with generation of **C++** code. Note that I've been using a VSCode Extension **vscode-cglang** for these code snippets.
See [vscode-cglang](https://github.com/David-Lindeque/vscode-cglang) for the extension (and the .vsix file and installation instructions).

## Getting Started
Download the binary for your OS from binaries/<i>version</i>/<i>Platform</i>/<i>binary</i>.

## Gherkin

https://en.wikipedia.org/wiki/Cucumber_(software)

The **Gherkin** language defines a **Feature**, and within that **Feature** a set
of **Scenario**s. A scenario can have any number of steps, but each step must be
a **Given**, **When** or **Then** step. When steps of the same kind follow each
other, then the keyword **And** can be used. The idea is to construct the 3 parts of
a test using these three step types, the three steps being **Arrange**, **Action**
and **Assert** (The triple-A of test authoring). A typical **Feature** will look
something like the following
```feature
# Comments can be created like this.
Feature: BankTests # cglang require feature names to be valid C++ identifiers.

Scenario: OpenAccount # cglang require scenario names to be valid C++ identifiers.
    Given an empty bank
    And a legal person called 'David' # The 'And' would be read as a 'Given'
    When I open a new bank account with account number 'ACC1234' for 'David'
    And I deposit £100 into account 'ACC1234'
    Then I expect no errors
    And I expect bank fees of £1.50 taken from 'ACC1234'
    And I expect a bank balance of £98.50 in 'ACC1234'
``` 
In essence, that is all there is to it. But, to make scenario authoring even
easier than this, two other constructs can be used. It is easy to imagine that
all tests in the above feature will require an empty bank account to start out
with. This statement can be move to a **Background** statement like so:
```feature
Feature: BankTests

Background: # Note that background statements do not have names
    Given an empty bank account # but are otherwize exactly the same as scenarios.
# cglang allow a background to be specified at most once, but can have any number
# of steps

# Then all scenario(s) following would not need to specify the step of the background.

Scenario: OpenAccount
    Given a legal person called 'David'
    When I open a new bank account with account number 'ACC1234' for 'David'
    And I deposit £100 into account 'ACC1234'
    Then I expect no errors
    And I expect bank fees of £1.50 taken from 'ACC1234'
    And I expect a bank balance of £98.50 in 'ACC1234'
``` 
The second construct to make scenario authoring even easier, is to create a scenario
outline with examples. This is **Gherkin**s way to create **parameterized** tests.
To create such a thing:
```feature
Feature: BankTests

Background:
    Given an empty bank account

Scenario Outline: DepositAndTransferSuccess
    Given a legal person called 'David'
    And a legal person called 'John'
    When I open a new bank account with account number 'A1' for 'David'
    And I open a new bank account with account number 'A2' for 'John'
    When I deposit <acc1o> into account '<acc1>'
    And I deposit <acc2o> into account '<acc2>'
    Then I expect no errors
    When I transfer <tx> from <f> to <t>
    Then I expect no errors
    And I expect bank fees of <acc1bf> taken from '<acc1>'
    And I expect a bank balance of £98.50 in 'ACC1234'

Examples:
    | id | acc1 | acc2 | acc1o | acc2o | tx  | f  | t  | acc1bf | acc2bf | acc1c  | acc2c  |
    | 1  | A1   | A2   | £100  | £50   | £25 | A1 | A2 | £1.50  | £0.75  | £73.50 | £74.25 |
    | 2  | A1   | A2   | £100  | £0    | £50 | A1 | A2 | £1.50  | £0.75  | £48.50 | £49.25 |

# Note that the first column of the 'Examples' table will be used to create a name
# for the parameterized tests by appending the value of the column to the back
# of the scenario outline name, after appending an underscore (_), for instance, the above would create two
# scenarios named DepositAndTransferSuccess_1 and DepositAndTransferSuccess_2.
```
The **Examples** clause contains an example of a **table**. There are two extensions
in **Gherkin** for step authoring. The first is to attach a **table** to a step, like
in this example:
```feature
Scenario: MyTableScenario
    Given the following set of transactions
    | Kind     | Party1 | Party2 | Amount |
    | Transfer | David  | John   | 100    |
    | Dividend | John   | ABC    | 1.5    |

# cglang requires the column names of tables to be valid C++ identifiers.
``` 
The second extension to step authoring is to attach a **multiline string** to a step, like
in the following example:
```feature
Scenario: MyMultilineTextScenario
    Given I do some operations on the bank account
    When I generate a bank statement report and file as 'report1'
    Then I expect the report 'report1' to have the following text
    """
    Bank Statement
    Owner: David
    Account Number: A1
    Date\tTransaction\tAmount\tBalance
    1 Jan 2000\tDeposit\t£100\t£100
    """
``` 
cglang does not support the notion of **attributes** yet.

## Extensions
The extensions to **Gherkin** implemented by **cglang** is all to enable easy **C++**
code generation of scenarios and scenario outlines using a flexible solution that
would not tie you down to a specific testing framework. The extensions basically
consist of three main skeletons, 1) a **%file** skeleton, 2) a **%test** skeleton
and 3) a set of **step** skeletons. The compiler will expect exactly one **%file**
skeleton and exactly one **%test** skeleton. It will process the **%file** skeleton by replacing tokens with calculated values. For instance, the **$tests** token will be replaced by the text of the
tests determined by using the **%test** skeleton and processing scenarios through
that skeleton. In the below example, the **%test** skeleton reference the token
**$steps** which is the text of the steps determine for the specific scenario.
The following is an oversimplified implementation which doesn't even use any
test framework (as an illustration).
```c++
%file
{
#include <string>
#include <iostream>
#include <unordered_map>

class Bank {
private:
    std::unordered_map<std::string, double> _balances;
public:
    bool try_open_account(const std::string &acct) {
        return _balances.emplace(acct, 0.0).second;
    }
    bool try_deposit(const std::string &acct, double amount) {
        auto f = _balances.find(acct);
        if (f == _balances.end()) {
            return false;
        }
        else {
            if (amount < 0 && f->second + amount < 0) {
                return false;
            }
            else {
                f->second += amount;
                return true;
            }
        }
    }
    bool try_get_balance(const std::string &acct, double &balance) {
        auto f = _balances.find(acct);
        if (f == _balances.end()) {
            return false;
        }
        else {
            balance = f->second;
            return true;
        }
    }
};

int main(int argc, char *argv[])
{
    int total = 0, total_passed = 0;

    $tests

    std::cout << "Total: " << total << std::endl;
    std::cout << "Passed: " << total_passed << std::endl;
    std::cout << "Failed: " << (total - total_passed) << std::endl;

    return 0;
}
}
``` 
The **%test** skeleton looks like this:
```c++
%test
{
    {
        total++;
        std::cout << "Running $feature.$scenario" << std::endl;
        Bank bank;
        bool error = false, passed = true;
        $steps
        if (passed) {
            total_passed++;
            std::cout << " - OK" << std::endl;
        }
        else {
            std::cout << " - FAILED" << std::endl;
        }
    }
}
``` 
In order to create a **step** skeleton, we use syntax like the following:
```
'I open a bank account with number \'(.+)\'' { error |= !bank.try_open_account("$1"); }
``` 
The compiler will interpret the text between the apostrophes as a regular expression, and match it to the text of the step. If a match is found, the capture groups of the regex will be available as **$1** for the first group, **$2** for the second, etc. and the text of the skeleton will be used as the text for the step in the generated code. If the step contains a **table** or a **multiline string**, then that will be available as **$0**. For **multiline string**, the type of **$0** is ```const char*```, and for a table, the type will be ```cglang::utils::table<n>``` where ```n``` is the number of columns of the table. A c++ header file (cglang.h) is available (https://github.com/David-Lindeque/cglang/blob/master/utils/cglang.h) that defines the template class. Note that each step **must** match exactly on regex.

### cglang::utils::table
This type supports the use of tables in the generated code. When a step has a
table attached, that table will be made available to the step skeleton as an
instance of this type. The type is defined as follows:
```c++
template<unsigned int _Cols, class _CharT = char>
struct table;
``` 
The compiler will also generate a struct called ```headers``` for each of these 
steps, scoped to the step. The ```headers``` struct will contain the indexes
of each column by name, for instance, the struct might look like this:
```c++
struct headers
{
    enum
    {
        name = 0,
        ticker = 1,
        currency = 2
    };
};
``` 
This helps authoring of step skeleton code, for instance, you might use it like
this
```c++
// A step skeleton with a table attached
{
    // Enumerate all rows of a specific column:
    for(auto it = $0.begin_column(headers::name, 1); it != $0.end_column(headers::name); ++it)
    {
        // *it is of type const char* or const wchar_t* (See -w)
    }

    // Create a set from a column:
    std::set<std::string> myset($0.begin_column(headers::name, 1), $0.end_column(headers::name));
    // The second parameter of the begin_column method is the offset (we're skipping the header with 1)

    // number of columns
    auto cols = $0.cols();
    // number of rows. Note that row 0 is the headers
    auto rows = $0.rows();

    // Get a specific cell
    auto value = $0.get(row, headers::name);
}
```

### defines
The user can create new **tokens** by using the **%define** operator. When the compiler see a **%define**, it'll immediately resolve all the tokens it can
in the body of the define, including tokens with the same name. This enables the following scenario:
```
%define headers
{

}

%file
{
$headers
}

%define headers
{
$headers
#include <myheader1.h>
}

%define headers
{
$headers
#include <myheader2.h>
}
``` 
Since the **%file** skeleton is processed only at the end of the compile process, the $headers token will only be queried at that time, but while the compiler process the file, it'll first assign an empty text to the $headers variable,
then, during the second %define of headers it'll paste the empty text for $headers
and append the ```#include <myheader1.h>```. The later, it'll append the ```#include <myheader2.h>```.

### tokens
The following tokens are available by default
* $feature: The name of the feature as found in the source file.
* $FEATURE: The name of the feature in uppercase.
* $scenario: The name of the scenario.
* $tests: The text of the tests available in the **%file** skeleton
* $steps: The text of the steps available in the **%test** skeleton
* $0: The reference to the **table** or **multiline string** available in any **step** skeleton or definition.
* $x: (where x > 0) is the text of the parameter captured during the regex match of group 'x'.
* $my_define: When you created a new variable by using the **%define** operator.

### Modules (%import)
The compiler support a very simple 'modules' solution using the **%import** operator. The compiler will perform a first scan of all files includes on the 
command-line and also the ones found from **%import** statements. Once that 
first scan is complete, the compiler will determine the order to process files,
and note that recursive references are not allowed. Another cgfeature file
can be referenced by either the relative path, or absolute path, and the 
.cgfeature extension is optional.

### Regex settings (%grammar & %case)
The regex matcher can be configured using the **%grammar** and **%case** operators. The following settings are available:
```
%grammar = ECMAScript
%grammar = basic
%grammar = extended
%grammar = awk
%grammar = grep
%grammar = egrep

%case = sensitive
%case = insensitive
```
See the C++ regex documentation for explanations of the grammar.

## Command-line tool
The command-line compiler (cglang) will process the file(s) specified on the command-line and generate the c++ output.

### Parameters
The following command-line parameters are available

| parameter | description
|-- |--
| -o | Specify the output filename
| -I | Specify an import search path
| -w | Specify that you want to use wide character tests
| --help | Request help of the command-line

An example:
```bash
cglang -I ../../gtest -I ../steps -w -o test.cpp test1.cgfeature test2.cgfeature
``` 
<small>See https://github.com/David-Lindeque/cglang for details about the tool.</small> 

<small>David Lindeque</small>
