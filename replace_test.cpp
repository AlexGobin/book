#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <functional>


//http://c.biancheng.net/view/625.html
int main()
{
    std::array<int, 10> s{5, 7, 4, 2, 8, 6, 1, 9, 0, 3};
 
    std::replace(s.begin(), s.end(), 8, 88);
 
    for (int a : s) {
        std::cout << a << " ";
    }
    std::cout << '\n';
 
    std::replace_if(s.begin(), s.end(), 
                    std::bind(std::less<int>(), std::placeholders::_1, 5), 55);
    for (int a : s) {
        std::cout << a << " ";
    }

    std::cout << '\n';

    std::string str ="57 268 \t\n";
    //std::replace_if(str.begin(), str.end(),std::bind(std::equal_to<char>(), std::placeholders::_1,' '),'\0');
    std::replace_if(str.begin(), str.end(),[](char ch){return std::isspace(ch)||(ch=='\t')||(ch=='\n');},'\0');
    std::cout << str << ";" <<'\n';
}