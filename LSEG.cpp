#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <map>
#include <iomanip> 
using namespace std;

struct Order {
    string cl_ord_id;
    string ordid;
    string instrument;
    int side; // 1 for buy, 2 for sell
    int quantity;
    string reson;
    double price;
    string status = "New";
};

struct OrderBook {
    vector<Order> buyOrders;
    vector<Order> sellOrders;
};

vector<Order> readCSV(const string& filename) {
    vector<Order> orders;

    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Unable to open the CSV file." << endl;
        return orders;
    }

    string line, token;
    getline(file, line); // Skip the header row

    while (getline(file, line)) {
        stringstream ss(line);
        vector<string> tokens;

        while (getline(ss, token, ',')) {
            tokens.push_back(token);
        }

        if (tokens.size() != 5) {
            cerr << "Error: Invalid CSV format." << endl;
            return orders;
        }

        Order order;
        order.cl_ord_id = tokens[0];
        order.instrument = tokens[1];
        order.side = stoi(tokens[2]);
        order.quantity = stoi(tokens[3]);
        order.price = stod(tokens[4]);
        order.status = "New";


        orders.push_back(order);
    }

    return orders;
}

bool isValidClientOrderID(const string& id) {
    return id.size() <= 7 && !id.empty();
}

bool isValidInstrument(const string& instrument) {
    static const vector<string> validInstruments = {"Rose", "Lavender", "Lotus", "Tulip", "Orchid"};
    return find(validInstruments.begin(), validInstruments.end(), instrument) != validInstruments.end();
}

bool isValidSide(int side) {
    return side == 1 || side == 2;
}

bool isValidPrice(double price) {
    return price > 0.0;
}

bool isValidQuantity(int quantity) {
    return quantity >= 10 && quantity <= 1000 && quantity % 10 == 0;
}

bool compareOrderByPriceAsc(const Order& a, const Order& b) {
    return a.price < b.price;
}

bool compareOrderByPriceDesc(const Order& a, const Order& b) {
    return a.price > b.price;
}

vector<Order> processOrders(vector<Order>& orders) {
    vector<Order> executionReports;
    map<string, OrderBook> orderBooks;
    int i=0;   
    for (Order& order : orders) {
        i++;
        order.ordid="ord"+to_string(i);
        if (!isValidClientOrderID(order.cl_ord_id) || !isValidInstrument(order.instrument) ||
            !isValidSide(order.side) || !isValidPrice(order.price) || !isValidQuantity(order.quantity)) {
            order.status = "Rejected";
            if(!isValidClientOrderID(order.cl_ord_id)){order.reson ="Invalid Client Order ID "   ;}
            if(!isValidInstrument(order.instrument)){
                                             if(!(order.reson=="")){order.reson=order.reson+"/";}
                                                order.reson +=  "Invalid Instrument " ;}
            if(!isValidPrice(order.price)){ if(!(order.reson=="")){order.reson=order.reson+"/ ";}
                                            order.reson +=  "Invalid Price " ;}

            if(!isValidSide(order.side)){ if(!(order.reson=="")){order.reson=order.reson+"/ ";}
                                            order.reson +=   "Invalid Side ";}
            if(!isValidQuantity(order.quantity)){ if(!(order.reson=="")){order.reson=order.reson+"/ ";}
                                            order.reson += "Invalid Quantity "  ;}


            executionReports.push_back(order);
        } else {
            if (orderBooks.find(order.instrument) == orderBooks.end()) {
                orderBooks[order.instrument] = OrderBook();
            }

            OrderBook& book = orderBooks[order.instrument];

            if (order.side == 1) { // Buy order
                if (book.sellOrders.empty()) {
                    executionReports.push_back(order);
                    //book.buyOrders.push_back(order);
                    book.buyOrders.insert(book.buyOrders.begin(), order);
                    
                } 
                else {
                if(book.sellOrders.size() > 1){
                    sort(book.sellOrders.begin(), book.sellOrders.end(), compareOrderByPriceAsc);
                }
                if (order.price < book.sellOrders.front().price) {
                    executionReports.push_back(order);
                    //book.buyOrders.push_back(order);
                    book.buyOrders.insert(book.buyOrders.begin(), order);   

                } 
                else {
    
                    while (order.quantity > 0 && !book.sellOrders.empty() && order.price >= book.sellOrders.front().price) {
                        int temp;
                        double price;

                        Order& sellOrder = book.sellOrders.front();
                        int reduce = min(order.quantity, sellOrder.quantity);
                        if(order.quantity == sellOrder.quantity){
                            price = order.price;
                            order.price = sellOrder.price;
                            order.status = "Fill";
                            executionReports.push_back(order);
                            sellOrder.status = "Fill";
                            executionReports.push_back(sellOrder);
                            order.quantity = 0;
                            sellOrder.quantity = 0;
                            book.sellOrders.erase(book.sellOrders.begin());
                            order.price = price;
                            break;
                        }
  
                        if (reduce == order.quantity) {

                            price = order.price;
                            order.price = sellOrder.price;
                            temp = sellOrder.quantity;
                            //order.quantity = 0;
                            order.status = "Fill";
                            executionReports.push_back(order);
                            sellOrder.quantity = reduce;
                            sellOrder.status = "Pfill";
                            executionReports.push_back(sellOrder);
                            sellOrder.quantity = temp;
                            sellOrder.quantity -= reduce;
                            order.quantity = 0;
                            order.price = price;
                            break;
                        } 
                        else {
                            price = order.price;
                            order.price = sellOrder.price;
                            temp= order.quantity;
                            order.quantity = reduce;
                            order.status = "Pfill";
                            executionReports.push_back(order);
                            order.price = price;
                            sellOrder.status = "Fill";
                            executionReports.push_back(sellOrder);
                            sellOrder.quantity = 0;
                            order.quantity = temp;
                            order.quantity -= reduce;
                            book.sellOrders.erase(book.sellOrders.begin());
                            
                       
                        }
                    }

                    if (order.quantity > 0) {

                        book.buyOrders.insert(book.buyOrders.begin(), order);
                    }
                }}
            } else if (order.side == 2) {
                 // Sell order
                if(book.buyOrders.empty()){
                    executionReports.push_back(order);
                    book.sellOrders.insert(book.sellOrders.begin(), order);
                }
                else{
                    if(book.buyOrders.size() > 1){
                        sort(book.buyOrders.begin(), book.buyOrders.end(), compareOrderByPriceDesc);
                    }
                    if(order.price > book.buyOrders.front().price){
                        executionReports.push_back(order);
                        book.sellOrders.insert(book.sellOrders.begin(), order);
                        
                        
                    }
                    else{
                        while(order.quantity > 0 && !book.buyOrders.empty() && order.price <= book.buyOrders.front().price){
                            int temp;
                            double price ;
                            Order& buyOrder = book.buyOrders.front();
                            int reduce = min(order.quantity, buyOrder.quantity);
                            
                            if(order.quantity == buyOrder.quantity){
                                price = order.price;
                                order.price = buyOrder.price;
                                order.status = "Fill";
                                executionReports.push_back(order);
                                buyOrder.status = "Fill";
                                executionReports.push_back(buyOrder);
                                order.quantity = 0;
                                buyOrder.quantity = 0;
                                order.price = price;
                                book.buyOrders.erase(book.buyOrders.begin());
                                break;
                            }
                            if(reduce == order.quantity){
                                price = order.price;
                                order.price = buyOrder.price;
                                temp = buyOrder.quantity;
                                order.status = "Fill";
                                executionReports.push_back(order);
                                buyOrder.quantity = reduce;
                                buyOrder.status = "Pfill";
                                executionReports.push_back(buyOrder);
                                order.quantity = 0;
                                buyOrder.quantity = temp;
                                buyOrder.quantity -= reduce;
                                order.price = price;
                                break;
                            }
                            else{
                                price = order.price;
                                order.price = buyOrder.price;
                                temp = order.quantity;
                                order.quantity = reduce;
                                order.status = "Pfill";
                                executionReports.push_back(order);
                                order.price = price;
                                buyOrder.status = "Fill";
                                executionReports.push_back(buyOrder);
                                buyOrder.quantity = 0;
                                order.quantity = temp;
                                order.quantity -=reduce ;
                                book.buyOrders.erase(book.buyOrders.begin());

                            }
                        }
                        if(order.quantity > 0){
                            book.sellOrders.insert(book.sellOrders.begin(), order);
                        }
                    }
                }


                
            }
        }
    }

    return executionReports;
}



void writeExecutionReports(vector<Order>& executionReports, const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Unable to open the CSV file for writing." << endl;
        return;
    }

    file << "Order ID,Client Order ID,Instrument,Side,Status,Quantity,Price,Transaction Time,Rejected Reason" << endl;

    for (Order& report : executionReports) {

        
        time_t now = time(nullptr);
        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%Y%m%d-%H%M%S", localtime(&now));
 
        file << report.ordid << "," << report.cl_ord_id << "," << report.instrument << ","
             << report.side << "," << report.status << "," << report.quantity << "," << report.price<< ".00" << "," << timestamp << ","<< report.reson<< endl;
    }

    file.close();
}

int main() {
    vector<Order> orders = readCSV("order.csv");
    vector<Order> executionReports = processOrders(orders);
    cout << "Execution processed successfully." << endl;
    writeExecutionReports(executionReports, "execution_report.csv");
    cout << "Execution generated successfully." << endl;

    return 0;
}
