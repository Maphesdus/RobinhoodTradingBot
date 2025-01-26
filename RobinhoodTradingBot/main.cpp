#include <iostream>
#include <string>
#include <curl/curl.h>
#include <json/json.h>

using namespace std;

// Function Declarations:
size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* s);
string loginToRobinhood(const string& username, const string& password);
string getMarketData(const string& token, const string& symbol);


// Main:
int main() {
    string username = "your_username";
    string password = "your_password";

    cout << "Welcome to the Robinhood Trading Bot.\n" << endl;
    cout << "Please enter your Robinhood username (email): ";
    cin >> username;
    cout << "Please enter your Robinhood password: ";
    cin >> password;

    // Login to Robinhood
    string loginResponse = loginToRobinhood(username, password);
    if (loginResponse.empty()) {
        cerr << "Login failed!" << endl;
        return 1;
    }
    cout << "Login Response: " << loginResponse << endl;

    // Parse the login response to get the access token
    Json::Value root;
    Json::Reader reader;
    bool parsingSuccessful = reader.parse(loginResponse, root);
    if (!parsingSuccessful) {
        cerr << "Failed to parse login response!" << endl;
        return 1;
    }
    string accessToken = root["access_token"].asString();
    cout << "Access Token: " << accessToken << endl;

    // Get market data for a specific symbol
    string symbol = "AAPL";
    cout << "Specify a market symbol to get data for (example: AAPL): ";
    cin >> symbol;
    string marketData = getMarketData(accessToken, symbol);
    if (marketData.empty()) {
        cerr << "Failed to get market data!" << endl;
        return 1;
    }
    cout << "Market Data: " << marketData << endl;

    // Parse the market data
    parsingSuccessful = reader.parse(marketData, root);
    if (!parsingSuccessful) {
        cerr << "Failed to parse market data!" << endl;
        return 1;
    }
    string lastPrice = root["last_trade_price"].asString();
    cout << "Last trade price for " << symbol << " is: " << lastPrice << endl;

    // Implement your trading logic here
    // For example, you could check the price and decide to buy or sell

    return 0;
}


// Write Callback:
// Function to handle HTTP response
size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* s) {
    size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
    }
    catch (std::bad_alloc& e) {
        // Handle memory problem
        return 0;
    }
    return newLength;
}




// Login To Robinhood:
// Function to login to Robinhood
string loginToRobinhood(const string& username, const string& password) {
    CURL* curl;
    CURLcode res;
    string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        string postData = "username=" + username + "&password=" + password;             // Set the POST data
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.robinhood.com/oauth2/token/"); // Set the URL
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
            return "";
        }
    }
    return readBuffer;
}

// Login To Robinhood 2:
// Function to login to Robinhood
std::string loginToRobinhood2(const std::string& username, const std::string& password) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        // Set the URL
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.robinhood.com/oauth2/token/");

        // Set the POST data
        std::string postData = "username=" + username + "&password=" + password +
            "&grant_type=password&client_id=c82SH0WZOsabOXGP2sxqcj34FxkvfnWRZBKlBjFS";

        // Set the headers
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Set the POST options
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());

        // Set the callback function to handle the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // Perform the request
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            return "";
        }
    }
    return readBuffer;
}


// Get Market Data:
// Function to get market data
string getMarketData(const string& token, const string& symbol) {
    CURL* curl;
    CURLcode res;
    string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        string url = "https://api.robinhood.com/quotes/" + symbol + "/";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
            return "";
        }

        // Check for missing instruments
        Json::Value root;
        Json::Reader reader;
        if (reader.parse(readBuffer, root)) {
            if (root.isMember("missing_instruments")) {
                cerr << "Invalid symbol: " << symbol << endl;
                return "";
            }
        }
    }
    return readBuffer;
}

