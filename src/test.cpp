#include"palgorithm.h"
#include<vector>
#include<iostream>
#include"arg.h"
using namespace pstd;
using std::cout;
using std::endl;
//基础算法测试
//int main() {
//	std::vector<double>vec = { 3,2,5,8,4,1,0 };
//	pstd::sort(vec.begin(), vec.end(), [](const double& a, const double& b) {return a > b; });
//	for (auto& it : vec) {
//		std::cout << it << ",";
//	}
//}


//命令行参数解析测试
//int main(int argc, char* argv[])
//{
//    argh::parser cmdl;
//    cmdl.parse(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);
//
//    if (cmdl["-v"])
//        cout << "Verbose, I am." << endl;
//
//    cout << "Positional args:\n";
//    for (auto& pos_arg : cmdl)
//        cout << '\t' << pos_arg << endl;
//
//    cout << "Positional args:\n";
//    for (auto& pos_arg : cmdl.pos_args())
//        cout << '\t' << pos_arg << endl;
//
//    cout << "\nFlags:\n";
//    for (auto& flag : cmdl.flags())
//        cout << '\t' << flag << endl;
//
//    cout << "\nParameters:\n";
//    for (auto& param : cmdl.params())
//        cout << '\t' << param.first << " : " << param.second << endl;
//
//    return EXIT_SUCCESS;
//}
/**

E:\visual\pBoost\x64\Debug>pBoost.exe --name=DYsodg --addr 127.0.0.1 -xabc
Positional args:
        pBoost.exe
Positional args:
        pBoost.exe

Flags:
        xabc

Parameters:
        addr : 127.0.0.1
        name : DYsodg


*/


//csv读写测试
//#include"csv.h"
//#include <vector>
//#include <string>
//int main() {
//    std::ofstream stream("foo.csv");
//    csv::Writer<csv::delimiter<','>> writer(stream);
//
//    std::vector<std::vector<std::string>> rows =
//    {
//        {"a", "b", "c"},
//        {"1", "2", "3"},
//        {"4", "5", "6"}
//    };
//
//    writer.write_rows(rows);
//    stream.close();
//    csv::Reader<csv::delimiter<','>,
//        csv::quote_character<'"'>,
//        csv::first_row_is_header<true>,
//        csv::trim_policy::trim_whitespace> csv;
//
//    if (csv.mmap("foo.csv")) {
//        const auto header = csv.header();
//        for (const auto row : csv) {
//            for (const auto cell : row) {
//                // Do something with cell value
//                // std::string value;
//                // cell.read_value(value);
//            }
//        }
//    }
//
//}

//#include"xml.h"
////xml解析测试
//int main()
//{
//	xml::XMLDocument doc;
//	doc.LoadFile("dream.xml");
//
//	// Structure of the XML file:
//	// - Element "PLAY"      the root Element, which is the
//	//                       FirstChildElement of the Document
//	// - - Element "TITLE"   child of the root PLAY Element
//	// - - - Text            child of the TITLE Element
//
//	// Navigate to the title, using the convenience function,
//	// with a dangerous lack of error checking.
//	const char* title = doc.FirstChildElement("PLAY")->FirstChildElement("TITLE")->GetText();
//	printf("Name of play (1): %s\n", title);
//
//	// Text is just another Node to TinyXML-2. The more
//	// general way to get to the XMLText:
//	xml::XMLText * textNode = doc.FirstChildElement("PLAY")->FirstChildElement("TITLE")->FirstChild()->ToText();
//	title = textNode->Value();
//	printf("Name of play (2): %s\n", title);
//}
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include"http.h"
//http测试
int main() {

    //打开ssl支持需要声明宏:#define CPPHTTPLIB_OPENSSL_SUPPORT并链接libssl and libcrypto
    // HTTP server
    /**
    http::Server svr;

    svr.Get("/hi", [](const http::Request&, http::Response& res) {
        res.set_content("Hello World!", "text/plain");
        });

    svr.listen("0.0.0.0", 8080);
    */

    //HTTP Client
    // HTTP


    // HTTPS
    http::Client cli("http://www.baidu.com");

    auto res = cli.Get("/s?tn=15007414_15_dg&ie=utf-8&wd=csdn");
    res->status;
    std::cout<<res->body;
    return 0;
}